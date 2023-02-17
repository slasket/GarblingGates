//
// Created by a on 17/02/2023.
//

#include <boost/test/included/unit_test.hpp>
#include "../util/circuitParser.h"

using namespace boost::unit_test;




BOOST_AUTO_TEST_SUITE( Parser_tests )

    BOOST_AUTO_TEST_CASE( Parsing_blood_circuit )
    {
        auto res = circuitParser::parseCircuit("../tests/circuits/BloodComp.txt");
        auto wires = get<0>(res);
        auto gates = get<1>(res);

        BOOST_TEST(wires.size() == 18);

        BOOST_TEST(gates.size() == 11);

        BOOST_TEST(gates[0] == "3 3 3 1");

        BOOST_TEST(gates[4] == "2 1 2 6 9 XOR");

        BOOST_TEST(gates[13] == "2 1 15 16 17 AND");
    }

BOOST_AUTO_TEST_SUITE_END()