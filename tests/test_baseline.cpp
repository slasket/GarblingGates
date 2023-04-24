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
        baseGarble::andGate(delta, permuteBitA, permuteBitB, A0, A1, B0, B1, ciphertext, gate0, gate1, k);

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
    vector<string> smalltest = {"1 3", "2 1 1", "1 1", "2 1 0 1 2 XOR"};
    int k = 128;
    auto smallCircuit = baseGarble::garble(smalltest, 128, util::fast);
    auto [invConst, garbledCircuit] = get<0>(smallCircuit);

    BOOST_AUTO_TEST_CASE( test_garbledCircuit )
    {
        BOOST_TEST(garbledCircuit.size() == 1); //one gate
    }

    vector<string> bigtest = circuitParser::parseCircuit("../tests/circuits/adder64.txt");
    auto bigCircuit = baseGarble::garble(bigtest, 128, util::fast); //tuple(gates, encInputLabels, encOutputLabels)
    auto [bigInvConst, bigGarbledCircuit] = get<0>(bigCircuit);
    auto firstGate = bigGarbledCircuit[0]; //tuple(inputWires, gate0, gate1)

    BOOST_AUTO_TEST_CASE( test_bigGarbledCircuit )
    {
        BOOST_TEST(bigGarbledCircuit.size() == 376); //313+63 gates
        BOOST_TEST(get<1>(bigCircuit).size() == 128); //128 input bits
        BOOST_TEST(get<2>(bigCircuit).size() == 64); //64 output bits
    }
BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE( Testing_BaseLineEncode )
    vector<string> smalltest = {"1 3", "2 1 1", "1 1", "2 1 0 1 2 XOR"};
    int k = 128;
    auto smallCircuit = baseGarble::garble(smalltest, 128, util::fast);
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
    vector<string> bigtest = circuitParser::parseCircuit("../tests/circuits/adder64.txt");
    auto bigCircuit = baseGarble::garble(bigtest, 128, util::fast);
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
    vector<string> smalltest = {"1 3", "2 1 1", "1 1", "2 1 0 1 2 XOR"};
    int k = 128;
    BOOST_AUTO_TEST_CASE( test_encode_small00 )
    {
        for (int i = 0; i < 10; ++i) {
            auto smallCircuit = baseGarble::garble(smalltest, 128, util::fast);
            auto X00 = baseGarble::encode(get<1>(smallCircuit), {0, 0});
            auto Y00 = baseGarble::eval(smallCircuit, X00, smalltest);
            auto y00 = baseGarble::decodeBits(get<2>(smallCircuit), Y00);
            BOOST_TEST(y00[0] == 0);
        }
    }

    BOOST_AUTO_TEST_CASE( test_encode_small01 )
    {
        auto smallCircuit = baseGarble::garble(smalltest, 128, util::fast);
        auto X01 = baseGarble::encode(get<1>(smallCircuit), {0, 1});
        auto Y01 = baseGarble::eval(smallCircuit, X01, smalltest);
        auto y01 = baseGarble::decodeBits(get<2>(smallCircuit), Y01);
        BOOST_TEST(y01[0] == 1);
    }

    BOOST_AUTO_TEST_CASE( test_encode_small10 )
    {
        auto smallCircuit = baseGarble::garble(smalltest, 128, util::fast);
        auto X10 = baseGarble::encode(get<1>(smallCircuit), {1, 0});
        auto Y10 = baseGarble::eval(smallCircuit, X10, smalltest);
        auto y10 = baseGarble::decodeBits(get<2>(smallCircuit), Y10);
        BOOST_TEST(y10[0] == 1);
    }

    BOOST_AUTO_TEST_CASE( test_encode_small11 )
    {
        auto smallCircuit = baseGarble::garble(smalltest, 128, util::fast);
        auto X11 = baseGarble::encode(get<1>(smallCircuit), {1, 1});
        auto Y11 = baseGarble::eval(smallCircuit, X11, smalltest);
        auto y11 = baseGarble::decodeBits(get<2>(smallCircuit), Y11);
        BOOST_TEST(y11[0] == 0);
    }
BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE( Testing_BaseLineEvalAND )
    vector<string> smalltest = {"1 3", "2 1 1", "1 1", "2 1 0 1 2 AND"};
    int k = 128;
    auto smallCircuit = baseGarble::garble(smalltest, 128, util::fast);
    auto X00 = baseGarble::encode(get<1>(smallCircuit), {0, 0});
    auto Y00 = baseGarble::eval(smallCircuit, X00, smalltest);
    auto y00 = baseGarble::decodeBits(get<2>(smallCircuit), Y00);
    BOOST_AUTO_TEST_CASE( test_encode_small00 )
    {
        BOOST_TEST(y00[0] == 0);
    }

    auto X01 = baseGarble::encode(get<1>(smallCircuit), {0, 1});
    auto Y01 = baseGarble::eval(smallCircuit, X01, smalltest);
    auto y01 = baseGarble::decodeBits(get<2>(smallCircuit), Y01);
    BOOST_AUTO_TEST_CASE( test_encode_small01 )
    {
        BOOST_TEST(y01[0] == 0);
    }

    auto X10 = baseGarble::encode(get<1>(smallCircuit), {1, 0});
    auto Y10 = baseGarble::eval(smallCircuit, X10, smalltest);
    auto y10 = baseGarble::decodeBits(get<2>(smallCircuit), Y10);
    BOOST_AUTO_TEST_CASE( test_encode_small10 )
    {
        BOOST_TEST(y10[0] == 0);
    }

    auto X11 = baseGarble::encode(get<1>(smallCircuit), {1, 1});
    auto Y11 = baseGarble::eval(smallCircuit, X11, smalltest);
    auto y11 = baseGarble::decodeBits(get<2>(smallCircuit), Y11);
    BOOST_AUTO_TEST_CASE( test_encode_small11 )
    {
        BOOST_TEST(y11[0] == 1);
    }
BOOST_AUTO_TEST_SUITE_END()


BOOST_AUTO_TEST_SUITE( Testing_BaseLineEvalINV )
    vector<string> smalltest = {"1 2", "1 1", "1 1", "1 1 0 1 INV"};
    int k = 128;
    BOOST_AUTO_TEST_CASE( test_encode_small0 )
    {
        for (int i = 0; i < 10; ++i) {
            auto smallCircuit = baseGarble::garble(smalltest, 128, util::fast);
            auto X0 = baseGarble::encode(get<1>(smallCircuit), {0});
            auto Y0 = baseGarble::eval(smallCircuit, X0, smalltest);
            auto y0 = baseGarble::decodeBits(get<2>(smallCircuit), Y0);
            BOOST_TEST(y0[0] == 1);
        }
    }
    BOOST_AUTO_TEST_CASE( test_encode_small1 )
    {
        for (int i = 0; i < 10; ++i) {
            auto smallCircuit = baseGarble::garble(smalltest, 128, util::fast);
            auto X1 = baseGarble::encode(get<1>(smallCircuit), {1});
            auto Y1 = baseGarble::eval(smallCircuit, X1, smalltest);
            auto y1 = baseGarble::decodeBits(get<2>(smallCircuit), Y1);
            BOOST_TEST(y1[0] == 0);
        }
    }
BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE( Testing_BaseLineEvalTwoGates )

    vector<string> smalltest = {"2 4", "2 1 1", "1 1", "2 1 0 1 2 XOR", "2 1 0 1 3 XOR"};
    auto circuit = baseGarble::garble(smalltest, 128, util::fast);
    vector<int> inputLabels = {0, 0};
    auto X = baseGarble::encode(get<1>(circuit), inputLabels);
    auto Y = baseGarble::eval(circuit, X, smalltest);
    auto y = baseGarble::decodeBits(get<2>(circuit), Y);

    BOOST_AUTO_TEST_CASE( test_encode_two_gates )
    {
        BOOST_TEST(y.size() == 1);
        BOOST_TEST(y[0] == 0);
    }

    BOOST_AUTO_TEST_CASE( test_encode_two_gates2 )
    {
        vector<string> smalltest = {"2 4", "2 1 1", "1 1", "2 1 0 1 2 XOR", "2 1 0 1 3 AND"};
        auto circuit = baseGarble::garble(smalltest, 128, util::fast);
        vector<int> inputLabels = {0, 1};
        auto X = baseGarble::encode(get<1>(circuit), inputLabels);
        auto Y = baseGarble::eval(circuit, X, smalltest);
        auto y = baseGarble::decodeBits(get<2>(circuit), Y);
        BOOST_TEST(y.size() == 1);
        BOOST_TEST(y[0] == 0);
    }

    BOOST_AUTO_TEST_CASE( test_encode_two_gates3 )
    {
        vector<string> smalltest = {"2 6", "4 1 1 1 1", "1 1", "2 1 0 1 4 XOR", "2 1 2 3 5 AND"};
        auto circuit = baseGarble::garble(smalltest, 128, util::fast);
        vector<int> inputLabels = {0, 1, 0, 1};
        auto X = baseGarble::encode(get<1>(circuit), inputLabels);
        auto Y = baseGarble::eval(circuit, X, smalltest);
        auto y = baseGarble::decodeBits(get<2>(circuit), Y);
        BOOST_TEST(y.size() == 1);
        BOOST_TEST(y[0] == 0);
    }
BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE( Testing_BaseLineEvalBlood )

    vector<string> bloodtest = circuitParser::parseCircuit("../tests/circuits/BloodComp.txt");
    auto bloodCircuit = baseGarble::garble(bloodtest, 128, util::fast);
    vector<int> inputLabelsBlood = {0, 0, 0,
                                    1, 1, 1,
                                    1};
    auto XBlood = baseGarble::encode(get<1>(bloodCircuit), inputLabelsBlood);
    auto YBlood = baseGarble::eval(bloodCircuit, XBlood, bloodtest);
    auto yBlood = baseGarble::decodeBits(get<2>(bloodCircuit), YBlood);

    BOOST_AUTO_TEST_CASE( test_encode_blood )
    {
        BOOST_TEST(yBlood.size() == 1);
        BOOST_TEST(yBlood[0] == 0);
    }
BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE( Testing_BaseLineEvalBig )

    vector<string> bigtest = circuitParser::parseCircuit("../tests/circuits/adder64.txt");
    auto bigCircuit = baseGarble::garble(bigtest, 128, util::fast);
    vector<int> inputLabelsBig = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, //16
                                       1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                                       1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                                       1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                                       1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                                       1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                                       1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                                       1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
    auto XBig = baseGarble::encode(get<1>(bigCircuit), inputLabelsBig);
    auto YBig = baseGarble::eval(bigCircuit, XBig, bigtest);
    auto yBig = baseGarble::decodeBits(get<2>(bigCircuit), YBig);

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
    auto XBig_zeroes = baseGarble::encode(get<1>(bigCircuit), inputLabelsBig_zeroes);
    auto YBig_zeroes = baseGarble::eval(bigCircuit, XBig_zeroes, bigtest);
    auto yBig_zeroes = baseGarble::decodeBits(get<2>(bigCircuit), YBig_zeroes);

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
BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE( Testing_BaseLineEvalBigSub )

    vector<string> bigtest = circuitParser::parseCircuit("../tests/circuits/sub64.txt");
    auto bigCircuit = baseGarble::garble(bigtest, 128, util::fast);
    vector<int> inputLabelsBig = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //16
                                  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0,
                                  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1}; //100...000 - 100...000
    auto XBig = baseGarble::encode(get<1>(bigCircuit), inputLabelsBig);
    auto YBig = baseGarble::eval(bigCircuit, XBig, bigtest);
    auto yBig = baseGarble::decodeBits(get<2>(bigCircuit), YBig);

    BOOST_AUTO_TEST_CASE( test_encode_big_ones )
    {
        BOOST_TEST(yBig.size() == 64); //64 output wires
        for (int i = 0; i < 63; i++) {
            BOOST_TEST(yBig[i] == 0);
        }
        BOOST_TEST(yBig[63] == 1);
    }

BOOST_AUTO_TEST_SUITE_END()
