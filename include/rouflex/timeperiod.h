#ifndef TIMEPERIOD_H
#define TIMEPERIOD_H

#include<assert.h>

#include<limits>
#include<vector>
#include<iostream>
#include<algorithm>

/*******************************************************************************************************
 * *****************************************************************************************************
//TIME PERIOD
        Header file containing a class used to represent timeperiods in the Gantt, and several methods useful
        for manipulating/getting results out of vectors of TimePeriods.
        This class is going to constitute a pillar for "resources.h" and "scheduler.h" headers.
 * *****************************************************************************************************
*******************************************************************************************************/

namespace timeperiod {

    using timeunit = int;

    const timeunit INFINITE_TIME = std::numeric_limits<timeunit>::max(); //Way to represent infinite time
    const timeunit INVALID_TIME = -1;

    class TimePeriod {
    public:
        TimePeriod() : start{0}, finish{INFINITE_TIME} {}
        TimePeriod(timeunit i_start, timeunit i_finish) : start{i_start}, finish{i_finish} {/*if(start > finish) {start = -1; finish = -1;}*/}
        inline void delayStart(timeunit additional_time) {start = start + additional_time;} //deprecated
        inline void anticipatefinish(timeunit required_time) {if( ! this->isInfinite()) finish = finish - required_time;} //deprecated
        inline void moveStart(timeunit t) {if( start != INFINITE_TIME ) start = std::max(0, start + t);}
        inline void moveFinish(timeunit t) {if( finish != INFINITE_TIME ) finish = finish + t;}
        inline void boundStart(timeunit t) {start = std::max(t, start);}
        inline timeunit getStart() const {return start;}
        inline timeunit getFinish() const {return finish;}
        inline timeunit getDuration() const {return this->getFinish() - this->getStart();}
        inline bool isInfinite() const {return finish == INFINITE_TIME;}
        inline bool isNever() const { return start == INFINITE_TIME; }  // if the start of a timeperiod is infinite, the the timeperiod represents the concept of "never"
                                                                        // (for example a jobsequence requires two furnace to change the temperature but only one is given,
                                                                        // in this case I consider the job sequence to be scheduled never)
        inline bool makesSense() const {return start <= finish;}    // if a timeperiod finishes before its start I shall assume it doesn't make sense and so the scheduler shall discard it (notice
                                                                    // that this doesn't have to be intended as an anomaly of the program, during scheduling timeperiods can righteously stop
                                                                    // being sensed if the required lavoration can't be allocated inside them)
        inline bool operator> (const TimePeriod & right_operand) const {
            if(right_operand.getFinish() <= start) return true;
            else return false;
        }
        inline bool operator< (const TimePeriod & right_operand) const {
            if(right_operand.getStart() >= finish) return true;
            else return false;
        }
        inline bool operator>= (const TimePeriod & right_operand) const {
            if(right_operand.getFinish() <= finish) return true;
            else return false;
        }
        inline bool operator<= (const TimePeriod & right_operand) const {
            if(right_operand.getFinish() >= finish) return true;
            else return false;
        }
        inline bool operator> (const timeunit right_operand) const {
            if(start > right_operand) return true;
            else return false;
        }
        inline bool operator< (const timeunit right_operand) const {
            if(finish < right_operand) return true;
            else return false;
        }
        inline bool operator>= (const timeunit right_operand) const {
            if(start > right_operand || this->contains(right_operand)) return true;
            else return false;
        }
        inline bool operator<= (const timeunit right_operand) const {
            if(finish < right_operand || this->contains(right_operand)) return true;
            else return false;
        }
        TimePeriod operator+ (const timeunit t) { if(this->isNever()) return *this; if( ! this->isInfinite()) return TimePeriod(start + t, finish + t); else return TimePeriod(start + t, INFINITE_TIME);}
        TimePeriod operator- (const timeunit t) { if(this->isNever()) return *this; if( ! this->isInfinite()) return TimePeriod(std::max(start - t, 0), std::max(finish - t, 0)); else return TimePeriod(std::max(start - t, 0), INFINITE_TIME);}
        inline bool contains(const timeunit operand) const {return (operand <= finish && operand >= start);}
        inline bool contains(const TimePeriod & operand) const {return (operand.getFinish() <= finish && operand.getStart() >= start);}
        inline bool isAttachedTo(const TimePeriod & right_operand) const {return start == right_operand.getFinish();}
        inline bool isConsecutiveTo(const TimePeriod & right_operand) const {return start == right_operand.getFinish() + 1;} //return true if the current time period is right after the one passed in the function
        bool intersect(const TimePeriod & right_operand) const;
        void show() {std::cout << "[ " << start << " - "; if(this->isInfinite()) std:: cout << "inf"; else std::cout << finish; std::cout << " ] "; }
        /*  this method shall serve the purpose of searching for an operation entry point */
        timeunit proposeEP(const timeunit proposedEP, const timeunit duration);
    protected:
        //every timeperiod has a start and a finish expressed in minutes
        timeunit start;
        timeunit finish;
    private:
        inline void setFinish( const timeunit new_finish ) { this->finish = new_finish; };
        friend void occupyCalendar( std::vector<TimePeriod> & calendar,
                                    const timeunit occ_start,
                                    const timeunit occ_duration, int search_point);
    };

/*******************************************************************************************************
 * *****************************************************************************************************
    //STUPID METHODS
 * *****************************************************************************************************
*******************************************************************************************************/

    //method used to obtain the timeperiod representing the intersection of two others
    //WARNING: the method is "blind" and won't execute any check to see if the two timeperiods can actually be intersected
    //(for example {1 - 2} and {3 - 5} can't be intersected. Still the merhod will return a stupid result, it's up to another task
    //to check if the two timeperiods will be able to generate a sensed intersection)
    TimePeriod getIntersection(const TimePeriod & tp1, const TimePeriod & tp2);

    /*******************************************************************************************************
     * *****************************************************************************************************
        //VECTOR METHODS DECLARATIONS
     * *****************************************************************************************************
    *******************************************************************************************************/

    //method used to cout a vector of timeperiods
    void showVector(std::vector<TimePeriod> const & vec);

    //method used to obtain a vector of timeperiods representing the intersection of two other vectors of timeperiods
    std::vector<TimePeriod> intersectVectors(std::vector<TimePeriod> v1, std::vector<TimePeriod> v2);

    // method used to move in time a vector of timeperiods (for example translating [{2 - 5},{6_18}] by 2 will result in:
    // [{4 - 7},{8_20}])
    void traslaVector(std::vector<TimePeriod> & in_vec, timeunit trasl);

    // method used to obtain from a vector of timeperiods the first timeinstant starting from a precise point in time (for example,
    // starting_instant=7 and input = [{2 - 4}, {6 - inf}] will result in 7)
    timeunit getFirstIstantFrom( std::vector<TimePeriod> source_vector, timeunit start_instant);

    //unuseful and incomplete method, still don't what I wanted to do with it
    //bool checkIfAllEqual(std::vector<TimePeriod> source_vector);

    void occupyCalendar(std::vector<TimePeriod> & calendar,
                        const timeunit occ_start, const timeunit occ_duration,
                        int search_point = 0);

}



#endif // TIMEPERIOD_H
