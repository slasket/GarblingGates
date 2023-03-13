//
// Created by simon on 08/03/2023.
//
#include <boost/test/unit_test.hpp>
#include "../schemes/threeHalves.h"

using namespace boost::unit_test;

BOOST_AUTO_TEST_SUITE( Testing_ThreeHalves )

    vector<string> smalltest = {"1 3", "2 1 1", "1 1", "2 1 0 1 2 XOR"};
    auto output = threeHalves::garble(128, smalltest);
    auto F = get<0>(output);
    auto e = get<1>(output);
    auto delta = get<0>(e);
    auto encLabelPairs = get<1>(e);
    auto d = get<2>(output);



    BOOST_AUTO_TEST_CASE( test_Garble )
    {
        BOOST_TEST(get<0>(delta)[0] != 0);
        BOOST_TEST(encLabelPairs.size() == 3);
        BOOST_TEST(d.size() == 1);
    }



BOOST_AUTO_TEST_SUITE_END()