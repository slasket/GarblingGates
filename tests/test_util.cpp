//
// Created by svend on 021, 21-02-2023.
//
#include <boost/test/unit_test.hpp>
#include "../util/util.h"
#include "../util/otUtil/otUtil.h"

using namespace boost::unit_test;

BOOST_AUTO_TEST_SUITE( Testing_generateRandomLabels )

    vector<tuple<vector<uint64_t>, vector<uint64_t>>> inputWiresLabels =
            vector<tuple<vector<uint64_t>, vector<uint64_t>>>(2);
    vector<tuple<vector<uint64_t>, vector<uint64_t>>> outputWiresLabels =
            vector<tuple<vector<uint64_t>, vector<uint64_t>>>(2);
    vector<uint64_t> globalDelta = vector<uint64_t>(2);
    auto output = util::generateRandomLabels(128, globalDelta, inputWiresLabels);
    vector<uint64_t> output0 = get<0>(output);
    vector<tuple<vector<uint64_t>, vector<uint64_t>>> output1 = get<1>(output);


    BOOST_AUTO_TEST_CASE( test_globalDelta )
    {
        BOOST_TEST( output0[0] != 0);
        BOOST_TEST( globalDelta[0] != 0);
        BOOST_TEST( (output0[0] & 1) == 1);

    }
    BOOST_AUTO_TEST_CASE( test_labelPointAndPermute )
    {
        tuple<vector<uint64_t>, vector<uint64_t>> wiresLabels =
                tuple<vector<uint64_t>, vector<uint64_t>>(vector<uint64_t>(2),
                                                          vector<uint64_t>(2));
        util::labelPointAndPermute(128, wiresLabels, output0);
        BOOST_TEST(get<0>(wiresLabels)[0] != 0);
        BOOST_TEST(get<0>(wiresLabels)[1] != 0);
        BOOST_TEST(get<1>(wiresLabels)[0] != 0);
        BOOST_TEST(get<1>(wiresLabels)[1] != 0);
        for (int i = 0; i < 10; ++i) {
            tuple<vector<uint64_t>, vector<uint64_t>> wiresLabels =
                    tuple<vector<uint64_t>, vector<uint64_t>>(vector<uint64_t>(2),
                                                              vector<uint64_t>(2));
            util::labelPointAndPermute(128, wiresLabels, output0, 0);
            //test color bit
            BOOST_TEST((get<0>(wiresLabels)[0] & 1) == 0);
            BOOST_TEST((get<1>(wiresLabels)[0] & 1) == 1);
            BOOST_TEST(util::VecXOR(get<0>(wiresLabels), get<1>(wiresLabels)) == output0);

        }
        for (int i = 0; i < 10; ++i) {
            tuple<vector<uint64_t>, vector<uint64_t>> wiresLabels =
                    tuple<vector<uint64_t>, vector<uint64_t>>(vector<uint64_t>(2),
                                                              vector<uint64_t>(2));
            util::labelPointAndPermute(128, wiresLabels, output0, 1);
            //test color bit
            BOOST_TEST((get<0>(wiresLabels)[0] & 1) == 1);
            BOOST_TEST((get<1>(wiresLabels)[0] & 1) == 0);
            BOOST_TEST(util::VecXOR(get<0>(wiresLabels), get<1>(wiresLabels)) == output0);

        }
    }


BOOST_AUTO_TEST_SUITE_END()


BOOST_AUTO_TEST_SUITE( Test_misc_util)

    BOOST_AUTO_TEST_CASE( test_all_nor_input )
    {
        uint64_t max = UINT64_MAX;
        BOOST_TEST(util::norOP(0,0)==max);
        BOOST_TEST(util::norOP(max,max)==0);
        BOOST_TEST(util::norOP(max,0)==0);
        BOOST_TEST(util::norOP(0,max)==0);
    }
    BOOST_AUTO_TEST_CASE( test_ith_bit_L2R )
    {
        ::uint64_t one = 1;
        ::uint64_t max = UINT64_MAX;
        ::uint64_t halfones = 12297829382473034648;
        ::uint64_t onezerozero = 9223372036854776000;
        ::uint64_t twentyisone = 8796093022208;
        ::uint64_t otherhalfones = 6148914691236517824;
        vector<::uint64_t> vec ={9223372036854775809,9223372036854775809,9223372036854775809};

        BOOST_TEST(util::checkBitL2R(one, 63) == 1);
        BOOST_TEST(util::checkBitL2R(onezerozero, 0) == 1);
        BOOST_TEST(util::checkBitL2R(twentyisone,20)==1);
        BOOST_TEST(util::checkBitL2R(halfones,1)==0);
        BOOST_TEST(util::checkBitL2R(halfones,1)==0);
        BOOST_TEST(util::ithBitL2R(vec,0)==1);
        BOOST_TEST(util::ithBitL2R(vec,63)==1);
        BOOST_TEST(util::ithBitL2R(vec,64)==1);
        BOOST_TEST(util::ithBitL2R(vec,127)==1);
        BOOST_TEST(util::ithBitL2R(vec,128)==1);
        BOOST_TEST(util::ithBitL2R(vec,191)==1);

    }
    BOOST_AUTO_TEST_CASE(test_hammingWeight)
    {
        ::uint64_t one = 1;
        ::uint64_t max = UINT64_MAX;
        ::uint64_t halfones = 12297829382473034648;
        ::uint64_t otherhalfones = 6148914691236517824;

        BOOST_TEST(util::hammingWeight(one)==1);
        BOOST_TEST(util::hammingWeight(max)==64);
        BOOST_TEST(util::hammingWeight(halfones)==32);
        //fukcing sadge FIX THIS
        //BOOST_TEST(util::hammingWeight(otherhalfones)==32);

    }


BOOST_AUTO_TEST_SUITE_END()

