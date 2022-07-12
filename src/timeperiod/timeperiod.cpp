#include "../../include/rouflex/timeperiod.h"

/*******************************************************************************************************
 * *****************************************************************************************************
//TIME PERIOD METHODS
 * *****************************************************************************************************
*******************************************************************************************************/

//method used to check if two timeperiods can be intersected
bool timeperiod::TimePeriod::intersect(const TimePeriod & right_operand) const {
    bool result = false;
    result = result || right_operand.contains(start);
    result = result || right_operand.contains(finish);
    result = result || this->contains(right_operand);
    return result;
}

/*  this method shall serve the purpose of searching for an operation entry point */
timeperiod::timeunit timeperiod::TimePeriod::proposeEP(
    const timeunit proposedEP, const timeunit duration) {
    if(proposedEP + duration > this->getFinish() || duration > this->getDuration()) return INVALID_TIME;
    return std::max( proposedEP , this->getStart() );
}

/*******************************************************************************************************
 * *****************************************************************************************************
//TIME PERIOD METHODS
    //this section will contain the TimePeriod class and encapsulating classes methods
    //(shall be transfered one day to something like TimePeriod.cpp)
 * *****************************************************************************************************
*******************************************************************************************************/

    //STUPID METHODS

timeperiod::TimePeriod timeperiod::getIntersection(const TimePeriod & tp1, const TimePeriod & tp2) {
    //I construct and return on the fly
    return TimePeriod(std::max(tp1.getStart(), tp2.getStart()), std::min(tp1.getFinish(), tp2.getFinish()));
}

    //VECTOR METHODS

std::vector<timeperiod::TimePeriod> timeperiod::intersectVectors(std::vector<TimePeriod> v1, std::vector<TimePeriod> v2) {
    std::vector<TimePeriod> result;
    unsigned long i1, i2; //the two iterators used to explore the vector
    unsigned long size1, size2; //the sizes of the two vectors
    TimePeriod intersection;
    //if either one of the two input vectors is empty, return an empty result vector
    if(v1.empty() || v2.empty()) return result;
    i1 = 0;
    i2 = 0; //I initialize the numerical iterators
    size1 = v1.size();
    size2 = v2.size();
    while(i1 < size1 && i2 < size2) {
        //I obtain the intersection of the two time period vectors, and evaluate it
        intersection = getIntersection(v1.at(i1), v2.at(i2));
        if(intersection.makesSense()) result.push_back(intersection);
        //In the end I decide which timeperiod to check next, based on
        //which time period is already more advanced in future
        if(v1.at(i1) >= v2.at(i2)) ++i2;
        else ++i1;
    }
    return result;
}

void timeperiod::traslaVector(std::vector<TimePeriod> & in_vec, timeunit trasl) {
    for(uint i = 0; i < in_vec.size(); ++i) {
        in_vec.at(i) = in_vec.at(i) + trasl;
    }
}


timeperiod::timeunit timeperiod::getFirstIstantFrom( std::vector<TimePeriod> source_vector, timeunit start_instant) {
    //search through all the time period vector and look for an availability which would allow me to return a result
    //the first availability that I find bigger is used to return the result
    for (auto tp : source_vector) {
        //in case I find an availability beyond the required time, I return the minimum between the start of the availability
        //and the required time instant
        if(tp >= start_instant) return std::max( tp.getStart(), start_instant );
    }
    //in case no satisfying time period is found I just renuturn infinite
    return INFINITE_TIME;

}

/*bool checkIfAllEqual(std::vector<TimePeriod> source_vector) {
    if( source_vector.empty() || source_vector.size() == 1 ) return true;
    for (uint i = 0; i < source_vector.size() - 1; ++i) {

    }
}*/

void timeperiod::showVector(std::vector<TimePeriod> const & vec) {for(auto tp : vec) tp.show(); std::cout << std::endl;}

void timeperiod::occupyCalendar(std::vector<TimePeriod> & calendar,
                                const timeunit occ_start, const timeunit occ_duration,
                                int search_point) {
    timeunit occ_finish = occ_start + occ_duration;
    //std::cout << occ_start << std::endl;
    //std::cout << occ_duration << std::endl;
    //std::cout << occ_finish << std::endl;
    //At the beginning I assert that the last element of the vector has infinite bound
    assert( calendar.back().isInfinite() );
    //at the begoinning I ensure that search point starts from a proper value
    if(search_point < 0) search_point = 0;
    else if(search_point >= calendar.size()) search_point = calendar.size() - 1;
    //after that I shall find the real timeperiood by updating the search_point
    while( ! calendar[ search_point ].contains( occ_start ) ) {
        //std::cout << "here" << std::endl;
        if( occ_start > calendar[ search_point ].getFinish() ) ++search_point;
        else --search_point;
    }
    assert( calendar[ search_point ].contains( occ_finish ) );
    assert( calendar[ search_point ].getDuration() >= occ_duration );
    //now that I have found the timeperiod, I shall update the calendar based on cases
    if( calendar[ search_point ].getStart() == occ_start && calendar[ search_point ].getFinish() == occ_finish ) {
        //in case the occupation would occupy the whole timeperiod, then I shall delete it
        calendar.erase( calendar.begin() + search_point );
    }
    else if ( calendar[ search_point ].getStart() == occ_start ) {
        calendar[ search_point ].moveStart( occ_duration );
    }
    else if ( calendar[ search_point ].getFinish() == occ_finish ) {
        calendar[ search_point ].moveFinish( - occ_duration );
    }
    else {
        calendar.insert( calendar.begin() + search_point + 1, TimePeriod( occ_finish , calendar[ search_point ].getFinish() ) );
        calendar[ search_point ].setFinish(occ_start);
    }
}