#include <iostream>
#include <string>
#include <random>
#include <thread>
#include <atomic>
#include <chrono>

#include "../include/rouflex/instance.h"
#include "../include/rouflex/encoding.h"
#include "../include/rouflex/decoder.h"
#include "../include/rouflex/slabc2.h"

int main() {

    const std::string INSTANCE_FILEPATH = "/home/nuc/Programmazione/C++/RouFlex/data/Instances/Mk10.json";
    const std::string folder_path = "/home/nuc/Programmazione/C++/RouFlex/data/Mk10";
    std::vector< std::default_random_engine > rengines;
    int i_reng = 0;

    std::atomic<int> global_read_point(0);
    const int n_threads = std::thread::hardware_concurrency();

    rouflex::Instance instance(INSTANCE_FILEPATH);

    const int n_runs = 10;

    const int n_jobs = instance.getNJobs();
    const int n_ops = instance.getNOps();
    const int n_res = instance.getNMachines();

    std::cout << "N Jobs: " << n_jobs << std::endl;
    std::cout << "N Ops: " << n_ops << std::endl;
    std::cout << "N Res: " << n_res << std::endl;

    //const unsigned int nIter = 30 * n_jobs * n_res;
    const unsigned int nIter = 9000;
    unsigned int nEP = 50;
    unsigned int nOP = 300;
    unsigned int nSC = 5;
    int limit_up = 20;
    int limit_low = 10;

    const float Phyb_init_inp = 1.0;
    const float eps_inp = 0.85;
    const double learning_rate_inp = 0.75;
    const double discount_factor_inp = 0.2;
    const double no_improvement_reward_inp = 0.000;
    std::default_random_engine rengine_inp(7);
    const std::vector< double > state_score_thresholds_inp = {0.05, 0.1};
    const std::vector< float > Pmpi_vector_inp = {0.2, 0.4, 0.8};
    const std::vector< int > nrp_vector_inp = {static_cast<int>( n_jobs * 0.1 ), static_cast<int>( n_jobs * 0.4 )};
    const std::vector< float > Pswap_vector_inp = {0.5};
    const std::vector< float > Phyb_swap_vector_inp = {0.0, 0.4, 0.8};
    const std::vector< float > Phyb_switch_vector_inp = {0.0, 0.4, 0.8};

    rouflex::SLABC2 slabc(
        instance,
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
        "/home/nuc/Programmazione/C++/RouFlex/data/ExpStats"
    );

    auto t1 = std::chrono::high_resolution_clock::now();

    slabc.solve();

    auto t2 = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::seconds>( t2 - t1 ).count();

    std::cout << "Duration: " << duration << std::endl;

    /*float Pbt = 0.85;
    float Pmpi = 0.9;
    unsigned int nrp = static_cast<int>( n_jobs * 0.2 );
    unsigned int nswp = static_cast<int>( n_jobs * 0.2 );
    unsigned int nmc = static_cast<int>( n_ops * 0.3 );
    float Pfinit = 0.0;
    float Pf = 0.0;

    std::vector<rouflex::ABCSetting> ABCs;

    for(int r = 0; r < n_runs; ++r, ++i_reng) {
        ABCs.emplace_back( rouflex::ABCSetting(instance, nIter, nEP, nOP, limit, Pbt, Pmpi, nrp, nswp, nmc, Pfinit, Pf, i_reng) );
    }

    Pfinit = 0.8;
    Pf = 0.8;

    for(int r = 0; r < n_runs; ++r, ++i_reng) {
        ABCs.emplace_back( rouflex::ABCSetting(instance, nIter, nEP, nOP, limit, Pbt, Pmpi, nrp, nswp, nmc, Pfinit, Pf, i_reng) );
    }

    std::vector<std::thread> threads;

    auto t1 = std::chrono::high_resolution_clock::now();
    
    for(int i = 0; i < n_threads - 1; ++i) {
        threads.emplace_back( std::thread( rouflex::abc_parallel_exec, std::ref(ABCs), std::ref(global_read_point), std::ref(folder_path) ) );
    }
    rouflex::abc_parallel_exec(ABCs, global_read_point, folder_path);
    for(auto & t : threads) t.join();

    auto t2 = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::seconds>( t2 - t1 ).count();

    std::cout << "Duration: " << duration << std::endl;
    std::cout << "Total numer of ABC executions:" << ABCs.size() << std::endl;*/ 

    return 0;

}