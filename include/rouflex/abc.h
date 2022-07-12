#ifndef ABC_H
#define ABC_H

#include <string>
#include <vector>
#include <random>
#include <memory>
#include <fstream>
#include <thread>
#include <atomic>

#include "../../include/rouflex/instance.h"
#include "../../include/rouflex/encoding.h"
#include "../../include/rouflex/decoder.h"

namespace rouflex {

    class ABC {
    public:
        //Artificial Bee Colony constructor
        ABC(const Instance & inp_instance,
            const unsigned int inp_nIter,
            const unsigned int inp_nEP,
            const unsigned int inp_nOP,
            const unsigned int inp_limit,
            const float inp_Pbt,
            const float inp_Pmpi,
            const unsigned int inp_nrp,
            const unsigned int inp_nswp,
            const unsigned int inp_nmc,
            const float inp_Pfinit,
            const float inp_Pf,
            std::default_random_engine & reng);
        timeperiod::timeunit solve();
        void iterate();
        inline timeperiod::timeunit getBestMakespan() const {
            return this->best_makespan;
        }
        void makespanToCsv(const std::string & filepath) const;
    private:
        const Instance & instance;
        const unsigned int nIter;
        const unsigned int nEP; //Number of Employer Bees
        const unsigned int nOP; //Number of Onlooker Bees
        const unsigned int limit; //limit after which we substitute a solution
        const float Pbt; //probability of selecting best food source in binary tournament
        const float Pmpi;
        const unsigned int nrp; //number of randomly selected postions from one job in a library
        const unsigned int nswp; //number of job swaps in permutation
        const unsigned int nmc; //number of machine swaps in permutation
        const float Pfinit; //probability of allowing TA machine selectin in encoding initialization
        const float Pf; //probability of letting machine selection to TA in machine permutation
        std::default_random_engine & rengine;
        Decoder dec;
        std::vector< std::unique_ptr<Encoding> > food_sources;
        std::vector<int> no_improvement_cycles;
        std::vector< std::unique_ptr<Encoding> > employer_bees;
        std::vector< std::unique_ptr<Encoding> > onlooker_bees;
        std::vector<int> onlooker_origin;
        std::vector<timeperiod::timeunit> makespan_at_gen;
        timeperiod::timeunit best_makespan;
        //methods
        void onLook(); //this method shall handlew onlooker bees
        void renovateFoodSources();
        void substituteFoodSource(const int fs, std::unique_ptr<Encoding> new_foodsource);
        void resetFoodSource(const int fs);
        inline void updateMakespan(timeperiod::timeunit makespan) {
            this->best_makespan = std::min(this->best_makespan, makespan);
        }
    };

    class ABCSetting {
    public:
        ABCSetting(const Instance & inp_instance,
            const unsigned int inp_nIter,
            const unsigned int inp_nEP,
            const unsigned int inp_nOP,
            const unsigned int inp_limit,
            const float inp_Pbt,
            const float inp_Pmpi,
            const unsigned int inp_nrp,
            const unsigned int inp_nswp,
            const unsigned int inp_nmc,
            const float inp_Pfinit,
            const float inp_Pf,
            const int inp_seed):    instance(inp_instance),
                                    nIter(inp_nIter),
                                    nEP(inp_nEP), nOP(inp_nOP),
                                    limit(inp_limit), Pbt(inp_Pbt),
                                    Pmpi(inp_Pmpi),nrp(inp_nrp),
                                    nswp(inp_nswp),nmc(inp_nmc),
                                    Pfinit(inp_Pfinit),Pf(inp_Pf),
                                    seed(inp_seed) {}
        timeperiod::timeunit solve(const std::string & filepath) const;
    private:
        const Instance & instance;
        const unsigned int nIter;
        const unsigned int nEP; //Number of Employer Bees
        const unsigned int nOP; //Number of Onlooker Bees
        const unsigned int limit; //limit after which we substitute a solution
        const float Pbt; //probability of selecting best food source in binary tournament
        const float Pmpi;
        const unsigned int nrp; //number of randomly selected postions from one job in a library
        const unsigned int nswp; //number of job swaps in permutation
        const unsigned int nmc; //number of machine swaps in permutation
        const float Pfinit; //probability of allowing TA machine selectin in encoding initialization
        const float Pf; //probability of letting machine selection to TA in machine permutation
        const int seed;
    };

    //In the end we will have a function to launch parallel execution of multiple algorithms
    void abc_parallel_exec(std::vector< ABCSetting > & ABCs, std::atomic<int> & global_read_point, const std::string & folder_path);

}

#endif //ABC_H