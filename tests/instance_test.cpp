#include "gtest/gtest.h"
#include "../include/rouflex/instance.h"

const std::string TEST_FILEPATH = "/home/nuc/Programmazione/C++/RouFlex/data/instance.json";

TEST(INSTANCE_TEST, IniTest) {
    rouflex::Instance InTest(TEST_FILEPATH);
}

TEST(INSTANCE_TEST, CorrecTest) {
    rouflex::Instance InTest(TEST_FILEPATH);
    
    ASSERT_EQ(InTest.getNJobs(), 4);

    ASSERT_EQ(InTest.getDurationQck(0,0), 5);
    ASSERT_EQ(InTest.getDurationQck(0,3), 9);

    ASSERT_EQ(InTest.getDurationQck(3,0), 4);
    ASSERT_EQ(InTest.getDurationQck(3,2), 8);

    ASSERT_EQ( InTest.isMachineAtOpQck(0,0,2), true );
    ASSERT_EQ( InTest.isMachineAtOpQck(0,0,0), false );
    ASSERT_EQ( InTest.isMachineAtOpQck(0,0,7), false );

    ASSERT_EQ( *(InTest.getOpMachinesQck(0,0)).begin(), 2 );
    ASSERT_EQ( *(InTest.getOpMachinesQck(0,2)).begin(), 1 );
}

TEST(INSTANCE_TEST, RandoMachineTest) {

    std::default_random_engine e;

    rouflex::Instance InTest(TEST_FILEPATH);
    
    const int N_LOOP = 10;

    for(int j = 0; j < InTest.getNJobs(); ++j) {
        for(int op = 0; op < InTest.getNOps(j); ++op) {
            ASSERT_TRUE( InTest.isMachineAtOpQck(j, op, InTest.getRandomMachineQck(j, op, e) ) );
        }
    }
}

TEST(INSTANCE_TEST, DelayTest) {
    rouflex::Instance InTest(TEST_FILEPATH);

    ASSERT_EQ( InTest.getDelayQck(0, 0) , 0 );
    ASSERT_EQ( InTest.getDelayQck(1, 0) , 0 );
    ASSERT_EQ( InTest.getDelayQck(2, 0) , 0 );
    ASSERT_EQ( InTest.getDelayQck(3, 0) , 0 );

    ASSERT_EQ( InTest.getDelayQck(0, 1) , 5 );
    ASSERT_EQ( InTest.getDelayQck(0, 2) , 9 );
    ASSERT_EQ( InTest.getDelayQck(0, 3) , 16 );

    ASSERT_EQ( InTest.getDelayQck(3, 1) , 4 );
    ASSERT_EQ( InTest.getDelayQck(3, 2) , 12 );

}