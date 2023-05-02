//
// Created by svend on 002, 02-05-2023.
//
#include <boost/test/unit_test.hpp>
#include "../util/util.h"
#include "../util/circuitParser.h"
#include "../schemes/threeHalves.h"
#include "../bloodcompatibility.h"

using namespace boost::unit_test;

BOOST_AUTO_TEST_SUITE( Testing_ThreeHalvesRO )
    vector<string> smalltest = {"1 3", "2 1 1", "1 1", "2 1 0 1 2 XOR"};
    auto output = threeHalves::garble(smalltest, 128, util::RO);
    auto F = get<0>(output);
    auto e = get<1>(output);
    auto delta = get<0>(e);
    auto encLabelPairs = get<1>(e);
    auto d = get<2>(output);

    BOOST_AUTO_TEST_CASE( test_Garble )
    {
        BOOST_TEST(get<0>(delta)[0] != 0);
        BOOST_TEST(encLabelPairs.size() == 2);
        BOOST_TEST(d.size() == 1);
    }
BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE( Testing_ThreeHalvesROAnd )
    vector<string> smalltest = {"1 3", "2 1 1", "1 1", "2 1 0 1 2 AND"};
    auto output = threeHalves::garble(smalltest, 128, util::RO);
    auto F = get<0>(output);
    auto e = get<1>(output);
    auto delta = get<0>(e);
    auto encLabelPairs = get<1>(e);
    auto d = get<2>(output);

    BOOST_AUTO_TEST_CASE( test_GarbleAnd )
    {
        BOOST_TEST(get<0>(delta)[0] != 0);
        BOOST_TEST(encLabelPairs.size() == 2);
        BOOST_TEST((get<0>(get<0>(((encLabelPairs))[0])))[0] != 0);
        BOOST_TEST(d.size() == 1);
    }
BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE( Testing_ThreeHalvesROEncode )

    vector<string> smalltest = {"1 3", "2 1 1", "1 1", "2 1 0 1 2 AND"};
    auto output = threeHalves::garble(smalltest, 128, util::RO);
    auto x = vector<int>{1, 1};
    auto e = get<1>(output);
    auto encLabels = threeHalves::encode(e, x);

    BOOST_AUTO_TEST_CASE( test_EncodeSize )
    {
        BOOST_TEST(encLabels.size() == 2);
        BOOST_TEST((get<0>(encLabels[0]))[0] != 0);
        BOOST_TEST((get<0>(encLabels[1]))[0] != 0);
    }

    /*auto d = get<2>(output);
    vector<halfLabels> Y = {{{1}, {1}}}; //input to get the right size output
    auto y = threeHalves::decodeBits(d, Y, smalltest, 128);
    BOOST_AUTO_TEST_CASE( test_Decode )
    {
        BOOST_TEST(y.size() == 1);
    }*/
BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE( Testing_ThreeHalvesROEvalXOR )

    vector<string> smalltest = {"1 3", "2 1 1", "1 1", "2 1 0 1 2 XOR"};
    auto [F,e,d,ic,hash] = threeHalves::garble(smalltest, 128, util::RO);


    BOOST_AUTO_TEST_CASE( test_Eval1xor1 )
    {
        auto x = vector<int>{1, 1};
        auto encLabels = threeHalves::encode(e, x);
        auto Y = threeHalves::eval(F, encLabels, smalltest, 128, ic, hash, util::RO);
        auto y = threeHalves::decodeBits(d, Y, smalltest, 128);
        BOOST_TEST(Y.size() == 1);
        BOOST_TEST(y.size() == 1);
        BOOST_TEST(y[0] == 0);
    }
    BOOST_AUTO_TEST_CASE( test_Eval0xor1 )
    {
        auto x = vector<int>{0, 1};
        auto encLabels = threeHalves::encode(e, x);
        auto Y = threeHalves::eval(F, encLabels, smalltest, 128, ic, hash, util::RO);
        auto y = threeHalves::decodeBits(d, Y, smalltest, 128);
        BOOST_TEST(y.size() == 1);
        BOOST_TEST(y[0] == 1);
    }
    BOOST_AUTO_TEST_CASE( test_Eval1xor0 )
    {
        auto x = vector<int>{1, 0};
        auto encLabels = threeHalves::encode(e, x);
        auto Y = threeHalves::eval(F, encLabels, smalltest, 128, ic, hash, util::RO);
        auto y = threeHalves::decodeBits(d, Y, smalltest, 128);
        BOOST_TEST(y.size() == 1);
        BOOST_TEST(y[0] == 1);
    }
    BOOST_AUTO_TEST_CASE( test_Eval0xor0 )
    {
        auto x = vector<int>{0, 0};
        auto encLabels = threeHalves::encode(e, x);
        auto Y = threeHalves::eval(F, encLabels, smalltest, 128, ic, hash, util::RO);
        auto y = threeHalves::decodeBits(d, Y, smalltest, 128);
        BOOST_TEST(y.size() == 1);
        BOOST_TEST(y[0] == 0);
    }


BOOST_AUTO_TEST_SUITE_END()


BOOST_AUTO_TEST_SUITE( Testing_ThreeHalvesROEvalAND )

    vector<string> smalltest = {"1 3", "2 1 1", "1 1", "2 1 0 1 2 AND"};

    BOOST_AUTO_TEST_CASE( test_Eval1and1 )
    {
        int ctr = 0;
        for (int i = 0; i < 10; ++i) {
            auto [F,e,d,ic, hash] = threeHalves::garble(smalltest, 128, util::RO);
            auto x = vector<int>{1, 1};
            auto encLabels = threeHalves::encode(e, x);
            auto Y = threeHalves::eval(F, encLabels, smalltest, 128, ic, hash, util::RO);
            auto y = threeHalves::decodeBits(d, Y, smalltest, 128);
            BOOST_TEST(y.size() == 1);
            if(y.size() == 1) {
                ctr++;
                BOOST_TEST(y[0] == 1);
            }
        }
    }
BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE( Testing_ThreeHalvesROEvalINV )

    vector<string> smalltest = circuitParser::parseCircuit("../tests/circuits/invTest.txt");

    BOOST_AUTO_TEST_CASE( test_EvalInv0 )
    {
        int ctr = 0;
        for (int i = 0; i < 10; ++i) {
            auto [F,e,d, ic, hash] = threeHalves::garble(smalltest, 128, util::RO);
            auto x = vector<int>{0};
            auto encLabels = threeHalves::encode(e, x);
            auto Y = threeHalves::eval(F, encLabels, smalltest, 128, ic, hash, util::RO);
            auto y = threeHalves::decodeBits(d, Y, smalltest, 128);
            BOOST_TEST(y.size() == 1);
            if(y.size() == 1) {
                ctr++;
                BOOST_TEST(y[0] == 1);
            }
        }
    }
BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE( Testing_ThreeHalvesROEvalXORAND )

    vector<string> xorAnd = circuitParser::parseCircuit("../tests/circuits/xorIntoAnd.txt");

    BOOST_AUTO_TEST_CASE( test_Eval1and1XORAND0 )
    {
        int ctr = 0;
        for (int i = 0; i < 10; ++i) {
            auto [F,e,d, ic, hash] = threeHalves::garble(xorAnd, 128, util::RO);
            auto x = vector<int>{1, 1};
            auto encLabels = threeHalves::encode(e, x);
            auto Y = threeHalves::eval(F, encLabels, xorAnd, 128, ic, hash, util::RO);
            auto y = threeHalves::decodeBits(d, Y, xorAnd, 128);
            BOOST_TEST(y.size() == 1);
            if(y.size() == 1) {
                ctr++;
                BOOST_TEST(y[0] == 0);
            }
        }
    }

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE( Testing_ThreeHalvesROEvalANDXOR )

    vector<string> xorAnd = circuitParser::parseCircuit("../tests/circuits/andIntoXor.txt");

    BOOST_AUTO_TEST_CASE( test_Eval1and1XORAND0 )
    {
        int ctr = 0;
        for (int i = 0; i < 10; ++i) {
            auto [F,e,d, ic, hash] = threeHalves::garble(xorAnd, 128, util::RO);
            auto x = vector<int>{1, 0};
            auto encLabels = threeHalves::encode(e, x);
            auto Y = threeHalves::eval(F, encLabels, xorAnd, 128, ic, hash, util::RO);
            auto y = threeHalves::decodeBits(d, Y, xorAnd, 128);
            BOOST_TEST(y.size() == 1);
            if(y.size() == 1) {
                ctr++;
                BOOST_TEST(y[0] == 1);
            }
        }
    }

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE( ThreeHalvesRO_bloodComp )
    BOOST_AUTO_TEST_CASE( decoding_Y )
    {
        for (int i = 0; i < 100; ++i) {
            int lInput =6; int rInput = 1;
            auto x = vector<int>{1,1,0,0,0,1,1};
            auto C = circuitParser::parseCircuit("../tests/circuits/BloodComp.txt");
            int l = 64;
            auto [F, e, d, ic, hash] = threeHalves::garble(C, 128, util::RO);
            auto encLabels = threeHalves::encode(e, x);
            auto Y = threeHalves::eval(F, encLabels, C, 128, ic, hash, util::RO);
            auto y = threeHalves::decodeBits(d, Y, C, 128);
            ::uint64_t bloodCompAns = bloodcompatibility::bloodCompLookup(lInput,rInput);
            BOOST_TEST(y.size()==1);
            BOOST_TEST(y[0]==bloodCompAns);
        }
    }
BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE( Testing_ThreeHalvesROEvalADDER64 )

    vector<string> bigtest = circuitParser::parseCircuit("../tests/circuits/adder64.txt");
    auto [F,e,d, ic, hash] = threeHalves::garble(bigtest, 128, util::RO);
    auto x = vector<int>{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                         0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                         1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                         0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    auto encLabels = threeHalves::encode(e, x);
    auto Y = threeHalves::eval(F, encLabels, bigtest, 128, ic, hash, util::RO);
    auto y = threeHalves::decodeBits(d, Y, bigtest, 128);

    BOOST_AUTO_TEST_CASE( adder64Adding1And1 )
    {
        BOOST_TEST(Y.size() == 64);
        BOOST_TEST(y.size() == 64);
    }

BOOST_AUTO_TEST_SUITE_END()


BOOST_AUTO_TEST_SUITE( ThreeHalvesRO_adder64bit )

    BOOST_AUTO_TEST_CASE( adder64Adding1And1 )
    {
        //least significant bit first :^)
        auto finput = vector<int>{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                                  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                                  1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                                  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
        auto C = circuitParser::parseCircuit("../tests/circuits/adder64.txt");
        auto [F,e,d,ic, hash] = threeHalves::garble(C, 128, util::RO);
        auto encLabels = threeHalves::encode(e, finput);
        auto Y = threeHalves::eval(F, encLabels, C, 128, ic, hash, util::RO);
        auto y = threeHalves::decodeBits(d, Y, C, 128);

        //util::printUintVec(y);

        BOOST_TEST(y.size()==64);
        for(int i = 0; i < y.size(); i++) {
            if (i != 1) {
                BOOST_TEST(y[i] == 0);
            } else {
                BOOST_TEST(y[i] == 1);
            }
        }
    }

    BOOST_AUTO_TEST_CASE( ThreeHalvesRO_adder64AddingMaxWMax )
    {
        //least significant bit first :^)
        auto maxPlusMax = UINT64_MAX<<1;
        auto finput = vector<int>{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                                  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                                  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                                  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};
        auto C = circuitParser::parseCircuit("../tests/circuits/adder64.txt");
        auto [F,e,d,ic , hash] = threeHalves::garble(C, 128, util::RO);
        auto encLabels = threeHalves::encode(e, finput);
        auto Y = threeHalves::eval(F, encLabels, C, 128, ic , hash, util::RO);
        auto y = threeHalves::decodeBits(d, Y, C, 128);


        BOOST_TEST(y.size()==64);
        for (int i = 0; i < y.size(); i++) {
            if(i!=0){
                BOOST_TEST(y[i]==1);
            } else{
                BOOST_TEST(y[i]==0);
            }
        }
    }

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE( ThreeHalvesRO_sub64 )

    BOOST_AUTO_TEST_CASE( adder64_2minus1 )
    {
        //least significant bit first :^)
        auto finput = vector<int>{0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                                  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                                  1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                                  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
        auto C = circuitParser::parseCircuit("../tests/circuits/sub64.txt");
        auto [F,e,d,ic , hash] = threeHalves::garble(C, 128, util::RO);
        auto encLabels = threeHalves::encode(e, finput);
        auto Y = threeHalves::eval(F, encLabels, C, 128, ic , hash, util::RO);
        auto y = threeHalves::decodeBits(d, Y, C, 128);

        BOOST_TEST(y.size()==64);
        BOOST_TEST(y[0]==1);
    }

    BOOST_AUTO_TEST_CASE( ThreeHalvesRO_adder64_8448minus2051 )
    {
        //least significant bit first :^)
        auto finput = vector<int>{0,0,0,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                                  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                                  1,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                                  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
        auto C = circuitParser::parseCircuit("../tests/circuits/sub64.txt");
        auto [F,e,d,ic , hash] = threeHalves::garble(C, 128, util::RO);
        auto encLabels = threeHalves::encode(e, finput);
        auto Y = threeHalves::eval(F, encLabels, C, 128, ic , hash, util::RO);
        auto y = threeHalves::decodeBits(d, Y, C, 128);
        string s = "1011111100011000000000000000000000000000000000000000000000000000";

        BOOST_TEST(y.size()==64);
        for (int i = 0; i < y.size(); i++) {
            BOOST_TEST(y[i]==s[i]-'0');
        }
    }
BOOST_AUTO_TEST_SUITE_END()