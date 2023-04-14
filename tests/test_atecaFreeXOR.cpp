//
// Created by a on 13/03/2023.
//
#include <boost/test/unit_test.hpp>
#include "../util/util.h"
#include "../bloodcompatibility.h"
#include "../schemes/atecaFreeXOR.h"
#include "../schemes/atecaGarble.h"
#include "../util/circuitParser.h"

using namespace boost::unit_test;


BOOST_AUTO_TEST_SUITE( ATECA_freexor_GarbleVals )
    auto C = circuitParser::parseCircuit("../tests/circuits/BloodComp.txt");
    int l = 64;
    auto [F,encodingInfo,decoding,secL,invVar,hashtype] = atecaFreeXOR::garble(l, C, 0);

    BOOST_AUTO_TEST_CASE( test_input_encoding )
    {

        //test encoding
        BOOST_TEST(encodingInfo.size()==7);
        BOOST_TEST(get<0>(encodingInfo[0])[0]!=0);
        BOOST_TEST(get<1>(encodingInfo[4])[0]!=0);

    }

    BOOST_AUTO_TEST_CASE( test_Garbled_wires )
    {
        //test F
        BOOST_TEST(F[3].size()==16);//gate
        BOOST_TEST(F[9].size()==16);//last gate
        BOOST_TEST(F[9][1]!=0);//test value not null
        BOOST_TEST(F[9][15]==0);//unlucky test last uint prolly has not bits
    }
    BOOST_AUTO_TEST_CASE( test_DecodingInfo )
    {
        //test Decoding info
        BOOST_TEST(decoding.size()==1);//only one output value
        BOOST_TEST(decoding[0][0]!=0);//val not zero
    }
BOOST_AUTO_TEST_SUITE_END()



BOOST_AUTO_TEST_SUITE( ATECA_freexor_input_encoding_choice )
    auto finput = vector<int>{0,0,0,0,0,0,1};
    auto C = circuitParser::parseCircuit("../tests/circuits/BloodComp.txt");
    int l = 64;
    auto [F,encodingInfo,decoding,secL,invVar,hashtype] = atecaFreeXOR::garble(l, C, 0);
    BOOST_AUTO_TEST_CASE( encoding_choice )
    {
        //compare chosen input with encoding
        auto encodedInput = atecaFreeXOR::encode(encodingInfo, finput);
        BOOST_TEST(get<0>(encodingInfo[5])[0]==encodedInput[5][0]);
        BOOST_TEST(get<0>(encodingInfo[6])[0]!=encodedInput[6][0]);
    }

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE( ATECA_freexor_garbled_Output )
    auto finput = vector<int>{0,0,0,0,0,0,1};
    auto C = circuitParser::parseCircuit("../tests/circuits/BloodComp.txt");
    int l = 64;
    auto [F,encodingInfo,decoding,secL,invVar,hashtype] = atecaFreeXOR::garble(l, C, 0);
    auto encodedInput = atecaFreeXOR::encode(encodingInfo, finput);
    auto Y = atecaFreeXOR::eval(F, encodedInput, C, secL, invVar);
    BOOST_AUTO_TEST_CASE( garbled_output )
    {
        BOOST_TEST(Y.size()==1);
        BOOST_TEST(Y[0][0]!=0);
        BOOST_TEST(Y[0].size() ==1);
    }
BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE( ATECA_freexor_decoding_garbled_Y )
    auto finput = vector<int>{0,0,0,0,0,0,1};
    auto C = circuitParser::parseCircuit("../tests/circuits/BloodComp.txt");
    int l = 64;
    auto [F,encodingInfo,decoding,secL,invVar,hashtype] = atecaFreeXOR::garble(l, C, 0);
    auto encodedInput = atecaFreeXOR::encode(encodingInfo, finput);
    auto Y = atecaFreeXOR::eval(F, encodedInput, C, secL, invVar);
    auto y = atecaFreeXOR::decode(Y, decoding);
    BOOST_AUTO_TEST_CASE( decoding_Y )
    {
        BOOST_TEST(y.size()==1);
        //BOOST_TEST(y[0]==1);
    }
BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE( ATECA_freexor_xorGate_test )

    BOOST_AUTO_TEST_CASE( xor_0w0 )
    {
        for (int i = 0; i < 10; ++i) {
            auto finput1 = vector<int>{0,0};
            auto C = circuitParser::parseCircuit("../tests/circuits/xorTest.txt");
            int l = 64;
            auto [F,encodingInfo,decoding,secL,invVar,hashtype] = atecaFreeXOR::garble(l, C, 0);
            auto encodedInput = atecaFreeXOR::encode(encodingInfo, finput1);
            auto Y = atecaFreeXOR::eval(F, encodedInput, C, secL, invVar);
            auto y = atecaFreeXOR::decode(Y, decoding);
            //testCase
            BOOST_TEST(y.size()==1);
            BOOST_TEST(y[0]==0);
        }
    }
    BOOST_AUTO_TEST_CASE( xor_0w1 )
    {
        for (int i = 0; i < 10; ++i) {
            auto finput = vector<int>{0,1};
            auto C = circuitParser::parseCircuit("../tests/circuits/xorTest.txt");
            int l = 64;
            auto [F,encodingInfo,decoding,secL,invVar,hashtype] = atecaFreeXOR::garble(l, C, 0);
            auto encodedInput = atecaFreeXOR::encode(encodingInfo, finput);
            auto Y = atecaFreeXOR::eval(F, encodedInput, C, secL, invVar);
            auto y = atecaFreeXOR::decode(Y, decoding);
            //testCase
            BOOST_TEST(y.size()==1);
            BOOST_TEST(y[0]==1);
        }
    }
BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE( ATECA_freexor_andGate_test )

    BOOST_AUTO_TEST_CASE( and_0w1 )
    {
        for (int i = 0; i < 10; ++i) {
            auto finput1 = vector<int>{0,1};
            auto C = circuitParser::parseCircuit("../tests/circuits/andTest.txt");
            int l = 64;
            auto [F,encodingInfo,decoding,secL,invVar,hashtype] = atecaFreeXOR::garble(l, C, 0);
            auto encodedInput = atecaFreeXOR::encode(encodingInfo, finput1);
            auto Y = atecaFreeXOR::eval(F, encodedInput, C, secL, invVar);
            auto y = atecaFreeXOR::decode(Y, decoding);
            //testCase
            BOOST_TEST(y.size()==1);
            BOOST_TEST(y[0]==0);
        }
    }
    BOOST_AUTO_TEST_CASE( and_1w1 )
    {
        for (int i = 0; i < 10; ++i) {
            auto finput = vector<int>{1,1};
            auto C = circuitParser::parseCircuit("../tests/circuits/andTest.txt");
            int l = 64;
            auto [F,encodingInfo,decoding,secL,invVar,hashtype] = atecaFreeXOR::garble(l, C, 0);
            auto encodedInput = atecaFreeXOR::encode(encodingInfo, finput);
            auto Y = atecaFreeXOR::eval(F, encodedInput, C, secL, invVar);
            auto y = atecaFreeXOR::decode(Y, decoding);
            //testCase
            BOOST_TEST(y.size()==1);
            BOOST_TEST(y[0]==1);
        }
    }
BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE( ATECA_freexor_invGate_test )

    BOOST_AUTO_TEST_CASE( inv_0 )
    {
        for (int i = 0; i < 10; ++i) {
            auto finput1 = vector<int>{0};
            auto C = circuitParser::parseCircuit("../tests/circuits/invTest.txt");
            int l = 64;
            auto [F,encodingInfo,decoding,secL,invVar,hashtype] = atecaFreeXOR::garble(l, C, 0);
            auto encodedInput = atecaFreeXOR::encode(encodingInfo, finput1);
            auto Y = atecaFreeXOR::eval(F, encodedInput, C, secL, invVar);
            auto y = atecaFreeXOR::decode(Y, decoding);
            //testCase
            BOOST_TEST(y.size()==1);
            BOOST_TEST(y[0]==1);
        }
    }
    BOOST_AUTO_TEST_CASE( inv_1 )
    {
        for (int i = 0; i < 10; ++i) {
            auto finput = vector<int>{1};
            auto C = circuitParser::parseCircuit("../tests/circuits/invTest.txt");
            int l = 64;
            auto [F,encodingInfo,decoding,secL,invVar,hashtype] = atecaFreeXOR::garble(l, C, 0);
            auto encodedInput = atecaFreeXOR::encode(encodingInfo, finput);
            auto Y = atecaFreeXOR::eval(F, encodedInput, C, secL, invVar);
            auto y = atecaFreeXOR::decode(Y, decoding);
            //testCase
            BOOST_TEST(y.size()==1);
            BOOST_TEST(y[0]==0);
        }
    }
BOOST_AUTO_TEST_SUITE_END()


BOOST_AUTO_TEST_SUITE( ATECA_freexor_xorIntoAnd_test )

    BOOST_AUTO_TEST_CASE( XORAND_1w0 )
    {
        for (int i = 0; i < 10; ++i) {
            auto finput1 = vector<int>{1,0};
            auto C = circuitParser::parseCircuit("../tests/circuits/xorIntoAnd.txt");
            int l = 64;
            auto [F,encodingInfo,decoding,secL,invVar,hashtype] = atecaFreeXOR::garble(l, C, 0);
            auto encodedInput = atecaFreeXOR::encode(encodingInfo, finput1);
            auto Y = atecaFreeXOR::eval(F, encodedInput, C, secL, invVar);
            auto y = atecaFreeXOR::decode(Y, decoding);
            //testCase
            BOOST_TEST(y.size()==1);
            BOOST_TEST(y[0]==1);
        }
    }
    BOOST_AUTO_TEST_CASE( XORAND_1w1 )
    {
        for (int i = 0; i < 10; ++i) {
            auto finput = vector<int>{1,1};
            auto C = circuitParser::parseCircuit("../tests/circuits/xorIntoAnd.txt");
            int l = 64;
            auto [F,encodingInfo,decoding,secL,invVar,hashtype] = atecaFreeXOR::garble(l, C, 0);
            auto encodedInput = atecaFreeXOR::encode(encodingInfo, finput);
            auto Y = atecaFreeXOR::eval(F, encodedInput, C, secL, invVar);
            auto y = atecaFreeXOR::decode(Y, decoding);
            //testCase
            BOOST_TEST(y.size()==1);
            BOOST_TEST(y[0]==0);
        }
    }
BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE( ATECA_freexor_xnorIntoAnd_test )

    BOOST_AUTO_TEST_CASE( XNORAND_1w0 )
    {
        for (int i = 0; i < 10; ++i) {
            auto finput1 = vector<int>{1,0};
            auto C = circuitParser::parseCircuit("../tests/circuits/xnorIntoAnd.txt");
            int l = 64;
            auto [F,encodingInfo,decoding,secL,invVar,hashtype] = atecaFreeXOR::garble(l, C, 0);
            auto encodedInput = atecaFreeXOR::encode(encodingInfo, finput1);
            auto Y = atecaFreeXOR::eval(F, encodedInput, C, secL, invVar);
            auto y = atecaFreeXOR::decode(Y, decoding);
            //testCase
            BOOST_TEST(y.size()==1);
            BOOST_TEST(y[0]==0);
        }
    }
    BOOST_AUTO_TEST_CASE( XNORAND_1w1 )
    {
        for (int i = 0; i < 10; ++i) {
            auto finput = vector<int>{1,1};
            auto C = circuitParser::parseCircuit("../tests/circuits/xnorIntoAnd.txt");
            int l = 64;
            auto [F,encodingInfo,decoding,secL,invVar,hashtype] = atecaFreeXOR::garble(l, C, 0);
            auto encodedInput = atecaFreeXOR::encode(encodingInfo, finput);
            auto Y = atecaFreeXOR::eval(F, encodedInput, C, secL, invVar);
            auto y = atecaFreeXOR::decode(Y, decoding);
            //testCase
            BOOST_TEST(y.size()==1);
            BOOST_TEST(y[0]==1);
        }
    }
BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE( ATECA_freexor_andIntoXOR_test )

    BOOST_AUTO_TEST_CASE( ANDXOR_1w1 )
    {
        for (int i = 0; i < 10; ++i) {
            auto finput1 = vector<int>{1,1};
            auto C = circuitParser::parseCircuit("../tests/circuits/andIntoXor.txt");
            int l = 64;
            auto [F,encodingInfo,decoding,secL,invVar,hashtype] = atecaFreeXOR::garble(l, C, 0);
            auto encodedInput = atecaFreeXOR::encode(encodingInfo, finput1);
            auto Y = atecaFreeXOR::eval(F, encodedInput, C, secL, invVar);
            auto y = atecaFreeXOR::decode(Y, decoding);
            //testCase
            BOOST_TEST(y.size()==1);
            BOOST_TEST(y[0]==0);
        }
    }
    BOOST_AUTO_TEST_CASE( ANDXOR_1w0 )
    {
        for (int i = 0; i < 10; ++i) {
            auto finput = vector<int>{1,0};
            auto C = circuitParser::parseCircuit("../tests/circuits/andIntoXor.txt");
            int l = 64;
            auto [F,encodingInfo,decoding,secL,invVar,hashtype] = atecaFreeXOR::garble(l, C, 0);
            auto encodedInput = atecaFreeXOR::encode(encodingInfo, finput);
            auto Y = atecaFreeXOR::eval(F, encodedInput, C, secL, invVar);
            auto y = atecaFreeXOR::decode(Y, decoding);
            //testCase
            BOOST_TEST(y.size()==1);
            BOOST_TEST(y[0]==1);
        }
    }
BOOST_AUTO_TEST_SUITE_END()


BOOST_AUTO_TEST_SUITE( ATECA_freexor_xorGate_test2 )
    auto finput2 = vector<int>{0,1};
    auto C = circuitParser::parseCircuit("../tests/circuits/xorTest.txt");
    int l = 64;
    auto [F,encodingInfo,decoding,secL,invVar,hashtype] = atecaFreeXOR::garble(l, C, 0);
    auto encodedInput = atecaFreeXOR::encode(encodingInfo, finput2);
    auto Y = atecaFreeXOR::eval(F, encodedInput, C, secL, invVar);
    auto y = atecaFreeXOR::decode(Y, decoding);

    BOOST_AUTO_TEST_CASE( xor_Output )
    {
        BOOST_TEST(y.size()==1);
        BOOST_TEST(y[0]==1);
    }
BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE( ATECA_freexor_andGate_test )
    auto finput1 = vector<int>{1,1};
    auto finput2 = vector<int>{0,1};
    auto C = circuitParser::parseCircuit("../tests/circuits/andTest.txt");
    int l = 64;
    auto [F,encodingInfo,decoding,secL,invVar,hashtype] = atecaFreeXOR::garble(l, C, 0);
    auto encodedInput = atecaFreeXOR::encode(encodingInfo, finput1);
    auto Y = atecaFreeXOR::eval(F, encodedInput, C, secL, invVar);
    auto y = atecaFreeXOR::decode(Y, decoding);

    BOOST_AUTO_TEST_CASE( encoding_choice )
    {
        BOOST_TEST(get<1>(encodingInfo[0])[0]==encodedInput[0][0]);
        encodedInput = atecaFreeXOR::encode(encodingInfo, finput2);
        BOOST_TEST(get<0>(encodingInfo[0])[0]==encodedInput[0][0]);
    }


    BOOST_AUTO_TEST_CASE( and_output )
    {
        BOOST_TEST(y.size()==1);
        BOOST_TEST(y[0]==1);
    }
BOOST_AUTO_TEST_SUITE_END()



BOOST_AUTO_TEST_SUITE( ATECA_freexor_bloodComp )
    int lInput =6; int rInput = 1;
    auto finput = vector<int>{1,1,0,0,0,1,1};
    auto C = circuitParser::parseCircuit("../tests/circuits/BloodComp.txt");
    int l = 64;
    auto [F,encodingInfo,decoding,secL,invVar,hashtype] = atecaFreeXOR::garble(l, C, 0);
    auto encodedInput = atecaFreeXOR::encode(encodingInfo, finput);
    auto Y = atecaFreeXOR::eval(F, encodedInput, C, secL, invVar);
    auto y = atecaFreeXOR::decode(Y, decoding);
    ::uint64_t bloodCompAns = bloodcompatibility::bloodCompLookup(lInput,rInput);
    BOOST_AUTO_TEST_CASE( decoding_Y )
    {
        BOOST_TEST(y.size()==1);
        BOOST_TEST(y[0]==bloodCompAns);
    }
BOOST_AUTO_TEST_SUITE_END()


BOOST_AUTO_TEST_SUITE( ATECA_freexor_adder64bit )

    BOOST_AUTO_TEST_CASE( adder64Adding1And1 )
    {
        //least significant bit first :^)
        auto finput = vector<int>{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                                  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                                  1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                                  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
        auto C = circuitParser::parseCircuit("../tests/circuits/adder64.txt");
        auto [F,encodingInfo,decoding,secL,invVar,hashtype] = atecaFreeXOR::garble(64, C, 0);
        auto encodedInput = atecaFreeXOR::encode(encodingInfo, finput);
        auto Y = atecaFreeXOR::eval(F, encodedInput, C, secL, invVar);
        auto y = atecaFreeXOR::decode(Y, decoding);

        //util::printUintVec(y);

        BOOST_TEST(y.size()==1);
        BOOST_TEST(y[0]==2);
    }

    BOOST_AUTO_TEST_CASE( freexor_adder64AddingMaxWMax )
    {
        //least significant bit first :^)
        auto maxPlusMax = UINT64_MAX<<1;
        auto finput = vector<int>{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                                  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                                  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                                  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};
        auto C = circuitParser::parseCircuit("../tests/circuits/adder64.txt");
        auto [F,encodingInfo,decoding,secL,invVar,hashtype] = atecaFreeXOR::garble(64, C, 0);
        auto encodedInput = atecaFreeXOR::encode(encodingInfo, finput);
        auto Y = atecaFreeXOR::eval(F, encodedInput, C, secL, invVar);
        auto y = atecaFreeXOR::decode(Y, decoding);


        BOOST_TEST(y.size()==1);
        BOOST_TEST(y[0]==maxPlusMax);
    }

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE( ATECA_freexor_sub64 )

    BOOST_AUTO_TEST_CASE( adder64_2minus1 )
    {
        //least significant bit first :^)
        auto finput = vector<int>{0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                                  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                                  1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                                  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
        auto C = circuitParser::parseCircuit("../tests/circuits/sub64.txt");
        auto [F,encodingInfo,decoding,secL,invVar,hashtype] = atecaFreeXOR::garble(64, C, 0);
        auto encodedInput = atecaFreeXOR::encode(encodingInfo, finput);
        auto Y = atecaFreeXOR::eval(F, encodedInput, C, secL, invVar);
        auto y = atecaFreeXOR::decode(Y, decoding);

        BOOST_TEST(y.size()==1);
        BOOST_TEST(y[0]==1);
    }

    BOOST_AUTO_TEST_CASE( freexor_adder64_8448minus2051 )
    {
        //least significant bit first :^)
        auto finput = vector<int>{0,0,0,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                                  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                                  1,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                                  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
        auto C = circuitParser::parseCircuit("../tests/circuits/sub64.txt");
        auto [F,encodingInfo,decoding,secL,invVar,hashtype] = atecaFreeXOR::garble(64, C, 0);
        auto encodedInput = atecaFreeXOR::encode(encodingInfo, finput);
        auto Y = atecaFreeXOR::eval(F, encodedInput, C, secL, invVar);
        auto y = atecaFreeXOR::decode(Y, decoding);

        BOOST_TEST(y.size()==1);
        BOOST_TEST(y[0]==6397);
    }

BOOST_AUTO_TEST_SUITE_END()