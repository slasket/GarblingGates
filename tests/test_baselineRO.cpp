//
// Created by svend on 020, 20-02-2023.
//
#include <boost/test/unit_test.hpp>
#include "../schemes/baseGarble.h"
#include "../util/circuitParser.h"

using namespace boost::unit_test;

BOOST_AUTO_TEST_SUITE( ROTesting_BaseLineOutput )
    auto smalltest = circuitParser::parse("../tests/circuits/xorTest.txt");
    int k = 128;
    auto smallCircuit = baseGarble::garble(smalltest, k, util::RO);
    auto [invConst, garbledCircuit, hash] = get<0>(smallCircuit);

    BOOST_AUTO_TEST_CASE( test_garbledCircuit )
    {
        BOOST_TEST(garbledCircuit.size() == 1); //one gate
    }

    auto bigtest = circuitParser::parse("../tests/circuits/adder64.txt");
    auto [F,e,d] = baseGarble::garble(bigtest, k, util::RO); //tuple(gates, encInputLabels, encOutputLabels)
    auto [bigInvConst, bigGarbledCircuit, bighash] = F;
    auto firstGate = bigGarbledCircuit[0]; //tuple(inputWires, gate0, gate1)

    BOOST_AUTO_TEST_CASE( test_bigGarbledCircuit )
    {
        BOOST_TEST(bigGarbledCircuit.size() == 376); //313+63 gates
        BOOST_TEST((e).size() == 128); //128 input bits
        BOOST_TEST((d).size() == 64); //64 output bits
    }
BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE( ROTesting_BaseLineEncode )
    auto smalltest = circuitParser::parse("../tests/circuits/xorTest.txt");
    int k = 128;
    auto smallCircuit = baseGarble::garble(smalltest, k, util::RO);
    auto garbledCircuit = get<0>(smallCircuit);
    vector<int> inputLabels = {1};
    auto encodedLabels = baseGarble::encode(get<1>(smallCircuit), inputLabels);
    auto label0 = get<0>(get<1>(smallCircuit)[0]);
    auto label1 = get<1>(get<1>(smallCircuit)[0]);

    BOOST_AUTO_TEST_CASE( test_encode_small )
    {
        BOOST_TEST(encodedLabels.size() == 1); //one gate
        BOOST_TEST(encodedLabels[0].size() == 2); //64*2 bit label
        //correct label
        BOOST_TEST(encodedLabels[0] == label1);
    }
    auto bigtest = circuitParser::parse("../tests/circuits/adder64.txt");
    auto bigCircuit = baseGarble::garble(bigtest, k, util::RO);
    //64 input labels
    vector<int> inputLabelsBig = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                                  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                                  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                                  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                                  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                                  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                                  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                                  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
    auto encodedLabelsBig = baseGarble::encode(get<1>(bigCircuit), inputLabelsBig);
    auto label0Big = get<0>(get<1>(bigCircuit)[0]);
    auto label1Big = get<1>(get<1>(bigCircuit)[0]);
    auto label70Big = get<1>(get<1>(bigCircuit)[70]);
    auto label127Big = get<1>(get<1>(bigCircuit)[127]);

    BOOST_AUTO_TEST_CASE( test_encode_big )
    {
        BOOST_TEST(encodedLabelsBig.size() == 128); //128 input wires
        BOOST_TEST(encodedLabelsBig[0].size() == 2); //64*2 bit label
        //correct label
        BOOST_TEST(encodedLabelsBig[0] == label1Big);
        BOOST_TEST(encodedLabelsBig[70] == label70Big);
        BOOST_TEST(encodedLabelsBig[127] == label127Big);
    }

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE( ROTesting_BaseLineEvalXOR )
    auto smalltest = circuitParser::parse("../tests/circuits/xorTest.txt");
    int k = 128;
    BOOST_AUTO_TEST_CASE( test_encode_small00 )
    {
        for (int i = 0; i < 10; ++i) {
            auto [F, e, d] = baseGarble::garble(smalltest, k, util::RO);
            auto hash = get<2>(F);
            auto X00 = baseGarble::encode(e, {0, 0});
            auto Y00 = baseGarble::eval(F, X00, smalltest, k);
            auto y00 = baseGarble::decodeBits(d, Y00, k, hash);
            BOOST_TEST(y00[0] == 0);
        }
    }

    BOOST_AUTO_TEST_CASE( test_encode_small01 )
    {
        auto [F,e,d] = baseGarble::garble(smalltest, k, util::RO);
        auto hash = get<2>(F);
        auto X01 = baseGarble::encode(e, {0, 1});
        auto Y01 = baseGarble::eval(F, X01, smalltest, k);
        auto y01 = baseGarble::decodeBits(d, Y01, k, hash);
        BOOST_TEST(y01[0] == 1);
    }

    BOOST_AUTO_TEST_CASE( test_encode_small10 )
    {
        auto [F,e,d] = baseGarble::garble(smalltest, k, util::RO);
        auto hash = get<2>(F);
        auto X10 = baseGarble::encode(e, {1, 0});
        auto Y10 = baseGarble::eval(F, X10, smalltest, k);
        auto y10 = baseGarble::decodeBits(d, Y10, k, hash);
        BOOST_TEST(y10[0] == 1);
    }

    BOOST_AUTO_TEST_CASE( test_encode_small11 )
    {
        auto [F,e,d] = baseGarble::garble(smalltest, k, util::RO);
        auto hash = get<2>(F);
        auto X11 = baseGarble::encode(e, {1, 1});
        auto Y11 = baseGarble::eval(F, X11, smalltest, k);
        auto y11 = baseGarble::decodeBits(d, Y11, k, hash);
        BOOST_TEST(y11[0] == 0);
    }
BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE( ROTesting_BaseLineEvalAND )
    //vector<string> smalltest = {"1 3", "2 1 1", "1 1", "2 1 0 1 2 AND"};
    //int k = 128;
    //auto [F,e,d] = baseGarble::garble(smalltest, k, util::RO);
    //auto hash = get<2>(F);
    //auto X00 = baseGarble::encode(e, {0, 0});
    //auto Y00 = baseGarble::eval(F, X00, smalltest, k);
    //auto y00 = baseGarble::decodeBits(d, Y00, k, hash);
    BOOST_AUTO_TEST_CASE( test_encode_small00 )
    {
        auto smalltest = circuitParser::parse("../tests/circuits/andTest.txt");
        int k = 128;
        auto [F,e,d] = baseGarble::garble(smalltest, k, util::RO);
        auto hash = get<2>(F);
        auto X00 = baseGarble::encode(e, {0, 0});
        auto Y00 = baseGarble::eval(F, X00, smalltest, k);
        auto y00 = baseGarble::decodeBits(d, Y00, k, hash);
        BOOST_TEST(y00[0] == 0);
    }


    BOOST_AUTO_TEST_CASE( test_encode_small01 )
    {
        auto smalltest = circuitParser::parse("../tests/circuits/andTest.txt");
        int k = 128;
        auto [F,e,d] = baseGarble::garble(smalltest, k, util::RO);
        auto hash = get<2>(F);
        auto X01 = baseGarble::encode(e, {0, 1});
        auto Y01 = baseGarble::eval(F, X01, smalltest, k);
        auto y01 = baseGarble::decodeBits(d, Y01, k, hash);
        BOOST_TEST(y01[0] == 0);
    }

    BOOST_AUTO_TEST_CASE( test_encode_small10 )
    {
        auto smalltest = circuitParser::parse("../tests/circuits/andTest.txt");
        int k = 128;
        auto [F,e,d] = baseGarble::garble(smalltest, k, util::RO);
        auto hash = get<2>(F);
        auto X10 = baseGarble::encode(e, {1, 0});
        auto Y10 = baseGarble::eval(F, X10, smalltest, k);
        auto y10 = baseGarble::decodeBits(d, Y10, k, hash);
        BOOST_TEST(y10[0] == 0);
    }

    BOOST_AUTO_TEST_CASE( test_encode_small11 )
    {
        auto smalltest = circuitParser::parse("../tests/circuits/andTest.txt");
        int k = 128;
        auto [F,e,d] = baseGarble::garble(smalltest, k, util::RO);
        auto hash = get<2>(F);
        auto X11 = baseGarble::encode(e, {1, 1});
        auto Y11 = baseGarble::eval(F, X11, smalltest, k);
        auto y11 = baseGarble::decodeBits(d, Y11, k, hash);
        BOOST_TEST(y11[0] == 1);
    }
BOOST_AUTO_TEST_SUITE_END()


BOOST_AUTO_TEST_SUITE( ROTesting_BaseLineEvalINV )
    auto smalltest = circuitParser::parse("../tests/circuits/invTest.txt");
    int k = 128;
    BOOST_AUTO_TEST_CASE( test_encode_small0 )
    {
        for (int i = 0; i < 10; ++i) {
            auto [F,e,d] = baseGarble::garble(smalltest, k, util::RO);
            auto hash = get<2>(F);
            auto X0 = baseGarble::encode(e, {0});
            auto Y0 = baseGarble::eval(F, X0, smalltest, k);
            auto y0 = baseGarble::decodeBits(d, Y0, k, hash);
            BOOST_TEST(y0[0] == 1);
        }
    }
    BOOST_AUTO_TEST_CASE( test_encode_small1 )
    {
        for (int i = 0; i < 10; ++i) {
            auto [F,e,d] = baseGarble::garble(smalltest, k, util::RO);
            auto hash = get<2>(F);
            auto X1 = baseGarble::encode(e, {1});
            auto Y1 = baseGarble::eval(F, X1, smalltest, k);
            auto y1 = baseGarble::decodeBits(d, Y1, k, hash);
            BOOST_TEST(y1[0] == 0);
        }
    }
BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE( ROTesting_BaseLineEvalTwoGates )
    int k = 128;
    BOOST_AUTO_TEST_CASE( test_encode_two_gates )
    {
        auto smalltest = circuitParser::parse("../tests/circuits/xorIntoAnd.txt");
        auto [F,e,d] = baseGarble::garble(smalltest, k, util::RO);
        auto hash = get<2>(F);
        vector<int> inputLabels = {0, 0};
        auto X = baseGarble::encode(e, inputLabels);
        auto Y = baseGarble::eval(F, X, smalltest, k);
        auto y = baseGarble::decodeBits(d, Y, k, hash);
        BOOST_TEST(y.size() == 1);
        BOOST_TEST(y[0] == 0);
    }

    BOOST_AUTO_TEST_CASE( test_encode_two_gates2 )
    {
        auto smalltest = circuitParser::parse("../tests/circuits/xorIntoAnd.txt");
        auto [F,e,d] = baseGarble::garble(smalltest, k, util::RO);
        auto hash = get<2>(F);
        vector<int> inputLabels = {0, 1};
        auto X = baseGarble::encode(e, inputLabels);
        auto Y = baseGarble::eval(F, X, smalltest, k);
        auto y = baseGarble::decodeBits(d, Y, k, hash);
        BOOST_TEST(y.size() == 1);
        BOOST_TEST(y[0] == 0);
    }

    BOOST_AUTO_TEST_CASE( test_encode_two_gates3 )
    {
        auto smalltest = circuitParser::parse("../tests/circuits/xorAnd4input.txt");
        auto [F,e,d] = baseGarble::garble(smalltest, k, util::RO);
        auto hash = get<2>(F);
        vector<int> inputLabels = {0, 1, 0, 1};
        auto X = baseGarble::encode(e, inputLabels);
        auto Y = baseGarble::eval(F, X, smalltest, k);
        auto y = baseGarble::decodeBits(d, Y, k, hash);
        BOOST_TEST(y.size() == 1);
        BOOST_TEST(y[0] == 0);
    }

    BOOST_AUTO_TEST_CASE( test_encode_two_gates4 )
    {
        auto smalltest = circuitParser::parse("../tests/circuits/andXor4input.txt");
        auto [F,e,d] = baseGarble::garble(smalltest, k, util::RO);
        auto hash = get<2>(F);
        vector<int> inputLabels = {0, 1, 0, 1};
        auto X = baseGarble::encode(e, inputLabels);
        auto Y = baseGarble::eval(F, X, smalltest, k);
        auto y = baseGarble::decodeBits(d, Y, k, hash);
        BOOST_TEST(y.size() == 1);
        BOOST_TEST(y[0] == 1);
    }
BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE( ROTesting_BaseLineEvalBlood )
    BOOST_AUTO_TEST_CASE( test_encode_blood )
    {
        int k = 128;
        auto bloodtest = circuitParser::parse("../tests/circuits/BloodComp.txt");
        auto [F,e,d] = baseGarble::garble(bloodtest, k, util::RO);
        auto hash = get<2>(F);
        vector<int> inputLabelsBlood = {0, 0, 0,
                                        1, 1, 1,
                                        1};
        auto XBlood = baseGarble::encode(e, inputLabelsBlood);
        auto YBlood = baseGarble::eval(F, XBlood, bloodtest, k);
        auto yBlood = baseGarble::decodeBits(d, YBlood, k, hash);
        BOOST_TEST(yBlood.size() == 1);
        BOOST_TEST(yBlood[0] == 0);
    }
BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE( ROTesting_BaseLineEvalBig )
    int k = 128;
    auto bigtest = circuitParser::parse("../tests/circuits/adder64.txt");
    auto [F,e,d] = baseGarble::garble(bigtest, k, util::RO);
    auto hash = get<2>(F);
    vector<int> inputLabelsBig = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //16
                                  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                                  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                                  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                                  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                                  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                                  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                                  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
    auto XBig = baseGarble::encode(e, inputLabelsBig);
    auto YBig = baseGarble::eval(F, XBig, bigtest, k);
    auto yBig = baseGarble::decodeBits(d, YBig, k, hash);

    BOOST_AUTO_TEST_CASE( test_encode_big_ones )
    {
        BOOST_TEST(yBig.size() == 64); //64 output wires
        for (int i = 1; i < 64; i++) {
            BOOST_TEST(yBig[i] == 1);
        }
        BOOST_TEST(yBig[0] == 0);
    }
    vector<int> inputLabelsBig_zeroes = {
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //16
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1}; //000...000 + 100...000
    auto XBig_zeroes = baseGarble::encode(e, inputLabelsBig_zeroes);
    auto YBig_zeroes = baseGarble::eval(F, XBig_zeroes, bigtest, k);
    auto yBig_zeroes = baseGarble::decodeBits(d, YBig_zeroes, k, hash);

    BOOST_AUTO_TEST_CASE( test_encode_big_zeroes )
    {
        BOOST_TEST(yBig_zeroes.size() == 64); //64 output wires
        for (int i = 0; i < 64; i++) {
            if ( i == 63) {
                BOOST_TEST(yBig_zeroes[i] == 1); // 100...000
            } else
                BOOST_TEST(yBig_zeroes[i] == 0);
        }
    }

    BOOST_AUTO_TEST_CASE( test_encode_big_numbers )
    {
        vector<int> inputLabelsBig_8448plus2051 = vector<int>{0,0,0,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                                                              0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                                                              1,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                                                              0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
        auto XBig_nums = baseGarble::encode(e, inputLabelsBig_8448plus2051);
        auto YBig_nums = baseGarble::eval(F, XBig_nums, bigtest, k);
        auto yBig_nums = baseGarble::decodeBits(d, YBig_nums, k, hash);
        auto nummer = baseGarble::decode(d, YBig_nums, k, hash);
        BOOST_TEST(nummer[0] == 10499);
    }
BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE( ROTesting_BaseLineEvalBigSub )
    int k = 128;
    auto bigtest = circuitParser::parse("../tests/circuits/sub64.txt");
    auto [F,e,d] = baseGarble::garble(bigtest, k, util::RO);
    auto hash = get<2>(F);
    vector<int> inputLabelsBig = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //16
                                  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0,
                                  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1}; //100...000 - 100...000
    auto XBig = baseGarble::encode(e, inputLabelsBig);
    auto YBig = baseGarble::eval(F, XBig, bigtest, 128);
    auto yBig = baseGarble::decodeBits(d, YBig, k, hash);

    BOOST_AUTO_TEST_CASE( test_encode_big_ones )
    {
        BOOST_TEST(yBig.size() == 64); //64 output wires
        for (int i = 0; i < 63; i++) {
            BOOST_TEST(yBig[i] == 0);
        }
        BOOST_TEST(yBig[63] == 1);
    }

    BOOST_AUTO_TEST_CASE( test_encode_big_numbers )
    {
        vector<int> inputLabelsBig_8448minus2051 = vector<int>{0,0,0,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                                                               0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                                                               1,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                                                               0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
        auto XBig_nums = baseGarble::encode(e, inputLabelsBig_8448minus2051);
        auto YBig_nums = baseGarble::eval(F, XBig_nums, bigtest, k);
        auto yBig_nums = baseGarble::decodeBits(d, YBig_nums, k, hash);
        auto nummer = baseGarble::decode(d, YBig_nums, k, hash);
        BOOST_TEST(nummer[0] == 6397);
    }

BOOST_AUTO_TEST_SUITE_END()
