#include <iostream>
#include <string>

#include "../include/rouflex/instance.h"

int main() {
    const std::string root_path = "/home/nuc/Programmazione/C++/RouFlex/data/Instances/Mk";
    std::string additional_zero;

    for(int i = 1; i <= 10; ++i) {
        if(i < 10) additional_zero = "0";
        else additional_zero = "";
        std::string instance_path = root_path + additional_zero + std::to_string(i) + ".json";
        rouflex::Instance instance(instance_path);
        std::cout << "Mk" << i <<std::endl;
        std::cout << "n_job " << instance.getNJobs() <<std::endl;
        std::cout << "n_machines " << instance.getNMachines() <<std::endl;
        std::cout << "n_ops " << instance.getNOps() <<std::endl;
        std::cout << "n_duration " << instance.getTotalDuration() <<std::endl;
    }
}