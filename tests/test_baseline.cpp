//
// Created by svend on 020, 20-02-2023.
//
#include <boost/test/unit_test.hpp>
#include "../schemes/baseGarble.h"

using namespace boost::unit_test;

BOOST_AUTO_TEST_SUITE( Testing_BaseLineExtractGate )
    string gateXOR = "2 1 0 6 7 XOR";
    string gateAND = "2 1 7 3 10 AND";
    tuple<vector<int>, vector<int>, string> gateInfoXOR = baseGarble::extractGate(gateXOR);
    tuple<vector<int>, vector<int>, string> gateInfoAND = baseGarble::extractGate(gateAND);
    vector<int> inputWiresXOR = get<0>(gateInfoXOR);
    vector<int> outputWiresXOR = get<1>(gateInfoXOR);
    string gateTypeXOR = get<2>(gateInfoXOR);
    vector<int> inputWiresAND = get<0>(gateInfoAND);
    vector<int> outputWiresAND = get<1>(gateInfoAND);
    string gateTypeAND = get<2>(gateInfoAND);
    BOOST_AUTO_TEST_CASE( test_inputWires )
    {
        BOOST_TEST(inputWiresXOR[0] == 0);
        BOOST_TEST(inputWiresXOR[1] == 6);
        BOOST_TEST(inputWiresAND[0] == 7);
        BOOST_TEST(inputWiresAND[1] == 3);
    }
    BOOST_AUTO_TEST_CASE( test_outputWires )
    {
        BOOST_TEST(outputWiresXOR[0] == 7);
        BOOST_TEST(outputWiresAND[0] == 10);
    }
    BOOST_AUTO_TEST_CASE( test_gateType )
    {
        BOOST_TEST(gateTypeXOR == "XOR");
        BOOST_TEST(gateTypeAND == "AND");
    }


BOOST_AUTO_TEST_SUITE_END()