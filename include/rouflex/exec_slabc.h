#ifndef EXEC_SLABC_H
#define EXEC_SLABC_H

#include <thread>
#include <atomic>
#include <chrono>

#include "instance.h"
//#include "encoding.h"
//#include "decoder.h"
#include "slabc2.h"

namespace rouflex {

    class ExecSLABC {
        public:
            ExecSLABC(  const Instance & instance_inp,
                        const std::string stats_sink_path_inp,
                        const float Phyb_init_inp,
                        const int seed_inp);
            void execute() const;
        private:
            const Instance & instance;
            const std::string stats_sink_path;
            const float Phyb_init;
            const int seed;
    };

    void slabc_parallel_exec(std::vector< std::unique_ptr<ExecSLABC> > & ABCs, std::atomic<int> & global_read_point);

}

#endif //EXEC_SLABC_H