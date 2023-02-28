//
// Created by a on 27/02/2023.
//
//
#include <boost/test/unit_test.hpp>
#include "../util/util.h"
#include "../schemes/atecaGarble.h"
#include "../bloodcompatibility.h"

using namespace boost::unit_test;


BOOST_AUTO_TEST_SUITE( Testing_garbling_values )
    auto C = circuitParser::parseCircuit("../tests/circuits/BloodComp.txt");
    int l = 64;
    auto feds = atecaGarble::Gb(l, C);
    auto F = get<0>(feds);
    auto encodingInfo = get<1>(feds);
    auto decoding = get<2>(feds);

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
        BOOST_TEST(F[3].size()==8);//gate
        BOOST_TEST(F[9].size()==8);//last gate
        BOOST_TEST(F[1][0]!=0);//test value not null
        BOOST_TEST(F[9][1]!=0);//test value not null
        BOOST_TEST(F[9][7]==0);//unlucky test last uint prolly has not bits
    }
    BOOST_AUTO_TEST_CASE( test_DecodingInfo )
    {
        //test Decoding info
        BOOST_TEST(decoding.size()==1);//only one output value
        BOOST_TEST(decoding[0][0]!=0);//val not zero
    }
BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE( Testing_input_encoding_choice )
    auto finput = vector<int>{0,0,0,0,0,0,1};
    auto C = circuitParser::parseCircuit("../tests/circuits/BloodComp.txt");
    int l = 64;
    auto feds = atecaGarble::Gb(l, C);
    auto F = get<0>(feds);
    auto encodingInfo = get<1>(feds);
    auto decoding = get<2>(feds);
    BOOST_AUTO_TEST_CASE( encoding_choice )
    {
        //compare chosen input with encoding
        auto encodedInput = atecaGarble::En(get<1>(feds), finput);
        BOOST_TEST(get<0>(encodingInfo[5])[0]==encodedInput[5][0]);
        BOOST_TEST(get<0>(encodingInfo[6])[0]!=encodedInput[6][0]);
    }

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE( Testing_garbled_Output )
    auto finput = vector<int>{0,0,0,0,0,0,1};
    auto C = circuitParser::parseCircuit("../tests/circuits/BloodComp.txt");
    int l = 64;
    auto feds = atecaGarble::Gb(l, C);
    auto encodedInput = atecaGarble::En(get<1>(feds), finput);
    auto Y = atecaGarble::Ev(get<0>(feds), encodedInput, C, get<3>(feds));
    BOOST_AUTO_TEST_CASE( garbled_output )
    {
        BOOST_TEST(Y.size()==1);
        BOOST_TEST(Y[0][0]!=0);
        BOOST_TEST(Y[0].size() ==1);
    }
BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE( Testing_decoding_garbled_Y )
    auto finput = vector<int>{0,0,0,0,0,0,1};
    auto C = circuitParser::parseCircuit("../tests/circuits/BloodComp.txt");
    int l = 64;
    auto feds = atecaGarble::Gb(l, C);
    auto encodedInput = atecaGarble::En(get<1>(feds), finput);
    auto Y = atecaGarble::Ev(get<0>(feds), encodedInput, C, get<3>(feds));
    auto y = atecaGarble::De(Y, get<2>(feds));
    BOOST_AUTO_TEST_CASE( decoding_Y )
    {
        BOOST_TEST(y.size()==1);
        //BOOST_TEST(y[0]==1);
    }
BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE( Testing_BloodComp_Alternate )
    int lInput =6; int rInput = 1;
    auto finput = vector<int>{1,1,0,0,0,1,1};
    auto C = circuitParser::parseCircuit("../tests/circuits/BloodComp.txt");
    int l = 64;
    auto feds = atecaGarble::Gb(l, C);
    auto encodedInput = atecaGarble::En(get<1>(feds), finput);
    auto Y = atecaGarble::Ev(get<0>(feds), encodedInput, C, get<3>(feds));
    auto y = atecaGarble::De(Y, get<2>(feds));
    ::uint64_t bloodCompAns = bloodcompatibility::bloodCompLookup(lInput,rInput);
    BOOST_AUTO_TEST_CASE( decoding_Y )
    {
        BOOST_TEST(y.size()==1);
        //BOOST_TEST(y[0]==bloodCompAns);
    }
BOOST_AUTO_TEST_SUITE_END()


BOOST_AUTO_TEST_SUITE( adder64bit )

    BOOST_AUTO_TEST_CASE( adder64Adding1And1 )
    {

        auto finput = vector<int>{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,};
        auto bloodCircuit = circuitParser::parseCircuit("../tests/circuits/adder64.txt");
        auto feds = atecaGarble::Gb(64, bloodCircuit);
        auto encodedInput = atecaGarble::En(get<1>(feds), finput);
        auto Y = atecaGarble::Ev(get<0>(feds), encodedInput, bloodCircuit, get<3>(feds));
        auto y = atecaGarble::De(Y, get<2>(feds));

        BOOST_TEST(y.size()==64);
        //BOOST_TEST(y[0]==2);
    }

BOOST_AUTO_TEST_SUITE_END()