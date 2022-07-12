#include "gtest/gtest.h"
#include "../include/rouflex/timeperiod.h"

TEST( TIMEPERIOD_TEST, IniTest ) {
    timeperiod::TimePeriod TPT(3,5);
}

TEST( TIMEPERIOD_TEST, propoeEPTest ) {
    timeperiod::TimePeriod TPT(3,5);

    ASSERT_EQ( TPT.proposeEP(3, 2) , 3 );

    ASSERT_EQ( TPT.proposeEP(4, 1) , 4 );

    ASSERT_EQ( TPT.proposeEP(5, 0) , 5 );

    ASSERT_EQ( TPT.proposeEP(3, 3) , timeperiod::INVALID_TIME );

    ASSERT_EQ( TPT.proposeEP(2, 3) , timeperiod::INVALID_TIME );

    ASSERT_EQ( TPT.proposeEP(2, 2) , 3 );

    ASSERT_EQ( TPT.proposeEP(2, 1) , 3 );

    ASSERT_EQ( TPT.proposeEP(6, 0) , timeperiod::INVALID_TIME );

    ASSERT_EQ( TPT.proposeEP(6, 1) , timeperiod::INVALID_TIME );

}

TEST( TIMEPERIOD_TEST, occupyCalendar1 ) {
    std::vector<timeperiod::TimePeriod> calendar;

    //I shall initialize the calendar
    calendar.push_back( timeperiod::TimePeriod(0, 5) );
    calendar.push_back( timeperiod::TimePeriod(6, 7) );
    calendar.push_back( timeperiod::TimePeriod(21, 27) );
    calendar.push_back( timeperiod::TimePeriod(30, timeperiod::INFINITE_TIME) );

    timeperiod::occupyCalendar(calendar, 6, 1, 0);

    ASSERT_EQ( calendar[0].getStart() , 0 );
    ASSERT_EQ( calendar[0].getFinish() , 5 );

    ASSERT_EQ( calendar[1].getStart() , 21 );
    ASSERT_EQ( calendar[1].getFinish() , 27 );

    ASSERT_EQ( calendar[2].getStart() , 30 );
    ASSERT_EQ( calendar[2].getFinish() , timeperiod::INFINITE_TIME );
}

TEST( TIMEPERIOD_TEST, occupyCalendar2 ) {
    std::vector<timeperiod::TimePeriod> calendar;

    //I shall initialize the calendar
    calendar.push_back( timeperiod::TimePeriod(0, 5) );
    calendar.push_back( timeperiod::TimePeriod(6, 7) );
    calendar.push_back( timeperiod::TimePeriod(21, 27) );
    calendar.push_back( timeperiod::TimePeriod(30, timeperiod::INFINITE_TIME) );

    timeperiod::occupyCalendar(calendar, 21, 5, 3);

    ASSERT_EQ( calendar[0].getStart() , 0 );
    ASSERT_EQ( calendar[0].getFinish() , 5 );

    ASSERT_EQ( calendar[1].getStart() , 6 );
    ASSERT_EQ( calendar[1].getFinish() , 7 );

    ASSERT_EQ( calendar[2].getStart() , 26 );
    ASSERT_EQ( calendar[2].getFinish() , 27 );

    ASSERT_EQ( calendar[3].getStart() , 30 );
    ASSERT_EQ( calendar[3].getFinish() , timeperiod::INFINITE_TIME );
}

TEST( TIMEPERIOD_TEST, occupyCalendar3 ) {
    std::vector<timeperiod::TimePeriod> calendar;

    //I shall initialize the calendar
    calendar.push_back( timeperiod::TimePeriod(0, 5) );
    calendar.push_back( timeperiod::TimePeriod(6, 7) );
    calendar.push_back( timeperiod::TimePeriod(21, 27) );
    calendar.push_back( timeperiod::TimePeriod(30, timeperiod::INFINITE_TIME) );

    timeperiod::occupyCalendar(calendar, 21, 5, 4);

    ASSERT_EQ( calendar[0].getStart() , 0 );
    ASSERT_EQ( calendar[0].getFinish() , 5 );

    ASSERT_EQ( calendar[1].getStart() , 6 );
    ASSERT_EQ( calendar[1].getFinish() , 7 );

    ASSERT_EQ( calendar[2].getStart() , 26 );
    ASSERT_EQ( calendar[2].getFinish() , 27 );

    ASSERT_EQ( calendar[3].getStart() , 30 );
    ASSERT_EQ( calendar[3].getFinish() , timeperiod::INFINITE_TIME );
}

TEST( TIMEPERIOD_TEST, occupyCalendar4 ) {
    std::vector<timeperiod::TimePeriod> calendar;

    //I shall initialize the calendar
    calendar.push_back( timeperiod::TimePeriod(0, 5) );
    calendar.push_back( timeperiod::TimePeriod(6, 7) );
    calendar.push_back( timeperiod::TimePeriod(21, 27) );
    calendar.push_back( timeperiod::TimePeriod(30, timeperiod::INFINITE_TIME) );

    timeperiod::occupyCalendar(calendar, 32, 4, 8);

    ASSERT_EQ( calendar[0].getStart() , 0 );
    ASSERT_EQ( calendar[0].getFinish() , 5 );

    ASSERT_EQ( calendar[1].getStart() , 6 );
    ASSERT_EQ( calendar[1].getFinish() , 7 );

    ASSERT_EQ( calendar[2].getStart() , 21 );
    ASSERT_EQ( calendar[2].getFinish() , 27 );

    ASSERT_EQ( calendar[3].getStart() , 30 );
    ASSERT_EQ( calendar[3].getFinish() , 32 );

    ASSERT_EQ( calendar[4].getStart() , 36 );
    ASSERT_EQ( calendar[4].getFinish() , timeperiod::INFINITE_TIME );
}

TEST( TIMEPERIOD_TEST, occupyCalendar5 ) {
    std::vector<timeperiod::TimePeriod> calendar;

    //I shall initialize the calendar
    calendar.push_back( timeperiod::TimePeriod(0, 5) );
    calendar.push_back( timeperiod::TimePeriod(6, 7) );
    calendar.push_back( timeperiod::TimePeriod(21, 27) );
    calendar.push_back( timeperiod::TimePeriod(30, timeperiod::INFINITE_TIME) );

    timeperiod::occupyCalendar(calendar, 32, 4, -1);

    ASSERT_EQ( calendar[0].getStart() , 0 );
    ASSERT_EQ( calendar[0].getFinish() , 5 );

    ASSERT_EQ( calendar[1].getStart() , 6 );
    ASSERT_EQ( calendar[1].getFinish() , 7 );

    ASSERT_EQ( calendar[2].getStart() , 21 );
    ASSERT_EQ( calendar[2].getFinish() , 27 );

    ASSERT_EQ( calendar[3].getStart() , 30 );
    ASSERT_EQ( calendar[3].getFinish() , 32 );

    ASSERT_EQ( calendar[4].getStart() , 36 );
    ASSERT_EQ( calendar[4].getFinish() , timeperiod::INFINITE_TIME );
}