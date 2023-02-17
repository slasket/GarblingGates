//
// Created by svend on 017, 17-02-2023.
//
#include <boost/test/unit_test.hpp>
#include "../toyExample/toyGarblingScheme.h"

using namespace boost::unit_test;

BOOST_AUTO_TEST_SUITE( Testing_ToyExampleScheme )
    int k = 1;
    const tuple<int, int, int> &F = toyGarblingScheme::garble(k, 1);
    BOOST_AUTO_TEST_CASE( test_garble )
    {
        BOOST_TEST( get<0>(F) == 1);
        BOOST_TEST( get<1>(F) == 1);
        BOOST_TEST( get<2>(F) == -1);
    }


    int e = get<1>(F);
    int d = get<2>(F);
    int X = toyGarblingScheme::encode(e, 1);
    BOOST_AUTO_TEST_CASE( test_encode )
    {
        BOOST_TEST( X == 2);
    }

    int Y = toyGarblingScheme::eval(get<0>(F), X);
    BOOST_AUTO_TEST_CASE( test_eval )
    {
        BOOST_TEST( Y == 3);
    }

    int y = toyGarblingScheme::decode(d, Y);
    BOOST_AUTO_TEST_CASE( test_decode )
    {
        BOOST_TEST( y == 2);
    }

BOOST_AUTO_TEST_SUITE_END()