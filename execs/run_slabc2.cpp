#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <atomic>
#include <memory>

#include "../include/rouflex/instance.h"
//#include "../include/rouflex/encoding.h"
//#include "../include/rouflex/decoder.h"
#include "../include/rouflex/exec_slabc.h"

int main() {

    const int N_MK = 10;
    const int N_RUNS = 20;
    int seed = 20;

    const std::string root_path = "/home/nuc/Programmazione/C++/RouFlex/data/brandison";

    std::string mk_path, instance_path, run_path;

    std::vector< std::unique_ptr< rouflex::ExecSLABC > > execs;

    std::atomic<int> global_read_point(0);
    const int n_threads = std::thread::hardware_concurrency();
    std::vector<std::thread> threads;
    std::vector< std::unique_ptr<rouflex::Instance> > instances;

    for(int mk = 1; mk <= N_MK; ++mk) {
        mk_path = root_path + "/Mk" + std::to_string(mk);
        instance_path = mk_path + "/Mk" + std::to_string(mk) + ".json";
        std::cout << instance_path << std::endl;
        instances.push_back( std::make_unique<rouflex::Instance>(instance_path) );
        for(int run = 1; run <= N_RUNS; ++run) {
            run_path = mk_path + "/HSLABC2/Run" + std::to_string(run);
            execs.emplace_back( std::make_unique<rouflex::ExecSLABC>(*instances.back(), run_path, 1.0, seed) );
            ++seed;
        }
    }

    //threads.reserve(n_threads - 1);
    for(int t = 0; t < n_threads - 1; ++t) {
        std::cout << "Launched thread " << t << std::endl;
        threads.emplace_back( std::thread(rouflex::slabc_parallel_exec, std::ref(execs), std::ref(global_read_point)) );
    }
    rouflex::slabc_parallel_exec( execs, global_read_point );

    for(auto & thread : threads) thread.join();

    return 0;



}