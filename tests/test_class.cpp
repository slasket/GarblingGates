
#include <boost/test/unit_test.hpp>
using namespace boost::unit_test;

BOOST_AUTO_TEST_SUITE( Testing_bit_position_function )

    BOOST_AUTO_TEST_CASE( test_does_it_work )
    {
        BOOST_TEST( true );
    }

BOOST_AUTO_TEST_SUITE_END()


BOOST_AUTO_TEST_CASE( test_outside_of_test_suite )
{
    BOOST_TEST( true );
}

BOOST_AUTO_TEST_SUITE( Testing_ElGamalStuff )

    BOOST_AUTO_TEST_CASE( test_extract_exponent )
    {
        BOOST_TEST( true );
    }

BOOST_AUTO_TEST_SUITE_END()

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
