#include <iostream>
#include <string>
#include <random>

#include "../include/rouflex/instance.h"
//#include "../include/rouflex/encoding.h"
//#include "../include/rouflex/decoder.h"
#include "../include/rouflex/sa.h"

int main() {

    //SA parameters
    const double starting_temperature = 100.0;
    const double min_temperature = 1.0;
    const double alpha = 0.99;
    const int max_iterations = 5000;

    //benchmark parameters
    int seed = 20;
    const int N_BENCHMARKS = 10;
    const int N_RUNS = 20;
    const std::string BENCHMARK_PATH = "/home/nuc/Programmazione/C++/RouFlex/data/brandison";

    for(int mk = 1; mk <= N_BENCHMARKS; ++mk) {
        const std::string BENCHMARK_FOLDER = BENCHMARK_PATH + "/Mk" + std::to_string(mk) + "/";
        const std::string INSTANCE_FILEPATH = BENCHMARK_FOLDER + "Mk" + std::to_string(mk) + ".json";
        const std::string RESULTS_FOLDER = BENCHMARK_FOLDER + "SA/";
        std::cout << INSTANCE_FILEPATH << std::endl;
        rouflex::Instance instance(INSTANCE_FILEPATH);

        for(int r = 0; r < N_RUNS; ++r) {
            const std::string RESULT_FILEPATH = RESULTS_FOLDER + "result" + std::to_string(r) + ".csv";
            
            std::default_random_engine rengine(seed);

            rouflex::SA sa(instance, starting_temperature, min_temperature, alpha, max_iterations, rengine);
            sa.execute();
            sa.makespanToCsv( RESULT_FILEPATH );
            
            ++seed;
        }
    }

    return 0;

}