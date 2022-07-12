#include "../../include/rouflex/slga.h"

rouflex::SLGA::SLGA(
    const Instance & instance_inp,
    const unsigned int nIter_inp,
    const unsigned int pop_size_inp,
    const float eps_inp,
    const double learning_rate_inp,
    const double discount_factor_inp,
    const double no_improvement_reward_inp,
    const double wf_inp,
    const double wd_inp,
    const double wm_inp,
    std::default_random_engine & rengine_inp,
    const std::vector<double> & state_score_thresholds_inp,
    const std::vector<double> & pc_by_action_inp,
    const std::vector<double> & pm_by_action_inp,
    const std::string & stats_dump_path_inp
) : instance{instance_inp}, nIter{nIter_inp}, pop_size{pop_size_inp},
eps{eps_inp}, learning_rate{learning_rate_inp}, discount_factor{discount_factor_inp},
no_improvement_reward{no_improvement_reward_inp}, wf{wf_inp}, wd{wd_inp},
wm{wm_inp},
sarsa_update_threshold{ ((state_score_thresholds_inp.size() + 1) * pm_by_action_inp.size()) / 2 },
rengine{rengine_inp},
state_score_thresholds{state_score_thresholds_inp},
pc_by_action{pc_by_action_inp}, pm_by_action{pm_by_action_inp},
stats_dump_path{stats_dump_path_inp}, best_solution{nullptr},
state{0}, prev_state{0},
pc_action{0}, pm_action{0},
prev_pc_action{0}, prev_pm_action{0} {
    this->best_sol_tracking.reserve(this->nIter);
    this->initPop();
    this->initActionProviders();
}

timeperiod::timeunit rouflex::SLGA::solve() {
    for(int iter = 0; iter < this->nIter; ++iter) {
        //std::cout << "begin iteration " << iter << std::endl;
        this->iterate(iter);
        //std::cout << "best makespan " << this->getBestMakespan() << std::endl;
    }
    this->saveFitnessStats();
    return this->best_solution->getMakespan();
}

void rouflex::SLGA::initPop() {
    this->pop = std::make_unique< std::vector< std::unique_ptr<Encoding> > >( std::vector< std::unique_ptr<Encoding> >() );
    this->pop->reserve(this->pop_size + 1);
    for(int i = 0; i < this->pop_size; ++i) {
        this->pop->emplace_back( std::make_unique<Encoding>(this->instance, this->rengine, true, 0) );
    }
    this->decodePop();
    this->initStateStatsParams();
}

void rouflex::SLGA::initStateStatsParams() {
    double pop_fit_avg;
    this->first_pop_fit_sum = 0;
    this->first_pop_fit_var = 0;
    this->first_pop_fit_max = 0;
    for(int i = 0; i < this->pop->size(); ++i) {
        this->first_pop_fit_sum += (*this->pop)[i]->getMakespan();
        if( (*this->pop)[i]->getMakespan() > this->first_pop_fit_max ) {
            this->first_pop_fit_max = (*this->pop)[i]->getMakespan();
        }
    }
    pop_fit_avg = static_cast<double>( this->first_pop_fit_sum ) / static_cast<double>( this->pop->size() );
    for(int i = 0; i < this->pop->size(); ++i) {
        this->first_pop_fit_var += std::abs( (*this->pop)[i]->getMakespan() - pop_fit_avg );
    }
    this->pop_fit_sum = this->first_pop_fit_sum;
    this->pop_fit_var = this->first_pop_fit_var;
    this->pop_fit_max = this->first_pop_fit_max;
}

void rouflex::SLGA::decodePop() {
    for(int i = 0; i < this->pop->size(); ++i) {
        this->dec.decode( *((*this->pop)[i]), true );
    }
    this->checkBestSolAndStats();
    this->calcRewards();
}

void rouflex::SLGA::checkBestSolAndStats() {
    double pop_fit_avg;
    int best_pos = -1;
    timeperiod::timeunit best_mk;
    this->savePrevStats();
    this->pop_fit_sum = 0;
    this->pop_fit_var = 0;
    this->pop_fit_max = 0;
    if(this-> best_solution == nullptr) {
        best_mk = timeperiod::INFINITE_TIME;
    } else {
        best_mk = this->best_solution->getMakespan();
    }
    for(int i = 0; i < this->pop_size; ++i) {
        if( (*this->pop)[i]->getMakespan() < best_mk ) {
            best_mk = (*this->pop)[i]->getMakespan();
            best_pos = i;
        }
        this->pop_fit_sum += (*this->pop)[i]->getMakespan();
        if( (*this->pop)[i]->getMakespan() > this->pop_fit_max ) {
            this->pop_fit_max = (*this->pop)[i]->getMakespan();
        }
    }
    if( best_pos != -1 ) {
        this->best_solution = std::make_unique<Encoding>( *((*this->pop)[best_pos]) );
    }
    pop_fit_avg = static_cast<double>( this->pop_fit_sum ) / static_cast<double>( this->pop->size() );
    for(int i = 0; i < this->pop_size; ++i) {
        this->pop_fit_var += std::abs( (*this->pop)[i]->getMakespan() - pop_fit_avg );
    }
    this->trackBestSolutionFound();
}

void rouflex::SLGA::savePrevStats() {
    this->prev_pop_fit_max = this->pop_fit_max;
    this->prev_pop_fit_sum = this->pop_fit_sum;
}

void rouflex::SLGA::calcRewards() {
    this->prev_reward_cross = this->reward_cross;
    this->prev_reward_mut = this->reward_mut;
    this->reward_cross = static_cast<double>( this->pop_fit_max - this->prev_pop_fit_max ) / static_cast<double>(this->prev_pop_fit_max);
    this->reward_mut = static_cast<double>( this->pop_fit_sum - this->prev_pop_fit_sum ) / static_cast<double>(this->prev_pop_fit_sum);
}

void rouflex::SLGA::iterate(const int n_iter) {
    double pc, pm;
    this->calcActions();
    pc = this->getPc();
    pm = this->getPm();
    this->evolvePop(pc, pm);
    this->decodePop();
    this->updateActionProviders(n_iter);
}

void rouflex::SLGA::evolvePop(const double pc, const double pm) {
    double rand;
    std::uniform_real_distribution<double> unif(0, 1);
    this->buildNewPop();
    for(int i = 0; i < this->pop->size() - 1; i += 2) {
        rand = unif(this->rengine);
        if(rand < pc) {
            (*this->pop)[i]->permuteWithOther(*((*this->pop)[i + 1]), this->rengine);
        }
        //I shall then keep going for the mutation oeprators
        rand = unif(this->rengine);
        if(rand < pm) {
            (*this->pop)[i]->swapResource(this->rengine);
        }
        rand = unif(this->rengine);
        if(rand < pm) {
            (*this->pop)[i+1]->swapResource(this->rengine);
        }
    }
    
}

int rouflex::SLGA::getBinTourSelectionPos() {
    int pos1, pos2;
    pos1 = this->rengine() % this->pop->size();
    pos2 = this->rengine() % this->pop->size();
    while(pos2 == pos1) {
        pos2 = this->rengine() % pop_size;
    }
    if( (*this->pop)[pos1]->getMakespan() < (*this->pop)[pos2]->getMakespan() ) {
        return pos1;
    }
    return pos2;
}

std::pair< std::vector<int>, std::vector<int> > rouflex::SLGA::getBinTourSelection() {
    int pos;
    std::vector<int> pop_selections_counter(this->pop_size + 1, 0);
    std::vector<int> pop_selection;
    pop_selection.reserve(this->pop_size);
    for(int i = 0; i < this->pop_size; ++i) {
        pos = this->getBinTourSelectionPos();
        ++pop_selections_counter[pos];
        pop_selection.push_back(pos);
    }
    return std::make_pair( pop_selection, pop_selections_counter );
}

void rouflex::SLGA::buildNewPop() {
    int pos;
    std::vector<int> pop_selections_counter;
    std::vector<int> pop_selection;
    this->pop->emplace_back( std::make_unique<Encoding>( (*this->best_solution) ) );
    std::pair< std::vector<int>, std::vector<int> > sel_result = this->getBinTourSelection();
    pop_selection = std::move( sel_result.first );
    pop_selections_counter = std::move( sel_result.second );
    std::unique_ptr< std::vector< std::unique_ptr<Encoding> > > next_pop;
    next_pop = std::make_unique< std::vector< std::unique_ptr<Encoding> > >( std::vector< std::unique_ptr<Encoding> >() );
    next_pop->reserve(this->pop_size + 1);
    for(int i = 0; i < this->pop_size; ++i) {
        pos = pop_selection[i];
        if( pop_selections_counter[pos] == 1 ) {
            next_pop->emplace_back( std::move( (*this->pop)[pos] ) );
        } else {
            next_pop->emplace_back( std::make_unique<Encoding>( *((*this->pop)[pos]) ) );
        }
        --pop_selections_counter[pos];
    }
    this->pop = std::move(next_pop);
}

void rouflex::SLGA::calcState() {
    this->savePrevState();
    double score = this->getStateScore();
    for(int s = 0; s < this->state_score_thresholds.size(); ++s) {
        if( this->state_score_thresholds[s] >= score ) {
            this->state = s;
            return;
        }
    }
    this->state = this->state_score_thresholds.size();
}

void rouflex::SLGA::initActionProviders() {
    this->PcActionProvider = EpsActionProvider(this->getNStates(), this->pc_by_action.size(), this->eps, this->learning_rate, this->discount_factor);
    this->PmActionProvider = EpsActionProvider(this->getNStates(), this->pm_by_action.size(), this->eps, this->learning_rate, this->discount_factor);
}

void rouflex::SLGA::updateActionProviders(const int n_iter) {
    if( this->isUpdateSARSA(n_iter) ) {
        this->PcActionProvider.updateByRewardSARSA(this->prev_state, this->prev_pc_action, this->state, this->pc_action, this->prev_reward_cross);
        this->PmActionProvider.updateByRewardSARSA(this->prev_state, this->prev_pm_action, this->state, this->pm_action, this->prev_reward_mut);
    } else {
        this->PcActionProvider.updateByRewardQL(this->prev_state, this->prev_pc_action, this->state, this->prev_reward_cross);
        this->PmActionProvider.updateByRewardQL(this->prev_state, this->prev_pm_action, this->state, this->prev_reward_mut);
    }
}

void rouflex::SLGA::calcActions() {
    this->savePrevActions();
    this->calcState();
    this->pc_action = PcActionProvider.getEGreedyAction(this->state, this->rengine);
    this->pm_action = PmActionProvider.getEGreedyAction(this->state, this->rengine);
}

void rouflex::SLGA::saveFitnessStats() const {
    const std::string filepath = this->stats_dump_path + "/fitness_stats.csv";
    std::ofstream csv_file;
    csv_file.open(filepath);
    csv_file << "iter";
    csv_file << ",best";
    for(int iter = 0; iter < this->best_sol_tracking.size(); ++iter){
        csv_file << "\n";
        csv_file << std::to_string(iter);
        csv_file << "," + std::to_string(this->best_sol_tracking[iter]);
    }
    csv_file.close();
}

/*
    Here I code for parallel execution of code
*/

rouflex::ExecSLGA::ExecSLGA(    const Instance & instance_inp,
                                const std::string stats_sink_path_inp,
                                const int seed_inp) :
                                    instance{instance_inp},
                                    stats_sink_path{stats_sink_path_inp},
                                    seed{seed_inp} {}

void rouflex::ExecSLGA::execute() const {

    const int n_jobs = this->instance.getNJobs();
    const int n_ops = this->instance.getNOps();
    const int n_res = this->instance.getNMachines();

    const unsigned int nIter_inp = 10 * n_jobs * n_res;
    const unsigned int pop_size_inp = 5 * n_jobs * n_res;
    const float eps_inp = 0.85;
    const double learning_rate_inp = 0.75;
    const double discount_factor_inp = 0.2;
    const double no_improvement_reward_inp = 0;
    const double wf_inp = 0.35;
    const double wd_inp = 0.35;
    const double wm_inp = 0.3;
    std::default_random_engine rengine_inp(this->seed);
    std::vector<double> state_score_thresholds_inp;
    std::vector<double> pc_by_action_inp;
    std::vector<double> pm_by_action_inp;
    const std::string stats_dump_path_inp = this->stats_sink_path;

    for(double s = 0.05; s <= 1.0; s += 0.05) {
        state_score_thresholds_inp.push_back(s);
    }
    for(double pc = 0.4; pc <= 0.9; pc += 0.05) {
        pc_by_action_inp.push_back(pc);
    }
    for(double pm = 0.01; pm <= 0.21; pm += 0.02) {
        pm_by_action_inp.push_back(pm);
    }

    rouflex::SLGA slga(
        this->instance,
        nIter_inp,
        pop_size_inp,
        eps_inp,
        learning_rate_inp,
        discount_factor_inp,
        no_improvement_reward_inp,
        wf_inp,
        wd_inp,
        wm_inp,
        rengine_inp,
        state_score_thresholds_inp,
        pc_by_action_inp,
        pm_by_action_inp,
        stats_dump_path_inp
    );

    slga.solve();

}

void rouflex::slga_parallel_exec(std::vector< std::unique_ptr<ExecSLGA> > & SLGAs, std::atomic<int> & global_read_point) {
    int read_point = global_read_point++;
    while(read_point < SLGAs.size()) {
        std::cout << "Began working on read point: " << read_point << std::endl;
        SLGAs[ read_point ]->execute();
        std::cout << "Executed read point: " << read_point << std::endl;
        read_point = global_read_point++;
    }
}