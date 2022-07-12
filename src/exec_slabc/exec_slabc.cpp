#include "../../include/rouflex/exec_slabc.h"

rouflex::ExecSLABC::ExecSLABC(  const Instance & instance_inp,
                                const std::string stats_sink_path_inp,
                                const float Phyb_init_inp,
                                const int seed_inp) :
                                    instance{instance_inp},
                                    stats_sink_path{stats_sink_path_inp},
                                    Phyb_init{Phyb_init_inp},
                                    seed{seed_inp} {}

void rouflex::ExecSLABC::execute() const {

    const int n_jobs = this->instance.getNJobs();
    const int n_ops = this->instance.getNOps();
    const int n_res = this->instance.getNMachines();

    const int nIter = 30 * n_jobs * n_res;
    const int nEP = 100;
    const int nOP = 100;
    const int nSC = 5;
    const int limit_up = 40;
    const int limit_low = 10;

    const float Phyb_init_inp = 1.0;
    const float eps_inp = 0.85;
    const double learning_rate_inp = 0.75;
    const double discount_factor_inp = 0.2;
    const double no_improvement_reward_inp = 0.000;
    std::default_random_engine rengine_inp( this->seed );
    const std::vector< double > state_score_thresholds_inp = {0.05, 0.1, 0.15, 0.2, 0.25, 0.3, 0.35, 0.4, 0.45, 0.5, 0.55, 0.6, 0.65, 0.7, 0.75, 0.8, 0.85, 0.9, 0.95};
    const std::vector< float > Pmpi_vector_inp = {0.7};
    const std::vector< int > nrp_vector_inp = {static_cast<int>( n_jobs * 0.1 ), static_cast<int>( n_jobs * 0.2 ), static_cast<int>( n_jobs * 0.3 )};
    const std::vector< float > Pswap_vector_inp = {0.5};
    const std::vector< float > Phyb_swap_vector_inp = {0.0, 0.4, 0.8};
    const std::vector< float > Phyb_switch_vector_inp = {0.0, 0.4, 0.8};

    //std::cout << "Instantiating slabc" << std::endl;
    
    rouflex::SLABC2 slabc(
        this->instance,
        nIter,
        nEP,
        nOP,
        nSC,
        limit_up,
        limit_low,
        Phyb_init_inp,
        eps_inp,
        learning_rate_inp,
        discount_factor_inp,
        no_improvement_reward_inp,
        rengine_inp,
        state_score_thresholds_inp,
        Pmpi_vector_inp,
        nrp_vector_inp,
        Pswap_vector_inp,
        Phyb_swap_vector_inp,
        Phyb_switch_vector_inp,
        this->stats_sink_path
    );

    //std::cout << "Instantiated slabc" << std::endl;

    slabc.solve();

}

void rouflex::slabc_parallel_exec(std::vector< std::unique_ptr<ExecSLABC> > & ABCs, std::atomic<int> & global_read_point) {
    int read_point = global_read_point++;
    while(read_point < ABCs.size()) {
        std::cout << "Began working on read point: " << read_point << std::endl;
        ABCs[ read_point ]->execute();
        std::cout << "Executed read point: " << read_point << std::endl;
        read_point = global_read_point++;
    }
}