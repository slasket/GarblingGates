
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