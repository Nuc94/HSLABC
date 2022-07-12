#include "../../include/rouflex/instance.h"

rouflex::Instance::Instance(const std::string & filepath) {
    std::ifstream t(filepath);
    std::string str((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());
    nlohmann::json json_data = nlohmann::json::parse(str);
    this->n_machines = 0;
    for(int j = 0; j < json_data["jobs"].size(); ++j) {
        this->job_op_durations.emplace_back( std::vector<timeperiod::timeunit>() );
        this->job_op_machines.emplace_back( std::vector<boost::container::flat_set<int>>() );
        for(int i = 0; i < json_data["jobs"][j]["operations"].size(); ++i) {
            this->job_op_durations.back().push_back( json_data["jobs"][j]["operations"][i]["duration"] );
            this->job_op_machines.back().emplace_back( boost::container::flat_set<int>() );
            for(int m : json_data["jobs"][j]["operations"][i]["machines"]) {
                this->job_op_machines.back().back().insert( m );
                if(m >= this->n_machines) this->n_machines = m + 1;
            }
        }
    }
    this->buildOpDelays();
    this->buildJobDurations();
    this->buildNOps();
    this->buildJobAvgResources();
}

int rouflex::Instance::getRandomMachineQck(const int job, const int op, std::default_random_engine & rng_engine) const {
    auto m_it = this->job_op_machines[job][op].begin();
    if(this->job_op_machines[job][op].size() == 1) return *(m_it);
    m_it += ((rng_engine()) % ( this->job_op_machines[job][op].size() ) );
    return *(m_it);
}

timeperiod::timeunit rouflex::Instance::getTotalDuration() const {
    timeperiod::timeunit total_duration = 0;
    for(int j = 0; j < this->getNJobs(); ++j) {
        for(int op = 0; op < this->getNOps(j); ++op) {
            total_duration += this->getDurationQck(j,op);
        }
    }
    return total_duration;
}

/*  this method shall build the matrix containing for each job which one shall
    be the operation's delay with respect to the start of the job */
void rouflex::Instance::buildOpDelays() {
    int delay_sum;
    for(const auto & ops_durations : this->job_op_durations) {
        this->job_op_delays.emplace_back( std::vector<timeperiod::timeunit>() );
        this->job_op_delays.back().reserve( ops_durations.size() );
        delay_sum = 0;
        for(const auto duration : ops_durations) {
            this->job_op_delays.back().push_back(delay_sum);
            delay_sum += duration;
        }
    }
}

/*  to build a vector containing durations of every job. To be invoked once delays
    vector is built, i.e. after buildOpDelays() */
void rouflex::Instance::buildJobDurations() {
    this->job_durations.reserve(this->getNJobs());
    for(int job = 0; job < this->getNJobs(); ++job) {
        this->job_durations.push_back( this->job_op_delays[job].back() + this->job_op_durations[job].back() );
    }
}

void rouflex::Instance::buildNOps() {
    this->n_ops = 0;
    for(int j = 0; j < this->getNJobs(); ++j) {
        this->n_ops += this->getNOps( j );
    }
}

void rouflex::Instance::buildJobAvgResources() {
    this->job_avg_resources.clear();
    this->job_avg_resources.reserve( this->getNJobs() );
    float avg;
    for( int job = 0; job < this->getNJobs(); ++job ) {
        avg = 0;
        for( int op = 0; op < this->getNOps(job); ++op) avg += static_cast<float>( this->getOpNMachinesQck(job, op) );
        avg = avg / static_cast<float>( this->getNOps(job) );
        this->job_avg_resources.push_back( avg );
    }
    assert( this->job_avg_resources.size() == this->getNJobs() );
}