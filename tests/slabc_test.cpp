#include "gtest/gtest.h"
#include "../include/rouflex/slabc.h"

TEST(SLABC_TEST, EGreedyBestActionTest) {
    
    ASSERT_EQ( rouflex::getActionFromProbEGreedy(5, 2, 0.18, 0.2), 2 );
    ASSERT_EQ( rouflex::getActionFromProbEGreedy(5, 3, 0.18, 0.2), 3 );
    ASSERT_EQ( rouflex::getActionFromProbEGreedy(5, 5, 0.18, 0.2), 5 );

}

TEST(SLABC_TEST, EGreedyRandomActionTest) {
    
    ASSERT_EQ( rouflex::getActionFromProbEGreedy(4, 2, 0.21, 0.2), 0 );
    ASSERT_EQ( rouflex::getActionFromProbEGreedy(4, 3, 0.25, 0.2), 0 );
    ASSERT_EQ( rouflex::getActionFromProbEGreedy(4, 5, 0.45, 0.2), 1 );
    ASSERT_EQ( rouflex::getActionFromProbEGreedy(4, 5, 0.61, 0.2), 2 );
    ASSERT_EQ( rouflex::getActionFromProbEGreedy(4, 5, 0.99999, 0.2), 3 );

}