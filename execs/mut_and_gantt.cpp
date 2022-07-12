#include <iostream>
#include <string>
#include <random>

#include "../include/rouflex/instance.h"
#include "../include/rouflex/encoding.h"
#include "../include/rouflex/decoder.h"

int main() {

    const std::string INSTANCE_FILEPATH = "/home/nuc/Programmazione/C++/RouFlex/data/Instances/Mk10.json";
    const std::string folder_path = "/home/nuc/Programmazione/C++/RouFlex/data/Mk10";
    std::default_random_engine rengine_inp(6);

    rouflex::Instance instance(INSTANCE_FILEPATH);

    const int n_jobs = instance.getNJobs();
    const int n_ops = instance.getNOps();
    const int n_res = instance.getNMachines();

    const int nswp = static_cast<int>( n_jobs * 0.2 );
    const int nmc = static_cast<int>( 1.1 * n_ops );
    const float Phyb = 1.0;
    const float Phyb_init = 0.9;

    std::cout << "nswp: " << nswp << std::endl;
    std::cout << "nmc: " << nmc << std::endl;

    rouflex::Encoding encode(instance, rengine_inp, true, Phyb_init);

    rouflex::Decoder decoder;

    decoder.decode(encode, true);

    encode.logEncoding();
    encode.recordGantt("/home/nuc/Programmazione/C++/RouFlex/data/ExpStats/gantt1.csv");

    encode.perturbation( nswp, nmc, Phyb, rengine_inp);

    decoder.decode(encode, true);

    encode.logEncoding();
    encode.recordGantt("/home/nuc/Programmazione/C++/RouFlex/data/ExpStats/gantt2.csv");

    timeperiod::timeunit best = timeperiod::INFINITE_TIME;

    for(int i = 0; i < 10000; ++i ) {
        rouflex::Encoding encodes(instance, rengine_inp, true, Phyb_init);
        decoder.decode(encodes, true);
        if(encodes.getMakespan() < best) {
            best = encodes.getMakespan();
            encodes.recordGantt("/home/nuc/Programmazione/C++/RouFlex/data/ExpStats/gantt3.csv");
        }
    }

    return 0;

}