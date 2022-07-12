#include "../../include/rouflex/decoder.h"

timeperiod::timeunit rouflex::Decoder::decode(  Encoding & encode,
                                                const bool assign_machines_inplace) {
    bool first_job_entry_point_imposed = false;
    const Instance & data_instance = encode.getInstance();
    std::vector<timeperiod::timeunit> jobEPs( data_instance.getNJobs(), timeperiod::INVALID_TIME );
    this->setMachinesDS(data_instance.getNMachines());
    assert(data_instance.getNJobs() == encode.getJobOrder().size());
    for(const int job : encode.getJobOrder()) {
        //std::cout << job << std::endl;
        if( ! first_job_entry_point_imposed ) {
            jobEPs[job] = this->scheduleJob(job, data_instance, encode, assign_machines_inplace, encode.getFirstJobImposedStart());
            first_job_entry_point_imposed = true;
        }
        else {
            jobEPs[job] = this->scheduleJob(job, data_instance, encode, assign_machines_inplace, 0);
        }
    }
    encode.setJobEPs( std::move(jobEPs) );

    if(!encode.isResourceAssignmentCorrect()) encode.logEncoding();
    assert(encode.isSequencingCorrect());
    assert(encode.isResourceAssignmentCorrect());
    //anyway, I want to ensure the first job start is reset by default
    encode.assignFirstJobImposedStart(0);
    return encode.getMakespan();
}

void rouflex::Decoder::setMachinesDS( const int n_machines ) {
    this->actual_machines = n_machines;
    if( this->m_calendar.size() < this->actual_machines ) {
        this->m_calendar.resize( this->actual_machines );
    }
    this->resetMachinesCalendar();
}

void rouflex::Decoder::resetMachinesCalendar() {
    assert(this->getActualMachines() == this->m_calendar.size());
    for(int i = 0; i < this->getActualMachines(); ++i) {
        this->m_calendar[i].clear();
        this->m_calendar[i].emplace_back( timeperiod::TimePeriod(0, timeperiod::INFINITE_TIME) );
    }
}

/*  this method schedules a job by determining its entry point and routing,
    then occupying decoder resources calendars and finally returning the entry_point */
timeperiod::timeunit rouflex::Decoder::scheduleJob(const int job, const Instance & data_instance,
                                    Encoding & encode,
                                    const bool assign_machines_inplace,
                                    timeperiod::timeunit proposed_entry_point) {
    timeperiod::timeunit entry_point = proposed_entry_point;
    int worst_index, epop_index; //they are indexes referring an operation
    std::vector<int> optional_res_assignment;   //this is an optional vector for the
                                                //case I'd like to return the resources
                                                //assignment instead of modifying the encoder
    const std::vector<int> & original_resource_assignments = encode.getMachineAssignmentAtJob(job);
    this->setOpsDS( original_resource_assignments, data_instance.getNOps(job),
                    ! encode.isResourceAssignmentImposed(job),
                    data_instance, job );
    assert(this->epop.size() == this->getActualOps());
    worst_index = 0;
    epop_index = 0;
    do {
        //std::cout << "ITER START" << std::endl;
        //std::cout << "epop_index " << epop_index << std::endl;
        //std::cout << "op delay " << data_instance.getDelayQck(job, epop_index) << std::endl;
        //std::cout << "epop " << this->epop[ epop_index ] << std::endl;
        if( this->epop[ epop_index ] < entry_point ) {
            this->timetableEP(job, epop_index, entry_point, data_instance, original_resource_assignments[epop_index]);
        }
        //std::cout << "new epop " << this->epop[ epop_index ] << std::endl;
        if( this->epop[ epop_index ] > entry_point ) {
            worst_index = epop_index;
            entry_point = this->epop[ epop_index ];
        }
        //std::cout << "worst_idex " << worst_index << std::endl;
        //at the end of the loop I shall augment epop index, remaining in the size of epops
        ++epop_index;
        if(epop_index >= this->getActualOps()) epop_index = 0;
        //if(epop_index == worst_index) std::cout << "WOOOOOOOOW" << std::endl;
    } while(epop_index != worst_index);
    //std::cout << "Scheduledsssssssssssssssssssssssssssss" << std::endl;
    //now I shall assign machines
    if(assign_machines_inplace) {
        this->assignMachines( data_instance, job, encode.getMachineAssignmentAtJobModdable(job), entry_point );
    }
    else {
        optional_res_assignment = encode.getMachineAssignmentAtJobModdable(job);
        this->assignMachines( data_instance, job, optional_res_assignment, entry_point );
    }
    //std::cout << "Assigned machines" << std::endl;
    //And now I finally need to occupy the resources with the given job
    const std::vector<int> & assigned_resources_ref = assign_machines_inplace ? encode.getMachineAssignmentAtJob(job) : optional_res_assignment;
    //std::cout << "Going to occupy resources" << std::endl;
    this->occupyResources( original_resource_assignments, assigned_resources_ref, entry_point, job, data_instance );
    //this-logCalendar();
    return entry_point;
    //THIS FUNCTION WILL HAVE TO BE REWRITTEN, TOGETHER WITH ALL OTHERS
    /*timeperiod::timeunit entry_point = 0;
    int actual_worst;
    std::vector<int> optional_res_assignment;   //this is an optional vector for the
                                                //case I'd like to return the resources
                                                //assignment instead of modifying the encoder
    //this->setOpsDS( data_instance.getNOps(job) );
    this->markInfUnusefulMachines(job, data_instance, encode);
    //I need a first run to bring all search points from -1 to the first possible
    //entry point
    this->timetableEPs( 0, job, data_instance );
    while ( ! std::equal(this->epop.begin() + 1, this->epop.end(), this->epop.begin()) ) {
        this->timetableEPs( *std::max_element( this->epop.begin(), this->epop.end() ), job, data_instance );
    }
    entry_point = this->epop.front();
    //this->logFFEPS();
    //I still need to pick resources, and then I shall occupy resources
    if(assign_machines_inplace) {
        this->assignMachines( encode.getMachineAssignmentAtJobModdable(job), entry_point);
    }
    else {
        optional_res_assignment = encode.getMachineAssignmentAtJobModdable(job);
        this->assignMachines(optional_res_assignment, entry_point);
    }
    //And now I finally need to occupy the resources with the given job
    const std::vector<int> & assigned_resources_ref = assign_machines_inplace ? encode.getMachineAssignmentAtJob(job) : optional_res_assignment;
    this->occupyResources( assigned_resources_ref, entry_point, job, data_instance );
    return entry_point;*/
}

/*  This method serves the purpose of setting the data structures aimed at performing
Optimal Job Insertion. This is a method to be invoked at every job schedule to
reset the insertion decision data structures */
void rouflex::Decoder::setOpsDS(    const std::vector<int> & resources_assignment,
                                    const int actual_ops, const bool flexible_job,
                                    const Instance & instance, const int job) {
    int reset_bound;
    this->actual_nops = actual_ops;
    this->search_points_stiff.assign( this->actual_nops, 0 );
    this->epop.assign(actual_ops, timeperiod::INVALID_TIME);
    if(flexible_job) {
        assert(this->search_points_flex.size() == this->ffeps.size());
        if( this->search_points_flex.capacity() < actual_ops ) {
            this->search_points_flex.reserve(actual_ops);
            this->ffeps.reserve(actual_ops);
        }
        reset_bound = std::min( static_cast<int>(this->search_points_flex.size()), actual_ops );
        for(uint i = 0; i < reset_bound; ++i) {
            if( resources_assignment[i] == -1 ) {
                this->search_points_flex[i].assign( instance.getOpNMachinesQck(job, i) , 0 );
                this->ffeps[i].assign( instance.getOpNMachinesQck(job, i) , -1 );
            }
            
        }
        for(uint i = reset_bound; i < actual_ops; ++i) {
            if( resources_assignment[i] == -1 ) {
                this->search_points_flex.emplace_back( std::vector<int>(instance.getOpNMachinesQck(job, i), 0) );
                this->ffeps.emplace_back( std::vector<int>(instance.getOpNMachinesQck(job, i), timeperiod::INVALID_TIME) );
            }
            else {
                this->search_points_flex.emplace_back( std::vector<int>() );
                this->ffeps.emplace_back( std::vector<int>() );
            }
        }
    }
}

/*  this method shall be used at the beginning of an insertion for setting to infinite
    all the machines not assigned to the job */
void rouflex::Decoder::markInfUnusefulMachines( const int job,
                                                const Instance & data_instance,
                                                const Encoding & encode) {
    int mach_ass;
    for( unsigned int op = 0; op < this->getActualOps(); ++op ) {
        mach_ass = encode.getMachineAssignmentQck(job, op);
        if( mach_ass != -1 ) {
            for( unsigned int m = 0; m < mach_ass; ++m) {
                this->ffeps[op][m] = timeperiod::INFINITE_TIME;
            }
            for( unsigned int m = mach_ass + 1; m < this->getActualMachines(); ++m) {
                this->ffeps[op][m] = timeperiod::INFINITE_TIME;
            }
        }
        //if otherwise machine assignment is -1, it means that the reosurce can be
        //chosen by the TA, in this case every unacceptable machine is marked at infinite
        else {
            for( unsigned int m = 0; m < this->getActualMachines(); ++m) {
                if( ! data_instance.isMachineAtOpQck(job, op, m) ) {
                    this->ffeps[op][m] = timeperiod::INFINITE_TIME;
                }
            }
        }        
    }
}

void rouflex::Decoder::timetableEP( const int job, const int op,
                                    timeperiod::timeunit search_start,
                                    const Instance & data_instance,
                                    const int assigned_resource) {
    int res_pos_index = 0; //res_pos_index shall be used to explore indexes of ffeps and search
    const boost::container::flat_set<int> & resources = data_instance.getOpMachinesQck(job, op);
    if(assigned_resource != -1) {
        this->epop[ op ] = this->searchFirstOpEP( search_start, op, assigned_resource, data_instance.getDelayQck(job, op), data_instance.getDurationQck(job, op), this->search_points_stiff[op] );
    }
    else {
        //here I need a method to set ffeps given
        //std::cout << "here" << std::endl;
        //std::cout << this->search_points_flex.size() << std::endl;
        //std::cout << this->ffeps.size() << std::endl;
        assert(resources.size() == this->search_points_flex[op].size());
        assert(resources.size() == this->ffeps[op].size());

        for(auto res: resources) {
            //std::cout << "res " << res << std::endl;
            //std::cout << "res_pos_index " << res_pos_index << std::endl;
            this->ffeps[op][res_pos_index] = this->searchFirstOpEP( search_start, op, res, data_instance.getDelayQck(job, op), data_instance.getDurationQck(job, op), this->search_points_flex[op][res_pos_index] );
            //In the end I shall update the res_pos_index
            ++res_pos_index;
        }
        this->epop[ op ] = *(std::min_element(this->ffeps[op].begin(), this->ffeps[op].end()));
    }
}

void rouflex::Decoder::timetableEPs(
    timeperiod::timeunit search_start, const int job, const Instance & data_instance
    ) {
    //this first loop shall bring in pair all matrix elements to the search_start
    for(int op = 0; op < this->getActualOps(); ++op) {
        for (int res = 0; res < this->getActualMachines(); ++res) {
            if( this->ffeps[op][res] < search_start ) {
                this->ffeps[op][res] = this->searchFirstOpEP( search_start, op, res, data_instance.getDelayQck(job, op), data_instance.getDurationQck(job, op), this->search_points_stiff[op] );
            }
        }
        this->epop[op] = *std::min_element( this->ffeps[op].begin(), this->ffeps[op].end() );
    }
}

/*  this shall be a methods used to, given a resource and an operation, check
    the first entry point for the job which the operation could guarantee on that machine */
timeperiod::timeunit rouflex::Decoder::searchFirstOpEP(
        const timeperiod::timeunit job_proposed_EP, const int op, const int res,
        const timeperiod::timeunit op_delay, const timeperiod::timeunit op_duration,
        int & search_point
    ) {
    assert( this->m_calendar[ res ].back().isInfinite() );
    assert(job_proposed_EP >= 0);
    timeperiod::timeunit op_EP, op_proposed_EP = job_proposed_EP + op_delay;
    //std::cout << "passed delay " << op_delay << std::endl;
    //std::cout << "passed search point " << search_point << std::endl;
    //std::cout << "Starting proposed EP " << op_proposed_EP << std::endl;
    while( (op_EP = this->m_calendar[ res ][ search_point ].proposeEP(op_proposed_EP, op_duration)) == timeperiod::INVALID_TIME ) {
        ++search_point;
    }
    //std::cout << "new search point " << search_point << std::endl;
    //std::cout << "Op proposed EP " << op_EP << std::endl;
    //std::cout << "proposed result " << op_EP - op_delay << std::endl;
    assert(op_EP - op_delay >= 0);
    return op_EP - op_delay;
}

void rouflex::Decoder::assignMachines(
    const Instance & instance, const int job,
    std::vector<int> & resources_assignment_vector,
    const timeperiod::timeunit entry_point
    ) const {
    int n_res;
    for(int op = 0; op < resources_assignment_vector.size(); ++op) {
        //in case I find a flexible resource assignment, I shall decide the resource
        if( resources_assignment_vector[op] == -1 ) {
            n_res = std::distance(this->ffeps[op].begin(), std::find(this->ffeps[op].begin(), this->ffeps[op].end(), entry_point));
            resources_assignment_vector[op] = instance.getNthOpMachineQck(job, op, n_res);
            //std::cout << "Resource chosen: " << resources_assignment_vector[op] << std::endl;
            assert( resources_assignment_vector[op] < this->getActualMachines() );
        }
    }
}

void rouflex::Decoder::occupyResources( 
    const std::vector<int> original_resources_assignment,
    const std::vector<int> resources_assignment,
    const timeperiod::timeunit entry_point,
    const int job, const Instance & data_instance ) {
    //this->logCalendar();
    int res;
    //I shall start this function with some assertions
    assert(resources_assignment.size() == this->getActualOps());
    assert( *std::max_element( resources_assignment.begin(), resources_assignment.end() ) < this->getActualMachines() );
    assert( *std::min_element( resources_assignment.begin(), resources_assignment.end() ) >= 0 );
    //then I shall occupy resources
    for(int op = 0; op < this->getActualOps(); ++op) {
        res = resources_assignment[op];

        timeperiod::occupyCalendar( this->m_calendar[ res ],
                                    entry_point + data_instance.getDelayQck(job, op),
                                    data_instance.getDurationQck(job, op),
                                    this->search_points_stiff[op] );
    }
    //this->logCalendar();
}

void rouflex::Decoder::logCalendar() const {
    std::cout << "Resources calendar:" << std::endl;
    for(int res = 0; res < this->getActualMachines(); ++res) {
        std::cout << "Calendar for resource " << res << ":";
        timeperiod::showVector(this->m_calendar[res]);
    }
}

void rouflex::Decoder::logFFEPS() const {
    std::cout << "First Feasible Entry Points for Resource:" << std::endl;
    for(int op = 0; op < this->getActualOps(); ++op) {
        std::cout << "FFEPS for Operation " << op << ":";
        assert( this->ffeps[op].size() == this->getActualMachines() );
        for(timeperiod::timeunit t : this->ffeps[op] ) {
            if(t == timeperiod::INFINITE_TIME) {
                std::cout << "inf ";
            }
            else {
                std::cout << t << " ";
            }
        }
        std::cout << std::endl;
    }
}
