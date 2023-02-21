//
// Created by svend on 020, 20-02-2023.
//
#include <boost/test/unit_test.hpp>
#include "../schemes/baseGarble.h"
#include "../util/util.h"
#include "../util/otUtil/otUtil.h"

using namespace boost::unit_test;

BOOST_AUTO_TEST_SUITE( Testing_BaseLineExtractGate )
    string gateXOR = "2 1 0 6 7 XOR";
    string gateAND = "2 1 7 3 10 AND";
    tuple<vector<int>, vector<int>, string> gateInfoXOR = baseGarble::extractGate(gateXOR);
    tuple<vector<int>, vector<int>, string> gateInfoAND = baseGarble::extractGate(gateAND);
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
    //vector<string> smalltest = {"1 3", "2 1 1", "1 1", "2 1 0 1 2 XOR"};
    string gate = "2 1 0 1 2 XOR";
    tuple<vector<int>, vector<int>, string> gateInfo = baseGarble::extractGate(gate);
    vector<int> inputWires = get<0>(gateInfo);
    vector<int> outputWires = get<1>(gateInfo);
    string gateType = get<2>(gateInfo);
    vector<tuple<vector<uint64_t>, vector<uint64_t>>> inputWiresLabels = vector<tuple<vector<uint64_t>, vector<uint64_t>>>(inputWires.size());
    vector<tuple<vector<uint64_t>, vector<uint64_t>>> outputWiresLabels = vector<tuple<vector<uint64_t>, vector<uint64_t>>>(outputWires.size());
    int k = 128;
    auto globalDelta = vector<uint64_t>(k);
    auto deltaAndLabels = util::generateRandomLabels(k, globalDelta, inputWiresLabels);
    auto delta = get<0>(deltaAndLabels);
    auto labels = get<1>(deltaAndLabels);
    int permuteBitA = (int)((get<0>(labels[0])[0]) & 1);
    int permuteBitB = (int)((get<0>(labels[1])[0]) & 1);
    vector<uint64_t> A0;
    vector<uint64_t> A1;
    vector<uint64_t> B0;
    vector<uint64_t> B1;

    BOOST_AUTO_TEST_CASE( test_inputWires )
    {
        //cout << "A0 " << otUtil::printBitsetofVectorofUints(get<0>(labels[0])) << endl;
        //cout << "B0 " << otUtil::printBitsetofVectorofUints(get<0>(labels[1])) << endl;
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
        auto res = baseGarble::andGate(globalDelta, permuteBitA, permuteBitB, A0, A1, B0, B1, ciphertext, gate0, gate1);

        //cout << "ciphertext: " << otUtil::printBitsetofVectorofUints(get<0>(res)) << endl;
        //cout << "gate0: " << otUtil::printBitsetofVectorofUints(get<1>(res)) << endl;
        //cout << "gate1: " << otUtil::printBitsetofVectorofUints(get<2>(res)) << endl;
        //cout << "delta: " << otUtil::printBitsetofVectorofUints(delta) << endl;
        //cout << "A0: " << otUtil::printBitsetofVectorofUints(A0) << endl;
        //cout << "A1: " << otUtil::printBitsetofVectorofUints(A1) << endl;
        //cout << "B0: " << otUtil::printBitsetofVectorofUints(B0) << endl;
        //cout << "B1: " << otUtil::printBitsetofVectorofUints(B1) << endl;
        //cout << "Permuted A: " << permuteBitA << endl;
        //cout << "Permuted B: " << permuteBitB << endl;

        if(permuteBitA == 0 | permuteBitB == 0) {
            BOOST_TEST(ciphertext == util::bitVecXOR(baseGarble::qouteUnqouteHashFunction(A0),
                                                     baseGarble::qouteUnqouteHashFunction(B0)));
            if (permuteBitA == 0 & permuteBitB == 0){
                BOOST_TEST(
                        util::bitVecXOR(ciphertext, delta) ==
                        util::bitVecXOR(baseGarble::qouteUnqouteHashFunction(A1),
                                        util::bitVecXOR(
                                                baseGarble::qouteUnqouteHashFunction(
                                                        B1),
                                                util::bitVecXOR(get<2>(res),
                                                                util::bitVecXOR(
                                                                        get<1>(res),
                                                                        A1)))));
            }
            else {
                BOOST_TEST(ciphertext ==
                           util::bitVecXOR(baseGarble::qouteUnqouteHashFunction(A1),
                                           util::bitVecXOR(baseGarble::qouteUnqouteHashFunction(B1),
                                                           util::bitVecXOR(get<2>(res),
                                                                           util::bitVecXOR(
                                                                                   get<1>(res),
                                                                                   A1)))));
            }
        } else {
            BOOST_TEST(util::bitVecXOR(ciphertext, delta) ==
            util::bitVecXOR(baseGarble::qouteUnqouteHashFunction(A0),
                            baseGarble::qouteUnqouteHashFunction(B0)));
            BOOST_TEST(ciphertext == util::bitVecXOR(baseGarble::qouteUnqouteHashFunction(A1),
                                                     util::bitVecXOR(baseGarble::qouteUnqouteHashFunction(B1),
                                                                     util::bitVecXOR(get<2>(res),
                                                                                     util::bitVecXOR(get<1>(res),
                                                                                                     A1)))));
        }


    }
BOOST_AUTO_TEST_SUITE_END()