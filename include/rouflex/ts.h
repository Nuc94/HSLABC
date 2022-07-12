#ifndef TS_H
#define TS_H

#include <string>
#include <vector>
#include <random>
#include <memory>
#include <fstream>
#include <thread>
#include <atomic>

#include <math.h>

#include "../../include/rouflex/instance.h"
#include "../../include/rouflex/encoding.h"
#include "../../include/rouflex/decoder.h"

namespace rouflex {

    class TS {
    public:
        TS(
            const Instance & instance_inp,
            const unsigned int nIter_inp,
            const unsigned int gmax_inp,
            const unsigned int tabu_size_inp,
            const int rengine_seed_inp,
            const std::string & stats_dump_path_inp
        );
        void solve();
    private:
        const Instance & instance;
        const unsigned int nIter;
        const unsigned int gmax;
        const unsigned int tabu_size;
        const int rengine_seed;
        std::default_random_engine rengine;
        const std::string & stats_dump_path;
        Decoder dec;
        timeperiod::timeunit best_makespan;
        std::unique_ptr<Encoding> p;
        std::vector< std::pair<int, int> > tabu_list;
        std::vector<timeperiod::timeunit> makespan_history;
        void iterate();
        void initP();
        bool notInTabuList(int k, int l) const;
        void saveFitnessStats() const;
    };

    /*
    Here is the class for parallel execution
    */

    class ExecTS {
    public:
        ExecTS(   const Instance & instance_inp,
                    const std::string stats_sink_path_inp,
                    const int seed_inp);
        void execute() const;
    private:
        const Instance & instance;
        const std::string stats_sink_path;
        const int seed;
    };

    void ts_parallel_exec(std::vector< std::unique_ptr<ExecTS> > & TSs, std::atomic<int> & global_read_point);

}

#endif //TS_H