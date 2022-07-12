#ifndef DECODER_H
#define DECODER_H

#include <assert.h>

#include <vector>

#include "timeperiod.h"
#include "instance.h"
#include "encoding.h"

namespace rouflex {

    class Decoder {
    public:
        Decoder() {}
        timeperiod::timeunit decode(Encoding & encode, const bool assign_machines_inplace);
        void logCalendar() const;
        void logFFEPS() const;
    private:
        //this vector shall be the free timeperiods for every machine, with the
        //array postion being the index of the resource
        std::vector< std::vector< timeperiod::TimePeriod > > m_calendar;
        //this matrix shall represent
        std::vector< std::vector< timeperiod::timeunit > > ffeps;
        std::vector<timeperiod::timeunit> epop; //entry points per operation
        std::vector< std::vector< int > > search_points_flex;
        std::vector< int > search_points_stiff;
        inline int getActualMachines() const { return actual_machines; }
        int actual_machines;
        void setMachinesDS(const int n_machines);
        void resetMachinesCalendar();
        timeperiod::timeunit scheduleJob(   const int job, const Instance & data_instance,
                                            Encoding & encode, const bool assign_machines_inplace,
                                            timeperiod::timeunit proposed_entry_point = 0);
        void markInfUnusefulMachines(   const int job, const Instance & data_instance,
                                        const Encoding & encode);
        void timetableEP(   const int job, const int op,
                            timeperiod::timeunit search_start,
                            const Instance & data_instance,
                            const int assigned_resource);
        void timetableEPs(  timeperiod::timeunit search_start, const int job,
                            const Instance & data_instance);
        /*  given an operation and a resource, from a starting instant this method
            shall che */
        timeperiod::timeunit searchFirstOpEP(   const timeperiod::timeunit job_proposed_EP,
                                                const int op, const int res,
                                                const timeperiod::timeunit op_delay,
                                                const timeperiod::timeunit op_duration,
                                                int & search_point );
        void setOpsDS(  const std::vector<int> & resources_assignment,
                        const int actual_ops, const bool flexible_job,
                        const Instance & instance, const int job);
        inline int getActualOps() const { return actual_nops; }
        void assignMachines(const Instance & instance, const int job,
                            std::vector<int> & resources_assignment_vector,
                            const timeperiod::timeunit entry_point) const;
        void occupyResources(   const std::vector<int> original_resources_assignment,
                                const std::vector<int> resources_assignment,
                                const timeperiod::timeunit entry_point,
                                const int job, const Instance & data_instance );
        int actual_nops;
    };

}

#endif //DECODER_H