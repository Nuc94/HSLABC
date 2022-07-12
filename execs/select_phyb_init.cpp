#include <iostream>
#include <string>
#include <random>
#include <vector>
#include <map>
#include <fstream>

#include "../include/rouflex/instance.h"
#include "../include/rouflex/encoding.h"
#include "../include/rouflex/decoder.h"

int main() {

    for(int n_instance = 1; n_instance <= 15; ++n_instance) {
        std::cout << n_instance << std::endl;
        std::string s_instance = std::to_string(n_instance);
        const std::string INSTANCE_FILEPATH = "/home/nuc/Programmazione/C++/RouFlex/data/B3/Mk" + s_instance + ".json";
        const std::string performances_dump_path = "/home/nuc/Programmazione/C++/RouFlex/data/B3/Phyb_init_sel";
        std::default_random_engine rengine_inp(11);

        rouflex::Instance instance(INSTANCE_FILEPATH);

        const std::vector<float> Phybs = {0.0, 0.2, 0.4, 0.6, 0.8, 1.0};

        const int n_runs = 10000;

        std::map<float, std::vector<timeperiod::timeunit> > performances_by_phyb;

        for(const float Phyb : Phybs) {
            performances_by_phyb.insert( std::pair<float, std::vector<timeperiod::timeunit> >(Phyb, std::vector<timeperiod::timeunit>()) );
        }

        for(auto it = performances_by_phyb.begin(); it != performances_by_phyb.end(); ++it) it->second.reserve(n_runs);

        rouflex::Decoder decoder;

        for(auto it = performances_by_phyb.begin(); it != performances_by_phyb.end(); ++it) {
            for(int i = 0; i < n_runs; ++i) {
                rouflex::Encoding encode(instance, rengine_inp, true, it->first);
                it->second.push_back( decoder.decode(encode, true) );
                if(encode.getMakespan() == *(std::min_element(it->second.begin(), it->second.end()))) {
                    encode.recordGantt(performances_dump_path + "/best_gantt" + std::to_string(it->first) + ".csv");
                }
            }
        }

        const std::string filepath = performances_dump_path + "/action_stats" + s_instance + ".csv";
        std::ofstream csv_file;
        csv_file.open(filepath);
        csv_file << "iter";
        for(auto it = performances_by_phyb.begin(); it != performances_by_phyb.end(); ++it) {
            csv_file << ",Phyb_" << it->first;
        }
        for(int iter = 0; iter < n_runs; ++iter) {
            csv_file << "\n";
            csv_file << iter;
            for(auto it = performances_by_phyb.begin(); it != performances_by_phyb.end(); ++it) {
                csv_file << "," << it->second[iter];
            }
        }
    }

    return 0;

}