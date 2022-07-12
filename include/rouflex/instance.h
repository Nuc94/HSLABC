#ifndef INSTANCE_H
#define INSTANCE_H

#include <string>
#include <fstream>
#include <iostream>
#include <streambuf>
#include <vector>
#include <random>
#include <boost/container/flat_set.hpp>

#include "timeperiod.h"
#include "../../third_party/json/single_include/nlohmann/json.hpp"

namespace rouflex {

    class Instance {
    public:
        Instance(const std::string & filepath);
        inline int getNMachines() const { return this->n_machines; }
        inline int getNJobs() const { return this->job_op_durations.size(); }
        inline int getNOps(const int job) const {
            if( ! this->isJobPresent(job) ) return -1;
            return this->job_op_durations[job].size();
        }
        inline int getNOps() const {
            return this->n_ops;
        }
        inline bool isJobPresent(const int job) const {
            if(job < 0 || job >= this->getNJobs()) return false;
            return true;
        }
        inline bool isOpPresent(const int job, const int op) const {
            if( ! this->isJobPresent(job) ) return false;
            if( op < 0 || op > this->getNOps(job) ) return false;
            return true;
        }
        inline timeperiod::timeunit getDurationQck(const int job, const int op) const {
            return this->job_op_durations[job][op];
        }
        inline const boost::container::flat_set<int> & getOpMachinesQck(const int job, const int op) const {
            return this->job_op_machines[job][op];
        }
        inline int getOpNMachinesQck(const int job, const int op) const {
            return this->job_op_machines[job][op].size();
        }
        inline const int getNthOpMachineQck(const int job, const int op, const int n_res) const {
            return *(this->job_op_machines[job][op].begin() + n_res);
        }
        inline bool isMachineAtOpQck(const int job, const int op, const int machine) const {
            return this->job_op_machines[job][op].find(machine) != this->job_op_machines[job][op].end();
        }
        inline bool isOpFlexibleQck(const int job, const int op) const {
            return this->job_op_machines[job][op].size() > 1;
        }
        int getRandomMachineQck(const int job, const int op, std::default_random_engine & rng_engine) const;
        inline timeperiod::timeunit getDelayQck(const int job, const int op) const {
            return this->job_op_delays[job][op];
        }
        inline timeperiod::timeunit getJobDuration(const int job) const { return job_durations[job]; }
        //dumb methods useful for mutations
        inline int getRandomJob(std::default_random_engine & rng_engine) const {
            return rng_engine() % this->getNJobs();
        }
        inline int getRandomOp(const int job, std::default_random_engine & rng_engine) const {
            return rng_engine() % this->getNOps(job);
        }
        timeperiod::timeunit getTotalDuration() const;
        inline float getJobAvgResourcesQck(const int job) const {
            return this->job_avg_resources[job];
        }
    private:
        std::vector< timeperiod::timeunit > job_durations;
        std::vector< std::vector< timeperiod::timeunit > > job_op_durations;
        std::vector< std::vector< timeperiod::timeunit > > job_op_delays;
        std::vector< float > job_avg_resources; //this vector shall provide me for every job the average number of available resources for each of its operations
        std::vector< std::vector< boost::container::flat_set<int> > > job_op_machines;
        int n_machines;
        int n_ops;
        //just a pair of methods to be invoked at construction
        void buildOpDelays();
        void buildJobDurations();
        void buildNOps();
        void buildJobAvgResources();
    };

}

#endif //INSTANCE_H