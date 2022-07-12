#include "../../include/rouflex/ts.h"

rouflex::TS::TS(
    const Instance & instance_inp,
    const unsigned int nIter_inp,
    const unsigned int gmax_inp,
    const unsigned int tabu_size_inp,
    const int rengine_seed_inp,
    const std::string & stats_dump_path_inp
    ) : instance{instance_inp}, nIter{nIter_inp},
        gmax{gmax_inp}, tabu_size{tabu_size_inp},
        rengine_seed{rengine_seed_inp}, rengine{rengine_seed_inp},
        stats_dump_path{stats_dump_path_inp},
        best_makespan{timeperiod::INFINITE_TIME},
        p{nullptr}
    {
        this->dec = Decoder();
        this->tabu_list.reserve(this->nIter);
        this->makespan_history.reserve(this->nIter + 1);
        this->initP();
    }

void rouflex::TS::initP() {
    this->p = std::make_unique<Encoding>(this->instance, this->rengine, true, 1.0);
    this->p->dispatchLJD();
    best_makespan = this->dec.decode(*(this->p), true);
}

void rouflex::TS::solve() {
    for(int iter = 0; iter < this->nIter; ++iter) {
        this->iterate();
    }
    this->saveFitnessStats();
}

void rouflex::TS::iterate() {
    const std::vector<int> original_permutation = this->p->getJobOrder();
    std::vector<int> actual_permutation;
    std::unique_ptr<Encoding> actual_p = std::make_unique<Encoding>(this->instance, this->rengine, true, 1.0);
    int exchange, k_best, l_best;
    timeperiod::timeunit actual_makespan, iteration_best_makespan = timeperiod::INFINITE_TIME;
    for(int g = 1; g <= this->gmax; ++g) {
        for(int k = 0; k <= this->instance.getNJobs() - 2*g; ++k) {
            for(int l = k + g; l <= this->instance.getNJobs() - g; ++l) {
                if(this->notInTabuList(k, l)) {
                    actual_permutation = original_permutation;
                    for(int pos = 0; pos < g; ++pos) {
                        exchange = actual_permutation[k + pos];
                        actual_permutation[k + pos] = actual_permutation[l + pos];
                        actual_permutation[l + pos] = exchange;
                    }
                    actual_p->resetByPermutation(actual_permutation);
                    actual_makespan = this->dec.decode( *actual_p, true );
                    if(actual_makespan < iteration_best_makespan) {
                        this->p = std::move(actual_p);
                        actual_p = std::make_unique<Encoding>(this->instance, this->rengine, true, 1.0);
                        k_best = k;
                        l_best = l;
                    }
                }
            }
        }
    }
    this->tabu_list.emplace_back( std::make_pair(k_best, l_best) );
    this->makespan_history.push_back( this->p->getMakespan() );
}

bool rouflex::TS::notInTabuList(int k, int l) const {
    int tabu_start = std::max(0, static_cast<int>(this->tabu_list.size() - this->tabu_size));
    for(int i = tabu_start; i < this->tabu_list.size(); ++i) {
        if( this->tabu_list[i].first == k && this->tabu_list[i].second == l ) return false;
    }
    return true;
}

void rouflex::TS::saveFitnessStats() const {
    const std::string filepath = this->stats_dump_path + "/fitness_stats.csv";
    std::ofstream csv_file;
    csv_file.open(filepath);
    csv_file << "iter";
    csv_file << ",best";
    for(int iter = 0; iter < this->makespan_history.size(); ++iter){
        csv_file << "\n";
        csv_file << std::to_string(iter);
        csv_file << "," + std::to_string(this->makespan_history[iter]);
    }
    csv_file.close();
}



rouflex::ExecTS::ExecTS(    const Instance & instance_inp,
                            const std::string stats_sink_path_inp,
                            const int seed_inp) :
                                instance{instance_inp},
                                stats_sink_path{stats_sink_path_inp},
                                seed{seed_inp} {}

void rouflex::ExecTS::execute() const {

    const int n_jobs = this->instance.getNJobs();
    const int n_ops = this->instance.getNOps();
    const int n_res = this->instance.getNMachines();

    const unsigned int nIter_inp = 500;
    const unsigned int gmax_inp = static_cast<unsigned int>(sqrt(static_cast<double>(n_jobs)) + 0.5) ;
    const unsigned int tabu_size_inp = 2 * gmax_inp;

    rouflex::TS ts(
        this->instance,
        nIter_inp,
        gmax_inp,
        tabu_size_inp,
        this->seed,
        this->stats_sink_path
    );

    ts.solve();

}

void rouflex::ts_parallel_exec(std::vector< std::unique_ptr<ExecTS> > & TSs, std::atomic<int> & global_read_point) {
    int read_point = global_read_point++;
    while(read_point < TSs.size()) {
        std::cout << "Began working on read point: " << read_point << std::endl;
        TSs[ read_point ]->execute();
        std::cout << "Executed read point: " << read_point << std::endl;
        read_point = global_read_point++;
    }
}