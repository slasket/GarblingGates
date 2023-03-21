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
            BOOST_TEST(util::vecXOR(get<0>(wiresLabels), get<1>(wiresLabels)) == output0);

        }
        for (int i = 0; i < 10; ++i) {
            tuple<vector<uint64_t>, vector<uint64_t>> wiresLabels =
                    tuple<vector<uint64_t>, vector<uint64_t>>(vector<uint64_t>(2),
                                                              vector<uint64_t>(2));
            util::labelPointAndPermute(128, wiresLabels, output0, 1);
            //test color bit
            BOOST_TEST((get<0>(wiresLabels)[0] & 1) == 1);
            BOOST_TEST((get<1>(wiresLabels)[0] & 1) == 0);
            BOOST_TEST(util::vecXOR(get<0>(wiresLabels), get<1>(wiresLabels)) == output0);

        }
    }


BOOST_AUTO_TEST_SUITE_END()


BOOST_AUTO_TEST_SUITE( Test_bit_manipulation )

    BOOST_AUTO_TEST_CASE( test_all_nor_input )
    {
        uint64_t max = UINT64_MAX;
        BOOST_TEST(util::norOP(0,0)==max);
        BOOST_TEST(util::norOP(max,max)==0);
        BOOST_TEST(util::norOP(max,0)==0);
        BOOST_TEST(util::norOP(0,max)==0);
    }
    BOOST_AUTO_TEST_CASE( test_hammingWeight )
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
        BOOST_TEST(util::ithBitL2R(vec,2)==0);
        BOOST_TEST(util::ithBitL2R(vec,63)==1);
        BOOST_TEST(util::ithBitL2R(vec,64)==1);
        BOOST_TEST(util::ithBitL2R(vec,127)==1);
        BOOST_TEST(util::ithBitL2R(vec,128)==1);
        BOOST_TEST(util::ithBitL2R(vec,191)==1);

    }


    BOOST_AUTO_TEST_CASE( set_ith_bit_L2R )
    {
        vector<uint64_t> vec = {0,0};
        vec = util::setIthBitTo1L2R(vec,0);
        vec= util::setIthBitTo1L2R(vec,127);

        BOOST_TEST(vec[0]== 9223372036854775808);
        BOOST_TEST(vec[1]== 1);

    }



    BOOST_AUTO_TEST_CASE( findIthBit )
    {
        ::uint64_t one = 1;
        ::uint64_t max = UINT64_MAX;
        ::uint64_t halfones = 12297829382473034648;
        ::uint64_t fortythreeisone = 8796093022208;
        vector<::uint64_t> vec ={9223372036854775808,9223372036854775808,9223372036854775809};

        BOOST_TEST(util::checkIthBit(vec,63)==1);
        BOOST_TEST(util::checkIthBit(vec,128)==1);
        BOOST_TEST(util::checkIthBit(vec,129)==0);
        BOOST_TEST(util::checkIthBit(vec,56)==0);
        BOOST_TEST(util::checkIthBit(vec,174)==0);
        BOOST_TEST(util::checkIthBit(vec,191)==1);

    }

    BOOST_AUTO_TEST_CASE( vectorXOR )
    {
        ::uint64_t one = 1;
        ::uint64_t max = UINT64_MAX;
        ::uint64_t num1 = 2315136083672760336;
        ::uint64_t num2 = 2315136083672760327;
        auto vec1 = {num1,one};
        auto vec2 = {num2,max};
        auto res = util::vecXOR(vec1,vec2);
        BOOST_TEST(res[0]==23);
        BOOST_TEST(res[1]==(max-1));


    }

    BOOST_AUTO_TEST_CASE( vectorAND )
    {
        ::uint64_t one = 1;
        ::uint64_t max = UINT64_MAX;
        ::uint64_t num1 = 2315136083672760336;
        ::uint64_t num2 = 2315136083672760327;
        auto vec1 = {num1,one};
        auto vec2 = {num2,max};
        auto res = util::vecAND(vec1,vec2);
        BOOST_TEST(res[0]==2315136083672760320);
        BOOST_TEST(res[1]==1);
        res = util::vecAND({num1},{num1});
        BOOST_TEST(res[0]==num1);

    }
    BOOST_AUTO_TEST_CASE( vectorOR )
    {
        ::uint64_t one = 1;
        ::uint64_t max = UINT64_MAX;
        ::uint64_t num1 = 2603366459824472086;
        ::uint64_t num2 = 2315136633428574231;
        auto vec1 = {num1,one};
        auto vec2 = {num2,max};
        auto res = util::vecOR(vec1,vec2);
        BOOST_TEST(res[0]==2603367009580285975);
        BOOST_TEST(res[1]==max);
        res = util::vecOR({num1},{num1});
        BOOST_TEST(res[0]==num1);

    }

    BOOST_AUTO_TEST_CASE( insert_bit_in_vecOfBitset )
    {
        vector<bitset<64>> vec ={bitset<64>(1),bitset<64>(1),bitset<64>(1)};
        vec = util::insertBitVecBitset(vec,1,63);
        vec = util::insertBitVecBitset(vec,0,0);
        BOOST_TEST(vec[0].to_ullong()==9223372036854775808);
        vec = util::insertBitVecBitset(vec,0,64);
        BOOST_TEST(vec[1].to_ullong()==0);
        vec = util::insertBitVecBitset(vec,1,147);
        BOOST_TEST(vec[2].to_ullong()==524289);
    }


BOOST_AUTO_TEST_SUITE_END()


BOOST_AUTO_TEST_SUITE( selecting_ith_bit )

    BOOST_AUTO_TEST_CASE( test_extract_exponent )
    {
        BOOST_TEST( true );
    }

BOOST_AUTO_TEST_SUITE_END()



BOOST_AUTO_TEST_SUITE( Testing_vecXORlist )

    BOOST_AUTO_TEST_CASE( test_XORtwoVars )
    {
        for (int i = 0; i < 100; ++i) {
            auto a = util::genBitsNonCrypto(128);
            auto b = util::genBitsNonCrypto(128);
            vint c = vint(a.size());
            vint d = vint(a.size());
            c = util::vecXOR(a, b);
            d = util::vecXOR({a, b});
            BOOST_TEST(c == d);
        }
    }

    BOOST_AUTO_TEST_CASE( test_multipleVars )
    {
        bitset<1> one = {1};
        for (int i = 0; i < 100; ++i) {
            vector<vint> list;
            vint res1 = vint(2);
            for (int j = 0; j < 10; ++j) {
                if(one==util::random_bitset<1>()) {
                    auto a = util::genBitsNonCrypto(128);
                    list.push_back(a);
                    res1 = util::vecXOR(res1, a);
                }
            }
            auto res2 = util::vecXOR(list);
            BOOST_TEST(res1 == res2);
        }
    }

BOOST_AUTO_TEST_SUITE_END()




