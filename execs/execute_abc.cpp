#include <iostream>
#include <string>
#include <random>

#include "../include/rouflex/instance.h"
//#include "../include/rouflex/encoding.h"
//#include "../include/rouflex/decoder.h"
#include "../include/rouflex/abc.h"

int main() {

    //benchmark parameters
    int seed = 20;
    const int N_BENCHMARKS = 10;
    const int N_RUNS = 20;
    const std::string BENCHMARK_PATH = "/home/nuc/Programmazione/C++/RouFlex/data/brandison";

    for(int mk = 1; mk <= N_BENCHMARKS; ++mk) {
        const std::string BENCHMARK_FOLDER = BENCHMARK_PATH + "/Mk" + std::to_string(mk) + "/";
        const std::string INSTANCE_FILEPATH = BENCHMARK_FOLDER + "Mk" + std::to_string(mk) + ".json";
        const std::string RESULTS_FOLDER = BENCHMARK_FOLDER + "ABC/";
        std::cout << INSTANCE_FILEPATH << std::endl;
        rouflex::Instance instance(INSTANCE_FILEPATH);

        const int n_jobs = instance.getNJobs();
        const int n_ops = instance.getNOps();
        const int n_res = instance.getNMachines();

        const unsigned int nIter = 30 * n_res * n_jobs;
        const unsigned int nEP = 100;
        const unsigned int nOP = 200;
        const unsigned int limit = 50;
        const float Pbt = 0.85;
        const float Pmpi = 0.90;
        const unsigned int nrp = static_cast<int>( n_jobs * 0.2 );
        const unsigned int nswp = static_cast<int>( n_jobs * 0.2 );
        const unsigned int nmc = static_cast<int>( n_ops * 0.3 );
        const float Pfinit = 0.0;
        const float Pf = 0.0;

        for(int r = 0; r < N_RUNS; ++r) {
            const std::string RESULT_FILEPATH = RESULTS_FOLDER + "result" + std::to_string(r) + ".csv";
            
            std::default_random_engine rengine(seed);

            rouflex::ABC abc(instance, nIter, nEP, nOP, limit, Pbt, Pmpi, nrp, nswp, nmc, Pfinit, Pf, rengine);
            abc.solve();
            abc.makespanToCsv( RESULT_FILEPATH );
            
            ++seed;
        }
    }

    return 0;

}