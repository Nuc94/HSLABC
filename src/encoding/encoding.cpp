#include "../../include/rouflex/encoding.h"

rouflex::Encoding::Encoding(const Instance & input_instance,
                            std::default_random_engine & rng_engine,
                            const bool assign_machines, const float Pfinit)
                            : instance(input_instance), first_job_imposed_start(0) {
    std::bernoulli_distribution bd(Pfinit);
    const int n_jobs = this->instance.getNJobs();
    int n_ops;
    this->job_order.reserve(n_jobs);
    this->machine_assignments.reserve(n_jobs);
    for(int j = 0; j < n_jobs; ++j) {
        n_ops = this->instance.getNOps(j);
        this->job_order.push_back( j );
        this->machine_assignments.emplace_back( std::vector<int>(n_ops, -1) );
        if( assign_machines ) {
            for(int op = 0; op < n_ops; ++op) {
                if( bd(rng_engine) ) this->machine_assignments.back()[op] = -1;
                else this->machine_assignments.back()[op] = this->instance.getRandomMachineQck(j, op, rng_engine);
            }
        }
    }
    std::shuffle( this->job_order.begin(), this->job_order.end(), rng_engine );
    if(!this->isResourceAssignmentCorrect()) this->logEncoding();
    assert(this->isSequencingCorrect());
    assert(this->isResourceAssignmentCorrect());
}

void rouflex::Encoding::resetByPermutation(const std::vector<int> permutation) {
    this->job_order = permutation;
    assert(this->isSequencingCorrect());
    for(int job = 0; job < this->machine_assignments.size(); ++job) {
        for(int op = 0; op < this->machine_assignments[job].size(); ++op) {
            this->machine_assignments[job][op] = -1;
        }
    }
    this->makespan = timeperiod::INVALID_TIME;
}

/*  this function shall ensure that the job sequencing inside the encoding is
    correct, i.e. all the jobs are present */
bool rouflex::Encoding::isSequencingCorrect() const {
    boost::container::flat_set<int> jobs_set;
    for(int j: this->job_order) jobs_set.insert(j);
    return (*jobs_set.begin()) == 0 && *std::prev(jobs_set.end()) == (this->getInstance().getNJobs() - 1) && (jobs_set.size() == this->getInstance().getNJobs());
}

/*  this method ensures that machine assignment is correct */
bool rouflex::Encoding::isResourceAssignmentCorrect() const {
    for(int job = 0; job < this->machine_assignments.size(); ++job) {
        for(int op = 0; op < this->machine_assignments[job].size(); ++op) {
            if(this->machine_assignments[job][op] != -1) {
                if( ! this->instance.isMachineAtOpQck(job, op, this->machine_assignments[job][op]) ) {
                    return false;
                }
            }
        }
    }
    return true;
}

/*  this method shall ensure that no resource choice is allowed to decoder */
bool rouflex::Encoding::isResourceAssignmentImposed() const {
    for(int job = 0; job < this->machine_assignments.size(); ++job) {
        if( ! this->isResourceAssignmentImposed(job) ) return false;
    }
    return true;
}

bool rouflex::Encoding::isResourceAssignmentImposed(const int job) const {
    for(const int m_ass: this->machine_assignments[job]) {
        if(m_ass == -1) return false;
    }
    return true;
}

void rouflex::Encoding::logEncoding() const {
    std::cout << "Logging encoding information" << std::endl;
    std::cout << "Makespan:" << this->getMakespan() << std::endl;
    std::cout << "Jobs order:" << std::endl;
    for(int job : this->getJobOrder()) std::cout << job << " ";
    std::cout << std::endl;
    std::cout << "Job durations:" << std::endl;
    for(int job = 0; job < this->getInstance().getNJobs(); ++job) std::cout << this->getInstance().getJobDuration(job) << " ";
    std::cout << std::endl;
    std::cout << "Jobs EPs:" << std::endl;
    for(timeperiod::timeunit ep : this->jobEPs) std::cout << ep << " ";
    std::cout << std::endl;
    for(int job = 0; job < this->machine_assignments.size(); ++job) {
        std::cout << "Resources assignments for job" << job << ": ";
        for(int res_assignment: this->machine_assignments[job])  std::cout << res_assignment << " ";
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

void rouflex::Encoding::setJobEPs(std::vector<timeperiod::timeunit> input_jobEPs) {
    this->jobEPs = std::move( input_jobEPs );
    assert( this->jobEPs.size() == this->instance.getNJobs() );
    //an assertion to ensure that no entry point of any job is negative, i.e. invalid
    assert( (*std::min_element(this->jobEPs.begin(), this->jobEPs.end())) >= 0 );
    this->updateMakespan();
}

//most important mutation operator
void rouflex::Encoding::permute(    const Encoding & other, const unsigned int nrp,
                                    const unsigned int nswp, const unsigned int nmc,
                                    const float Pf, const float Pmpi,
                                    std::default_random_engine & reng) {
    std::bernoulli_distribution bd(Pmpi);
    if( bd(reng) ) {
        this->permuteByOther(other, nrp, reng);
    } else {
        this->perturbation(nswp, nmc, Pf, reng);
    }
    if(!this->isResourceAssignmentCorrect()) this->logEncoding();
    assert(this->isSequencingCorrect());
    assert(this->isResourceAssignmentCorrect());
}

void rouflex::Encoding::permuteByOther( const Encoding & other, const unsigned int nrp,
                                        std::default_random_engine & reng) {
    assert(this->job_order.size() == other.getJobOrder().size());
    if(!other.isResourceAssignmentCorrect()) other.logEncoding();
    assert(other.isSequencingCorrect());
    assert(other.isResourceAssignmentCorrect());
    int job_pos, oi = 0;
    boost::container::flat_set<int> jobs_taken;
    std::vector<int> new_job_order = std::vector<int>(this->job_order.size(), -1);
    for(int i = 0; i < nrp; ++i) {
        job_pos = reng() % this->job_order.size();
        new_job_order[job_pos] = other.getJobOrder()[job_pos];
        jobs_taken.insert( new_job_order[job_pos] );
    }
    while( jobs_taken.find( this->job_order[oi] ) != jobs_taken.end() ) ++oi;
    for(int i = 0; i < new_job_order.size(); ++i) {
        if( new_job_order[i] == -1 ) {
            new_job_order[i] = this->job_order[oi];
            ++oi;
            while( oi < this->job_order.size() && jobs_taken.find( this->job_order[oi] ) != jobs_taken.end() ) ++oi;
        }
    }
    this->job_order = std::move(new_job_order);
    for(int j : jobs_taken) {
        this->machine_assignments[j] = other.getMachineAssignmentAtJob(j);
    }
    if(!this->isResourceAssignmentCorrect()) this->logEncoding();
    assert(this->isSequencingCorrect());
    assert(this->isResourceAssignmentCorrect());
}

void rouflex::Encoding::permuteWithOther(Encoding & other, std::default_random_engine & reng) {
    int pos, job;
    std::vector<int> & p1 = this->job_order;
    std::vector<int> & p2 = other.job_order;
    std::vector<int> c1, c2;
    c1 = std::vector<int>(this->instance.getNJobs(), -1);
    c2 = std::vector<int>(this->instance.getNJobs(), -1);
    boost::container::flat_set<int> sub;
    std::bernoulli_distribution bd(0.5);
    for(int i = 0; i < p1.size(); ++i) {
        job = p1[i];
        if( bd(reng) ) {
            sub.insert(job);
            c1[i] = job;
        }
    }
    pos = 0;
    for(int i = 0; i < p2.size(); ++i) {
        job = p2[i];
        if( sub.find(job) != sub.end() ) {
            c2[i] = job;
        } else {
            while(c1[pos] >= 0) ++pos;
            c1[pos] = job;
        }
    }
    pos = 0;
    for(int i = 0; i < p1.size(); ++i) {
        job = p1[i];
        if( sub.find(job) == sub.end() ) {
            while(c2[pos] >= 0) ++pos;
            c2[pos] = job;
        }
    }
    p1 = std::move(c1);
    p2 = std::move(c2);
    assert(this->isSequencingCorrect());
    assert(other.isSequencingCorrect());
}

void rouflex::Encoding::perturbation(   const unsigned int nswp, const unsigned int nmc,
                                        const float Pf, std::default_random_engine & reng) {
    for(int i = 0; i < nswp; ++i) this->shiftJobs(reng);
    for(int i = 0; i < nmc; ++i) this->mutateResource(Pf, reng);
    if(!this->isResourceAssignmentCorrect()) this->logEncoding();
    assert(this->isSequencingCorrect());
    assert(this->isResourceAssignmentCorrect());
}

void rouflex::Encoding::shiftJobsAndRandomizeResources(const float ta_prob, std::default_random_engine & rng_engine) {
    int first_job = this->shiftJobs(rng_engine);
    std::bernoulli_distribution hyb(ta_prob);
    if(ta_prob != 0.0) {
        for(int job = first_job + 1; job < this->instance.getNJobs(); ++job) {
            for(int op = 0; op < this->instance.getNOps(job); ++op) {
                if(this->instance.isOpFlexibleQck(job, op)) {
                    if(hyb(rng_engine)) {
                        this->machine_assignments[job][op] = -1;
                    }
                }
            }
        }
    }
}

int rouflex::Encoding::shiftJobs(std::default_random_engine & rng_engine) {
    int pos0 = rng_engine() % this->job_order.size();
    int pos1 = rng_engine() % this->job_order.size();
    while(pos0 == pos1) pos1 = rng_engine() % this->job_order.size();
    this->shiftJobs(pos0, pos1);
    return std::min(pos0, pos1);
}

void rouflex::Encoding::mutateResource(const float ta_prob, std::default_random_engine & rng_engine) {
    std::bernoulli_distribution bd(ta_prob);
    int job, op, rand_ass;
    //At first I shall select a job op pair allowing for resource flexibility
    job = this->instance.getRandomJob(rng_engine);
    op = this->instance.getRandomOp(job, rng_engine);
    while( ! this->instance.isOpFlexibleQck(job, op) ) {
        job = this->instance.getRandomJob(rng_engine);
        op = this->instance.getRandomOp(job, rng_engine);
    }
    if( bd(rng_engine) ) {
        this->machine_assignments[job][op] = -1;
    }
    else {
        rand_ass = this->instance.getRandomMachineQck(job,op, rng_engine);
        while(rand_ass == this->machine_assignments[job][op]) rand_ass = this->instance.getRandomMachineQck(job,op, rng_engine);
        this->machine_assignments[job][op] = rand_ass;
    }
    if(!this->isResourceAssignmentCorrect()) this->logEncoding();
    assert(this->isSequencingCorrect());
    assert(this->isResourceAssignmentCorrect());
}

void rouflex::Encoding::swapResource(std::default_random_engine & rng_engine) {
    int job, op, rand_ass;
    job = this->instance.getRandomJob(rng_engine);
    op = this->instance.getRandomOp(job, rng_engine);
    while( ! this->instance.isOpFlexibleQck(job, op) ) {
        job = this->instance.getRandomJob(rng_engine);
        op = this->instance.getRandomOp(job, rng_engine);
    }
    rand_ass = this->instance.getRandomMachineQck(job,op, rng_engine);
    while(rand_ass == this->machine_assignments[job][op]) rand_ass = this->instance.getRandomMachineQck(job,op, rng_engine);
    this->machine_assignments[job][op] = rand_ass;
}

void rouflex::Encoding::shiftJobs(const int pos0, const int pos1) {
    assert( this->job_order.size() > pos0 );
    assert( this->job_order.size() > pos1 );
    int jex = this->job_order[pos0];
    this->job_order[pos0] = this->job_order[pos1];
    this->job_order[pos1] = jex;
    if(!this->isResourceAssignmentCorrect()) this->logEncoding();
    assert(this->isSequencingCorrect());
    assert(this->isResourceAssignmentCorrect());
}

void rouflex::Encoding::updateMakespan() {
    this->makespan = 0;
    assert( this->jobEPs.size() == this->instance.getNJobs() );
    //for(auto ep: this->jobEPs) std::cout << ep << " ";
    for(int job = 0; job < this->instance.getNJobs(); ++job) {
        this->makespan = std::max( this->makespan, this->jobEPs[job] + this->instance.getJobDuration(job) );
    }
    //std::cout << std::endl << "makespan: " << this->makespan << std::endl;
}

void rouflex::Encoding::recordGantt(const std::string filepath) const {
    std::ofstream csv_file;
    timeperiod::timeunit start, end;
    csv_file.open(filepath);
    csv_file << "serial,job,op,start,end,resource";
    for(int job = 0; job < this->instance.getNJobs(); ++job) {
        for(int op = 0; op < this->instance.getNOps(job); ++op) {
            csv_file << "\n";
            csv_file << std::to_string(job) << "," << std::to_string(job) << "," << std::to_string(op) << ",";
            start = this->jobEPs[job] + this->getInstance().getDelayQck(job, op);
            end = start + this->getInstance().getDurationQck(job, op);
            csv_file << std::to_string(start) << "," << std::to_string(end) << "," << std::to_string( this->getMachineAssignmentQck(job, op) );
        }
    }
    csv_file.close();
}

//FROM NOW ON THERE ARE METHODS USEFUL TO IMPLEMENT SIMULATED ANNEALING

/*  this method shall return the last completed jobs */
std::vector< int > rouflex::Encoding::getLastCompletedJobs() const {
    std::vector<int> last_completed_jobs;
    timeperiod::timeunit job_completion, last_job_completion = this->getJobEntryPointQck(0) + this->getInstance().getJobDuration(0);
    for(int job = 0; job < this->getInstance().getNJobs(); ++job) {
        job_completion = this->getJobEntryPointQck(job) + this->getInstance().getJobDuration(job);
        if(job_completion == last_job_completion) last_completed_jobs.push_back(job);
        else if (job_completion > last_job_completion) {
            last_job_completion = job_completion;
            last_completed_jobs.clear();
            last_completed_jobs.push_back(job);
        }
    }
    return last_completed_jobs;
}

/*  this method shall return the set of jobs different from the last ones, constituting
    the set of initial uncritical jobs */
std::unordered_set< int > rouflex::Encoding::buildUncriticalJobs(const std::unordered_set< int > & last_jobs) const {
    std::unordered_set< int > uncritical_jobs;
    for(int candidate_job = 0; candidate_job < this->getInstance().getNJobs(); ++candidate_job) {
        if( last_jobs.find( candidate_job ) == last_jobs.end() ) {
            uncritical_jobs.insert( candidate_job );
        }
    }
    assert( (uncritical_jobs.size() + last_jobs.size()) == this->getInstance().getNJobs() );
    return uncritical_jobs;
}

/*  this method shall be used to initialize the set of end times for jobs */
void rouflex::Encoding::buildJobOpEndsByMachine(
    std::unordered_map< int, std::vector< std::vector<timeperiod::timeunit> > > & jobs_ops_ends,
    const std::unordered_set< int > & last_jobs
                                                ) const {
    timeperiod::timeunit job_start, op_end;
    int resource;
    for(int job = 0; job < this->getInstance().getNJobs(); ++job) {
        if( last_jobs.find(job) == last_jobs.end() ) {
            jobs_ops_ends.insert( std::make_pair(job, std::vector< std::vector< timeperiod::timeunit > >() ) );
            jobs_ops_ends[job].reserve( this->getInstance().getNMachines() );
            for(resource = 0; resource < this->getInstance().getNMachines(); ++resource) {
                jobs_ops_ends[job].emplace_back( std::vector<timeperiod::timeunit>() );
            }
            //now for every operation I shall check whenever (and wherever) it ended
            job_start = this->getJobEntryPointQck(job);
            for(int op = 0; op < this->getInstance().getNOps(job); ++op) {
                resource = this->getMachineAssignmentQck(job, op);
                op_end = job_start + this->getInstance().getDelayQck(job, op) + this->getInstance().getDurationQck(job, op);
                jobs_ops_ends[job][resource].push_back( op_end );
            }
        }
    }
}

/*  this method shall add operation starts of critical jobs */
void rouflex::Encoding::updateCriticalJobsOpStarts(
    std::unordered_map< int, std::vector< std::unordered_map< timeperiod::timeunit, int > > > & critical_jobs_opstarts,
    const std::unordered_set< int > & current_critical_jobs
                                                    ) const {
    timeperiod::timeunit job_start, op_start;
    int res;
    for( const int crit_job : current_critical_jobs ) {
        job_start = this->getJobEntryPointQck(crit_job);
        critical_jobs_opstarts.insert( std::make_pair( crit_job, std::vector< std::unordered_map< timeperiod::timeunit, int > >() ) );
        //I proceed to emplace a starting point - operation index unordered_map for every resource
        for(res = 0; res < this->getInstance().getNMachines(); ++res) {
            critical_jobs_opstarts[crit_job].emplace_back( std::unordered_map< timeperiod::timeunit, int >() );
        }
        for(int op = 0; op < this->getInstance().getNOps(crit_job); ++op) {
            res = this->getMachineAssignmentQck(crit_job, op);
            op_start = job_start + this->getInstance().getDelayQck(crit_job, op);
            critical_jobs_opstarts[crit_job][res].insert( std::make_pair( op_start, op ) );
        }
    }
}

/*  this method shall return a map having as keys the indexes of jobs on the critical path,
    and as values the corresponding set of critical operations */
std::unordered_map< int, boost::container::flat_set<int> > rouflex::Encoding::getLongestPathJobs() const {
    std::unordered_map< int, boost::container::flat_set<int> > critical_path_jobs;
    std::unordered_set< int > current_critical_jobs, next_critical_jobs, uncritical_jobs;
    //the following data structure shall contain, for every job which is going to lie on the longest path, a vector with,
    //for all the resources, a map containing pairs with the start of an operation and its index; in such a way I hope
    //to check whether or not an uncritical job's operation precedes one of the critical ones; checks shall be executed
    //only for current critical jobs
    std::unordered_map< int, std::vector< std::unordered_map< timeperiod::timeunit, int > > > critical_jobs_opstarts;
    //the following data structure contains for every job, for every resource, the end times of its operations
    std::unordered_map< int, std::vector< std::vector< timeperiod::timeunit > > > jobs_ops_ends;
    std::unordered_map< timeperiod::timeunit, int >::const_iterator it_op_start;
    //I shall initialize the set of current critical jobs as the result of the search for the last jobs in the schedule
    for( const int last_job : this->getLastCompletedJobs() ) current_critical_jobs.insert( last_job );
    //I shall now initialize the critical path jobs that I'll return
    for( const int crit_job : current_critical_jobs ) {
        critical_path_jobs.insert( std::make_pair(crit_job, boost::container::flat_set<int>()) );
    }
    //The uncritical_jobs are then determined as all the remaining jobs
    uncritical_jobs = this->buildUncriticalJobs( current_critical_jobs );
    //I shall then initialize the job_ops_ends data structure
    this->buildJobOpEndsByMachine(jobs_ops_ends, current_critical_jobs);
    do {
        next_critical_jobs.clear();
        this->updateCriticalJobsOpStarts( critical_jobs_opstarts, current_critical_jobs );
        for(const int un_job : uncritical_jobs) {
            for(int res = 0; res < this->getInstance().getNMachines(); ++res) {
                for(const timeperiod::timeunit op_end : jobs_ops_ends[un_job][res]) {
                    for(const int crit_job : current_critical_jobs) {
                        it_op_start = critical_jobs_opstarts[crit_job][res].find( op_end );
                        if( it_op_start != critical_jobs_opstarts[crit_job][res].end() ) {
                            //I shall then insert the current operation to the unordered map
                            if( critical_path_jobs.find( crit_job ) == critical_path_jobs.end() ) {
                                critical_path_jobs.insert( std::make_pair( crit_job, boost::container::flat_set<int>() ) );
                            }
                            critical_path_jobs[crit_job].insert( it_op_start->second );
                            next_critical_jobs.insert(un_job);
                        }
                    }
                }
            }
        }
        //I shall now remove the newly found critical jobs at this stage from the overall uncritical_jobs set
        for(int next_crit_job : next_critical_jobs) uncritical_jobs.erase( next_crit_job );
        if( ! next_critical_jobs.empty() ) current_critical_jobs = next_critical_jobs;
    } while ( ! next_critical_jobs.empty() );
    //In the end I shall add as critical operations the first ones for the last found jobs
    for( int crit_job : current_critical_jobs ) {
        if( critical_path_jobs.find( crit_job ) == critical_path_jobs.end() ) {
            critical_path_jobs.insert( std::make_pair( crit_job, boost::container::flat_set<int>() ) );
        }
        critical_path_jobs[crit_job].insert( 0 );
    }
    //Finally, I return the map of critical jobs
    assert( ! critical_path_jobs.empty() );
    return critical_path_jobs;
}

std::unordered_map<int, timeperiod::timeunit> rouflex::Encoding::buildMachineLoads(int job_to_rebalance) const {
    std::unordered_map<int, timeperiod::timeunit> machine_loads;
    //I shall check flexible resources whose loads I need for the rebelance in the SA
    for(int op = 0; op < this->getInstance().getNOps(job_to_rebalance); ++op) {
        if( this->getInstance().isOpFlexibleQck(job_to_rebalance, op) ) {
            for(int res : this->getInstance().getOpMachinesQck(job_to_rebalance, op) ) {
                if( machine_loads.find(res) == machine_loads.end() ) machine_loads.insert( std::make_pair(res, 0) );
            }
        }
    }
    //now I shall calculate the loads
    for(int job = 0; job < this->getInstance().getNJobs(); ++job) {
        for(int op = 0; op < this->getInstance().getNOps(job); ++op) {
            int resource = this->getMachineAssignmentQck(job, op);
            if( machine_loads.find(resource) != machine_loads.end() ) machine_loads[resource] += this->getInstance().getDurationQck(job, op);
        }
    }
    return machine_loads;
}

void rouflex::Encoding::rebalanceSingleSAMachine(   const std::unordered_map<int, timeperiod::timeunit> & machine_loads,
                                                    int job_to_rebalance, int op, std::default_random_engine & rengine) {
    timeperiod::timeunit total_resources_load = 0;
    double n_resources_minus_one;
    double resource_cumulated_prob = 0;
    double prob;
    std::uniform_real_distribution<double> unif(0, 1);
    std::vector< std::pair<double, int> > resources_cumulated_probs;
    const boost::container::flat_set<int> & ops_resources = this->getInstance().getOpMachinesQck(job_to_rebalance, op);
    int selection_complete = false;
    n_resources_minus_one = static_cast<double>( this->getInstance().getOpNMachinesQck(job_to_rebalance, op) ) - static_cast<double>(1.0);
    for( int res : ops_resources ) total_resources_load += machine_loads.at(res);
    for( int res : ops_resources ) {
        resources_cumulated_probs.emplace_back( std::make_pair(resource_cumulated_prob, res) );
        resource_cumulated_prob += static_cast<double>( machine_loads.at(res) ) / ( n_resources_minus_one * static_cast<double>( total_resources_load ) );        
    }
    prob = unif(rengine);
    for(int i = 0; i < resources_cumulated_probs.size() && ! selection_complete; ++i) {
        if( prob >= resources_cumulated_probs[i].first ) {
            this->machine_assignments[job_to_rebalance][op] = resources_cumulated_probs[i].second;
            selection_complete = true;
        }
    }
    if( ! selection_complete ) this->machine_assignments[job_to_rebalance][op] = resources_cumulated_probs.back().second;
}

void rouflex::Encoding::rebalanceResourcesSA( int job_to_rebalance, std::default_random_engine & rengine ) {
    std::unordered_map<int, timeperiod::timeunit> machine_loads = this->buildMachineLoads( job_to_rebalance );
    for(int op = 0; op < this->getInstance().getNOps(job_to_rebalance); ++op) {
        if( this->getInstance().isOpFlexibleQck(job_to_rebalance, op) ) this->rebalanceSingleSAMachine(machine_loads, job_to_rebalance, op, rengine);
    }
}

void rouflex::Encoding::reorderJobsSA(int job_to_reinsert) {
    /*std::vector< std::pair<int, timeperiod::timeunit> > jobs_and_start;
    jobs_and_start.reserve( this->getInstance().getNJobs() - 1 );
    for(int job = 0; job < this->getInstance().getNJobs(); ++job) {
        if(job != job_to_reinsert) jobs_and_start.emplace_back( std::make_pair( job, this->getJobEntryPointQck(job) ) );
    }
    std::sort( jobs_and_start.begin() )ù*/

    bool job_inversion_executed = false;
    int job_exchange_support;
    std::vector<int>::iterator it = this->job_order.begin();
    for(int i = 1; i < this->job_order.size() && ! job_inversion_executed; ++i) {
        if(this->job_order[i] == job_to_reinsert) {
            job_exchange_support = this->job_order[i];
            this->job_order[i] = this->job_order[0];
            this->job_order[0] = job_exchange_support;
            job_inversion_executed = true;
        }
    }
    std::advance(it, 1);
    std::sort(it, this->job_order.end(), [this](const int & job1, const int & job2) -> bool
    { 
        return this->getJobEntryPointQck(job1) < this->getJobEntryPointQck(job2); 
    });
    //now I shall ensure everything to be correct
    assert( this->isSequencingCorrect() );
    for(int i = 2; i < this->job_order.size(); ++i) {
        assert( this->getJobEntryPointQck( this->job_order[i] ) >= this->getJobEntryPointQck( this->job_order[i - 1] ) );
    }
}

/*  this method shall return a vector of neighbors for a given encoding */
std::vector< std::unique_ptr<rouflex::Encoding/*Encoding*/> > rouflex::Encoding::getSANeighbors( std::default_random_engine & rengine ) {
    timeperiod::timeunit job_start, op_end, reinsertion_point;
    int job_to_reinsert, random_job_pos, resource;
    std::vector< std::unique_ptr<Encoding> > neighbors;
    std::unordered_map< int, std::vector<timeperiod::timeunit> > entry_points_builder;
    std::unordered_set< timeperiod::timeunit > reentry_points;
    std::unordered_map< int, boost::container::flat_set<int> > critical_path = this->getLongestPathJobs();
    //I start by inserting 0 as a valid reinsertion point
    reentry_points.insert(0);
    //I shall randomly select a job to reinsert
    random_job_pos = rengine() % critical_path.size();
    //I shall then select the job to reinsert
    auto it_job_to_reinsert = critical_path.begin();
    std::advance(it_job_to_reinsert, random_job_pos);
    job_to_reinsert = it_job_to_reinsert->first;
    //now before everything else I shall rebalance resources as in SA
    this->rebalanceResourcesSA(job_to_reinsert, rengine);
    //I shall then check the resources whose critical operations are assigned
    for(int crit_op : it_job_to_reinsert->second) {
        resource = this->getMachineAssignmentQck(job_to_reinsert, crit_op);
        if( entry_points_builder.find(resource) == entry_points_builder.end() ) {
            entry_points_builder.insert( std::make_pair( resource, std::vector<timeperiod::timeunit>() ) );
        }
        //I should then add the delay of the operation and add it to the second vector
        entry_points_builder[resource].push_back( this->getInstance().getDelayQck(job_to_reinsert, crit_op) );
    }
    //for all the remaining jobs, I shall insert their operation end times if they belong to resources for critical operations
    for(int other_job = 0; other_job < this->getInstance().getNJobs(); ++other_job) {
        if(other_job != job_to_reinsert) {
            job_start = this->getJobEntryPointQck( other_job );
            for(int op = 0; op < this->getInstance().getNOps(other_job); ++op) {
                resource = this->getMachineAssignmentQck(other_job, op);
                if( entry_points_builder.find(resource) != entry_points_builder.end() ) {
                    op_end = job_start + this->getInstance().getDelayQck(other_job, op) + this->getInstance().getDurationQck(other_job, op);
                    for(timeperiod::timeunit crit_op_del : entry_points_builder[resource] ) {
                        reinsertion_point = op_end - crit_op_del;
                        if(reinsertion_point > 0) reentry_points.insert( reinsertion_point );
                    }
                }
            }
        }
    }
    this->reorderJobsSA(job_to_reinsert);
    for(timeperiod::timeunit rp : reentry_points) {
        neighbors.emplace_back( std::make_unique<Encoding>( *this ) );
        neighbors.back()->assignFirstJobImposedStart( rp );
    }    
    return neighbors;
}

//METHODS TO REORDER JOBS ACCORDING TO DISPATCHING RULES

void rouflex::Encoding::dispatchLNO() {
    std::sort(this->job_order.begin(), this->job_order.end(), [this](const int & job1, const int & job2) -> bool
    { 
        return this->getInstance().getNOps(job1) > this->getInstance().getNOps(job2); 
    });
}

void rouflex::Encoding::dispatchLJD() {
    std::sort(this->job_order.begin(), this->job_order.end(), [this](const int & job1, const int & job2) -> bool
    { 
        return this->getInstance().getJobDuration(job1) > this->getInstance().getJobDuration(job2); 
    });
}

void rouflex::Encoding::dispatchLRA() {
    std::sort(this->job_order.begin(), this->job_order.end(), [this](const int & job1, const int & job2) -> bool
    { 
        return this->getInstance().getJobAvgResourcesQck(job1) < this->getInstance().getJobAvgResourcesQck(job2); 
    });
}