#ifndef ENCODING_H
#define ENCODING_H

#include <set>
#include <vector>
#include <random>
#include <fstream>
#include <iostream>
#include <iterator>
#include <unordered_map>
#include <unordered_set>
#include <boost/container/flat_set.hpp>

#include "instance.h"

namespace rouflex {

    class Encoding {
    public:
        Encoding(const Instance & input_instance,
                std::default_random_engine & rng_engine,
                const bool assign_machines, const float Pfinit);
        void resetByPermutation(const std::vector<int> permutation);
        inline bool isLegal() const {
            return this->isSequencingCorrect() && this->isResourceAssignmentCorrect();
        }
        bool isSequencingCorrect() const;
        bool isResourceAssignmentCorrect() const;
        bool isResourceAssignmentImposed() const;
        bool isResourceAssignmentImposed(const int job) const;
        inline const Instance & getInstance() const { return this->instance; }
        inline const std::vector<int> & getJobOrder() const { return this->job_order; }
        inline std::vector<int> & getMachineAssignmentAtJobModdable(const int job) {return this->machine_assignments[job];}
        inline const std::vector<int> & getMachineAssignmentAtJob(const int job) const {return this->machine_assignments[job];}
        inline int getMachineAssignmentQck(const int job, const int op) const {
            return this->machine_assignments[job][op];
        }
        inline bool isOpFlexiblySchedulable(const int job, const int op) const {
            return this->getMachineAssignmentQck(job, op) == -1;
        }
        inline timeperiod::timeunit getJobEntryPointQck(const int job) const {
            return this->jobEPs[job];
        }
        inline timeperiod::timeunit getMakespan() const { return this->makespan; }
        void setJobEPs(std::vector<timeperiod::timeunit> input_jobEPs);
        void logEncoding() const;
        //most important mutation operator
        void permute(   const Encoding & other, const unsigned int nrp,
                        const unsigned int nswp, const unsigned int nmc,
                        const float Pf, const float Pmpi,
                        std::default_random_engine & reng);
        void permuteByOther(const Encoding & other, const unsigned int nrp,
                            std::default_random_engine & reng);
        void permuteWithOther(Encoding & other, std::default_random_engine & reng);
        void perturbation(  const unsigned int nswp, const unsigned int nmc,
                            const float Pf, std::default_random_engine & reng);
        //here come a series of mutation operators
        void shiftJobsAndRandomizeResources(const float ta_prob, std::default_random_engine & rng_engine);
        int shiftJobs(std::default_random_engine & rng_engine);
        void mutateResource(const float ta_prob, std::default_random_engine & rng_engine);
        void swapResource(std::default_random_engine & rng_engine);
        void recordGantt(const std::string filepath) const;
        //FROM NOW ON THERE ARE METHODS USEFUL TO IMPLEMENT SIMULATED ANNEALING
        std::vector< int > getLastCompletedJobs() const;
        std::unordered_set< int > buildUncriticalJobs(const std::unordered_set< int > & last_jobs) const;
        void buildJobOpEndsByMachine(
            std::unordered_map< int, std::vector< std::vector< timeperiod::timeunit > > > & jobs_ops_ends,
            const std::unordered_set< int > & last_jobs
        ) const;
        void updateCriticalJobsOpStarts(
            std::unordered_map< int, std::vector< std::unordered_map< timeperiod::timeunit, int > > > & critical_jobs_opstarts,
            const std::unordered_set< int > & current_critical_jobs
        ) const;
        std::unordered_map< int, boost::container::flat_set<int> > getLongestPathJobs() const;
        std::unordered_map<int, timeperiod::timeunit> buildMachineLoads(int job_to_rebalance) const;
        void rebalanceSingleSAMachine(  const std::unordered_map<int, timeperiod::timeunit> & machine_loads,
                                        int job_to_rebalance, int op, std::default_random_engine & rengine);
        void rebalanceResourcesSA(int job_to_rebalance, std::default_random_engine & rengine);
        void reorderJobsSA(int job_to_reinsert);
        std::vector< std::unique_ptr<Encoding> > getSANeighbors( std::default_random_engine & rengine );
        inline timeperiod::timeunit getFirstJobImposedStart() const {
            return this->first_job_imposed_start;
        }
        inline void assignFirstJobImposedStart(timeperiod::timeunit imposed_start) {
            this->first_job_imposed_start = imposed_start;
        }
        //METHODS TO REORDER JOBS ACCORDING TO DISPATCHING RULES
        void dispatchLNO();
        void dispatchLJD();
        void dispatchLRA();
    private:
        //here come a series of mutation operators
        void shiftJobs(const int pos0, const int pos1);
        //method to be called for updating the makespan after having received some
        //input jobs entry points
        void updateMakespan();
        const Instance & instance;
        timeperiod::timeunit makespan;
        std::vector< int > job_order;
        std::vector< std::vector< int > > machine_assignments;
        //vector stating, for every job, its entry point
        std::vector< timeperiod::timeunit > jobEPs;
        //this attribute shall be used to esnure that the first job in the sequencing job_order
        //gets scheduled at a specific time instant according to the
        timeperiod::timeunit first_job_imposed_start;
    };

}

#endif //ENCODING_H
