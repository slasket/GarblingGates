#define BOOST_TEST_MODULE MainTest
#include <boost/test/included/unit_test.hpp>
#include "../util/circuitParser.h"
#include "../util/util.h"

using namespace boost::unit_test;




BOOST_AUTO_TEST_SUITE( Parser_tests )

    BOOST_AUTO_TEST_CASE( Parsing_blood_circuit )
    {
        auto res = circuitParser::parseCircuit("../tests/circuits/BloodComp.txt");

        BOOST_TEST(res.size() == 14);

        BOOST_TEST(res[0] == "11 18");

        BOOST_TEST(res[1] == "3 3 3 1");

        BOOST_TEST(res[5] == "2 1 2 6 9 XOR");

        BOOST_TEST(res[13] == "2 1 15 16 17 AND");
    }

BOOST_AUTO_TEST_SUITE_END()