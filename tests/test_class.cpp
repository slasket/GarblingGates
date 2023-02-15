#include <boost/test/included/unit_test.hpp>
using namespace boost::unit_test;

void free_test_function()
{
    BOOST_TEST( true /* test assertion */ );
}

void scam_test_function()
{
    BOOST_TEST( 1 == 0);
}

void not_scam_test_function()
{
    BOOST_TEST(false);
}

test_suite* init_unit_test_suite( int /*argc*/, char* /*argv*/[] )
{
    framework::master_test_suite().
            add( BOOST_TEST_CASE( &free_test_function ) );
    framework::master_test_suite().
            add( BOOST_TEST_CASE_NAME( &scam_test_function, "1. brick function" ) );
    framework::master_test_suite().
            add( BOOST_TEST_CASE_NAME( &not_scam_test_function, "2. brick function"));
    return 0;
}