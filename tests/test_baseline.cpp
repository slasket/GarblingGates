//
// Created by svend on 020, 20-02-2023.
//
#include <boost/test/unit_test.hpp>
#include "../schemes/baseGarble.h"
#include "../util/circuitParser.h"


using namespace boost::unit_test;

BOOST_AUTO_TEST_SUITE( Testing_BaseLineExtractGate )
    string gateXOR = "2 1 0 6 7 XOR";
    string gateAND = "2 1 7 3 10 AND";
    tuple<vector<int>, vector<int>, string> gateInfoXOR = util::extractGate(gateXOR);
    tuple<vector<int>, vector<int>, string> gateInfoAND = util::extractGate(gateAND);
    vector<int> inputWiresXOR = get<0>(gateInfoXOR);
    vector<int> outputWiresXOR = get<1>(gateInfoXOR);
    string gateTypeXOR = get<2>(gateInfoXOR);
    vector<int> inputWiresAND = get<0>(gateInfoAND);
    vector<int> outputWiresAND = get<1>(gateInfoAND);
    string gateTypeAND = get<2>(gateInfoAND);
    BOOST_AUTO_TEST_CASE( test_inputWires )
    {
        BOOST_TEST(inputWiresXOR[0] == 0);
        BOOST_TEST(inputWiresXOR[1] == 6);
        BOOST_TEST(inputWiresAND[0] == 7);
        BOOST_TEST(inputWiresAND[1] == 3);
    }
    BOOST_AUTO_TEST_CASE( test_outputWires )
    {
        BOOST_TEST(outputWiresXOR[0] == 7);
        BOOST_TEST(outputWiresAND[0] == 10);
    }
    BOOST_AUTO_TEST_CASE( test_gateType )
    {
        BOOST_TEST(gateTypeXOR == "XOR");
        BOOST_TEST(gateTypeAND == "AND");
    }
BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE( Testing_BaseLineCipher )
    string gate = "2 1 0 1 2 XOR";
    tuple<vector<int>, vector<int>, string> gateInfo = util::extractGate(gate);
    vector<int> inputWires = get<0>(gateInfo);
    vector<int> outputWires = get<1>(gateInfo);
    string gateType = get<2>(gateInfo);
    vector<tuple<vector<uint64_t>, vector<uint64_t>>> inputWiresLabels = vector<tuple<vector<uint64_t>, vector<uint64_t>>>(inputWires.size());
    vector<tuple<vector<uint64_t>, vector<uint64_t>>> outputWiresLabels = vector<tuple<vector<uint64_t>, vector<uint64_t>>>(outputWires.size());
    int k = 128;
    auto delta = util::genDelta(k);
    auto labels = util::generateRandomLabels(k, delta, 2);
    int permuteBitA = (int)((get<0>(labels[0])[0]) & 1);
    int permuteBitB = (int)((get<0>(labels[1])[0]) & 1);
    vector<uint64_t> A0;
    vector<uint64_t> A1;
    vector<uint64_t> B0;
    vector<uint64_t> B1;
    hashRTCCR hash = hashRTCCR();

    BOOST_AUTO_TEST_CASE( test_inputWires )
    {
        if (permuteBitA == 1){
            A0 = (get<1>(labels[0]));
            A1 = (get<0>(labels[0]));
        } else {
            A0 = (get<0>(labels[0]));
            A1 = (get<1>(labels[0]));
        }
        if(permuteBitB == 1){
            B0 = (get<1>(labels[1]));
            B1 = (get<0>(labels[1]));
        } else {
            B0 = (get<0>(labels[1]));
            B1 = (get<1>(labels[1]));
        }

        vector<uint64_t> ciphertext;
        vector<uint64_t> gate0;
        vector<uint64_t> gate1;
        baseGarble::andGate(delta, permuteBitA, permuteBitB, A0, A1, B0, B1, ciphertext, gate0, gate1, k, hash);

        if(permuteBitA == 0 | permuteBitB == 0) {
            BOOST_TEST(ciphertext == util::vecXOR(baseGarble::hashFunc(A0, k),
                                                  baseGarble::hashFunc(B0, k)));
            if (permuteBitA == 0 & permuteBitB == 0){
                BOOST_TEST(
                        util::vecXOR(ciphertext, delta) ==
                        util::vecXOR(baseGarble::hashFunc(A1, k),
                                             util::vecXOR(
                                                     baseGarble::hashFunc(
                                                             B1, k),
                                                     util::vecXOR(gate1,
                                                                  util::vecXOR(
                                                                          gate0,
                                                                          A1)))));
            }
            else {
                BOOST_TEST(ciphertext ==
                                   util::vecXOR(baseGarble::hashFunc(A1, k),
                                                util::vecXOR(baseGarble::hashFunc(B1, k),
                                                             util::vecXOR(gate1,
                                                                          util::vecXOR(
                                                                                  gate0,
                                                                                  A1)))));
            }
        } else {
            BOOST_TEST(util::vecXOR(ciphertext, delta) ==
                       util::vecXOR(baseGarble::hashFunc(A0, k),
                                    baseGarble::hashFunc(B0, k)));
            BOOST_TEST(ciphertext == util::vecXOR(baseGarble::hashFunc(A1, k),
                                                  util::vecXOR(baseGarble::hashFunc(B1, k),
                                                               util::vecXOR(gate1,
                                                                            util::vecXOR(gate0,
                                                                                         A1)))));
        }


    }
BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE( Testing_BaseLineOutput )
    auto smalltest = circuitParser::parse("../tests/circuits/xorTest.txt");
    int k = 128;
    auto smallCircuit = baseGarble::garble(smalltest, k, util::fast);
    auto [invConst, garbledCircuit, hash] = get<0>(smallCircuit);

    BOOST_AUTO_TEST_CASE( test_garbledCircuit )
    {
        BOOST_TEST(garbledCircuit.size() == 1); //one gate
    }

    auto bigtest = circuitParser::parse("../tests/circuits/adder64.txt");
    auto [F,e,d] = baseGarble::garble(bigtest, k, util::fast); //tuple(gates, encInputLabels, encOutputLabels)
    auto [bigInvConst, bigGarbledCircuit, bighash] = F;
    auto firstGate = bigGarbledCircuit[0]; //tuple(inputWires, gate0, gate1)

    BOOST_AUTO_TEST_CASE( test_bigGarbledCircuit )
    {
        BOOST_TEST(bigGarbledCircuit.size() == 376); //313+63 gates
        BOOST_TEST((e).size() == 128); //128 input bits
        BOOST_TEST((d).size() == 64); //64 output bits
    }
BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE( Testing_BaseLineEncode )
    auto smalltest = circuitParser::parse("../tests/circuits/xorTest.txt");
    int k = 128;
    auto smallCircuit = baseGarble::garble(smalltest, k, util::fast);
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
    auto bigCircuit = baseGarble::garble(bigtest, k, util::fast);
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

BOOST_AUTO_TEST_SUITE( Testing_BaseLineEvalXOR )
    auto smalltest = circuitParser::parse("../tests/circuits/xorTest.txt");
    int k = 128;
    BOOST_AUTO_TEST_CASE( test_encode_small00 )
    {
        for (int i = 0; i < 10; ++i) {
            auto [F, e, d] = baseGarble::garble(smalltest, k, util::fast);
            auto hash = get<2>(F);
            auto X00 = baseGarble::encode(e, {0, 0});
            auto Y00 = baseGarble::eval(F, X00, smalltest, k);
            auto y00 = baseGarble::decodeBits(d, Y00, k, hash);
            BOOST_TEST(y00[0] == 0);
        }
    }

    BOOST_AUTO_TEST_CASE( test_encode_small01 )
    {
        auto [F,e,d] = baseGarble::garble(smalltest, k, util::fast);
        auto hash = get<2>(F);
        auto X01 = baseGarble::encode(e, {0, 1});
        auto Y01 = baseGarble::eval(F, X01, smalltest, k);
        auto y01 = baseGarble::decodeBits(d, Y01, k, hash);
        BOOST_TEST(y01[0] == 1);
    }

    BOOST_AUTO_TEST_CASE( test_encode_small10 )
    {
        auto [F,e,d] = baseGarble::garble(smalltest, k, util::fast);
        auto hash = get<2>(F);
        auto X10 = baseGarble::encode(e, {1, 0});
        auto Y10 = baseGarble::eval(F, X10, smalltest, k);
        auto y10 = baseGarble::decodeBits(d, Y10, k, hash);
        BOOST_TEST(y10[0] == 1);
    }

    BOOST_AUTO_TEST_CASE( test_encode_small11 )
    {
        auto [F,e,d] = baseGarble::garble(smalltest, k, util::fast);
        auto hash = get<2>(F);
        auto X11 = baseGarble::encode(e, {1, 1});
        auto Y11 = baseGarble::eval(F, X11, smalltest, k);
        auto y11 = baseGarble::decodeBits(d, Y11, k, hash);
        BOOST_TEST(y11[0] == 0);
    }
BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE( Testing_BaseLineEvalAND )
    //vector<string> smalltest = {"1 3", "2 1 1", "1 1", "2 1 0 1 2 AND"};
    //int k = 128;
    //auto [F,e,d] = baseGarble::garble(smalltest, k, util::fast);
    //auto hash = get<2>(F);
    //auto X00 = baseGarble::encode(e, {0, 0});
    //auto Y00 = baseGarble::eval(F, X00, smalltest, k);
    //auto y00 = baseGarble::decodeBits(d, Y00, k, hash);
    BOOST_AUTO_TEST_CASE( test_encode_small00 )
    {
        auto smalltest = circuitParser::parse("../tests/circuits/andTest.txt");
        int k = 128;
        auto [F,e,d] = baseGarble::garble(smalltest, k, util::fast);
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
        auto [F,e,d] = baseGarble::garble(smalltest, k, util::fast);
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
        auto [F,e,d] = baseGarble::garble(smalltest, k, util::fast);
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
        auto [F,e,d] = baseGarble::garble(smalltest, k, util::fast);
        auto hash = get<2>(F);
        auto X11 = baseGarble::encode(e, {1, 1});
        auto Y11 = baseGarble::eval(F, X11, smalltest, k);
        auto y11 = baseGarble::decodeBits(d, Y11, k, hash);
        BOOST_TEST(y11[0] == 1);
    }
BOOST_AUTO_TEST_SUITE_END()


BOOST_AUTO_TEST_SUITE( Testing_BaseLineEvalINV )
    auto smalltest = circuitParser::parse("../tests/circuits/invTest.txt");
    int k = 128;
    BOOST_AUTO_TEST_CASE( test_encode_small0 )
    {
        for (int i = 0; i < 10; ++i) {
            auto [F,e,d] = baseGarble::garble(smalltest, k, util::fast);
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
            auto [F,e,d] = baseGarble::garble(smalltest, k, util::fast);
            auto hash = get<2>(F);
            auto X1 = baseGarble::encode(e, {1});
            auto Y1 = baseGarble::eval(F, X1, smalltest, k);
            auto y1 = baseGarble::decodeBits(d, Y1, k, hash);
            BOOST_TEST(y1[0] == 0);
        }
    }
BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE( Testing_BaseLineEvalTwoGates )
    int k = 128;
    BOOST_AUTO_TEST_CASE( test_encode_two_gates )
    {
        auto smalltest = circuitParser::parse("../tests/circuits/xorIntoAnd.txt");
        auto [F,e,d] = baseGarble::garble(smalltest, k, util::fast);
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
        auto [F,e,d] = baseGarble::garble(smalltest, k, util::fast);
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
        auto [F,e,d] = baseGarble::garble(smalltest, k, util::fast);
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
        auto [F,e,d] = baseGarble::garble(smalltest, k, util::fast);
        auto hash = get<2>(F);
        vector<int> inputLabels = {0, 1, 0, 1};
        auto X = baseGarble::encode(e, inputLabels);
        auto Y = baseGarble::eval(F, X, smalltest, k);
        auto y = baseGarble::decodeBits(d, Y, k, hash);
        BOOST_TEST(y.size() == 1);
        BOOST_TEST(y[0] == 1);
    }
BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE( Testing_BaseLineEvalBlood )
    BOOST_AUTO_TEST_CASE( test_encode_blood )
    {
        int k = 128;
        auto bloodtest = circuitParser::parse("../tests/circuits/BloodComp.txt");
        auto [F,e,d] = baseGarble::garble(bloodtest, k, util::fast);
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

BOOST_AUTO_TEST_SUITE( Testing_BaseLineEvalBig )
    int k = 128;
    auto bigtest = circuitParser::parse("../tests/circuits/adder64.txt");
    auto [F,e,d] = baseGarble::garble(bigtest, k, util::fast);
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

BOOST_AUTO_TEST_SUITE( Testing_BaseLineEvalBigSub )
    int k = 128;
    auto bigtest = circuitParser::parse("../tests/circuits/sub64.txt");
    auto [F,e,d] = baseGarble::garble(bigtest, k, util::fast);
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
