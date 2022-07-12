#include "gtest/gtest.h"
#include "../include/rouflex/instance.h"
#include "../include/rouflex/abc.h"

const std::string TEST_FILEPATH = "/home/nuc/Programmazione/C++/RouFlex/data/instance.json";

TEST(ABC_TEST, IniTest) {
    
    std::default_random_engine reng;

    rouflex::Instance instance(TEST_FILEPATH);

    rouflex::ABC abc(instance, 1000, 50, 200, 30,0.5,0.5,2,2,2,0.2,0.2, reng);

}

TEST(ABC_TEST, IterTest) {
    
    std::default_random_engine reng;

    rouflex::Instance instance(TEST_FILEPATH);

    rouflex::ABC abc(instance, 1000, 50, 200, 30,0.5,0.5,2,2,2,0.2,0.2, reng);

    abc.iterate();

}

TEST(ABC_TEST, SolveTest) {
    
    std::default_random_engine reng;

    rouflex::Instance instance(TEST_FILEPATH);

    rouflex::ABC abc(instance, 1000, 50, 200, 30,0.5,0.5,2,2,2,0.2,0.2, reng);

    abc.solve();

}