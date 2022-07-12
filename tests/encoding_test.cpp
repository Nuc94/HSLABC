#include "gtest/gtest.h"
#include "../include/rouflex/instance.h"
#include "../include/rouflex/encoding.h"

const std::string TEST_FILEPATH = "/home/nuc/Programmazione/C++/RouFlex/data/instance.json";

TEST(ENCODING_TEST, IniTest) {
    std::default_random_engine reng;

    rouflex::Instance instance(TEST_FILEPATH);

    rouflex::Encoding encode(instance, reng, true, 0.0);

    ASSERT_EQ(encode.isSequencingCorrect(), true);

}

TEST(ENCODING_TEST, CorrecTest) {
    const int TEST_SIZE = 500;
    std::default_random_engine reng;
    rouflex::Instance instance(TEST_FILEPATH);

    for(int i = 0; i < TEST_SIZE; ++i) {
        rouflex::Encoding encode(instance, reng, true, 0.0);
        ASSERT_EQ(encode.isSequencingCorrect(), true);
        ASSERT_EQ(encode.isLegal(), true);
    }
}

TEST(ENCODING_TEST, DeterministicInitTest) {
    const int TEST_SIZE = 500;
    std::default_random_engine reng;
    rouflex::Instance instance(TEST_FILEPATH);

    for(int i = 0; i < TEST_SIZE; ++i) {
        rouflex::Encoding encode(instance, reng, true, 0.0);
        ASSERT_EQ(encode.isResourceAssignmentImposed(), true);
    }
}

TEST(ENCODING_TEST, ShiftJobsTest) {
    const int TEST_SIZE = 500;
    std::default_random_engine reng;
    rouflex::Instance instance(TEST_FILEPATH);
    rouflex::Encoding encode(instance, reng, true, 0.0);

    for(int i = 0; i < TEST_SIZE; ++i) {
        encode.shiftJobs(reng);
        ASSERT_EQ(encode.isSequencingCorrect(), true);
    }
}

TEST(ENCODING_TEST, MachineMutTest) {
    const int TEST_SIZE = 500;
    std::default_random_engine reng;
    rouflex::Instance instance(TEST_FILEPATH);
    rouflex::Encoding encode(instance, reng, true, 0.0);

    for(int i = 0; i < TEST_SIZE; ++i) {
        encode.mutateResource(0.0, reng);
        ASSERT_EQ(encode.isResourceAssignmentImposed(), true);
    }

    for(int i = 0; i < TEST_SIZE; ++i) {
        rouflex::Encoding flexcode(instance, reng, true, 0.0);
        flexcode.mutateResource(1.1, reng);
        ASSERT_EQ(flexcode.isResourceAssignmentImposed(), false);
    }
}

TEST(ENCODING_TEST, PermOtherTest) {
    std::default_random_engine reng;
    rouflex::Instance instance(TEST_FILEPATH);
    rouflex::Encoding encode(instance, reng, true, 0.0);
    encode.logEncoding();

    rouflex::Encoding encode2(instance, reng, true, 0.0);
    encode2.logEncoding();

    encode2.permuteByOther(encode, 4, reng);
    encode2.logEncoding();
}

TEST(ENCODING_TEST, PermWithOtherTest) {
    std::default_random_engine reng;
    rouflex::Instance instance(TEST_FILEPATH);
    const int TEST_SIZE = 500;
    for(int i = 0; i < TEST_SIZE; ++i) {
        rouflex::Encoding encode(instance, reng, true, 0.0);
        rouflex::Encoding encode2(instance, reng, true, 0.0);
        encode2.permuteWithOther(encode, reng);
    }
}