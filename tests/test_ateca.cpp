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
    auto feds = atecaGarble::Gb(l, C,"RO");
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

BOOST_AUTO_TEST_SUITE( Test_projection )

    auto b = vector<::uint64_t>{15,0};
    auto a = vector<::uint64_t>{9,0};


    BOOST_AUTO_TEST_CASE( simpleProjection )
    {
        auto res = atecaGarble::projection(a,b);
        BOOST_TEST(res.size()==1);
        BOOST_TEST(res[0]==9);
    }

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE( Test_large_projection )
    auto b = vector<::uint64_t>{152109056,0};
    auto a = vector<::uint64_t>{134283264,0};


    auto b1 = vector<::uint64_t>{4755837490663786591,0};
    auto a1 = vector<::uint64_t>{5908722711386916953,0};
    BOOST_AUTO_TEST_CASE( largerVec )
    {
        auto res = atecaGarble::projection(a,b);
        BOOST_TEST(res.size()==1);
        BOOST_TEST(res[0]==9);
        res = atecaGarble::projection(a1,b1);
        BOOST_TEST(res[0]==5107);

    }
BOOST_AUTO_TEST_SUITE_END()


BOOST_AUTO_TEST_SUITE( Testing_input_encoding_choice )
    auto finput = vector<int>{0,0,0,0,0,0,1};
    auto C = circuitParser::parseCircuit("../tests/circuits/BloodComp.txt");
    int l = 64;
    auto feds = atecaGarble::Gb(l, C,"RO");
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
    auto feds = atecaGarble::Gb(l, C,"RO");
    auto encodedInput = atecaGarble::En(get<1>(feds), finput);
    auto Y = atecaGarble::Ev(get<0>(feds), encodedInput, C, get<3>(feds),get<4>(feds));
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
    auto feds = atecaGarble::Gb(l, C,"RO");
    auto encodedInput = atecaGarble::En(get<1>(feds), finput);
    auto Y = atecaGarble::Ev(get<0>(feds), encodedInput, C, get<3>(feds), get<4>(feds));
    auto y = atecaGarble::De(Y, get<2>(feds));
    BOOST_AUTO_TEST_CASE( decoding_Y )
    {
        BOOST_TEST(y.size()==1);
        //BOOST_TEST(y[0]==1);
    }
BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE( xorTest )
    auto finput1 = vector<int>{0,0};
    auto finput2 = vector<int>{0,1};
    auto C = circuitParser::parseCircuit("../tests/circuits/xorTest.txt");
    int l = 64;
    auto feds = atecaGarble::Gb(l, C,"RO");
    auto encodingInfo = get<1>(feds);
    auto encodedInput = atecaGarble::En(get<1>(feds), finput1);
    auto Y = atecaGarble::Ev(get<0>(feds), encodedInput, C, get<3>(feds), get<4>(feds));
    auto y = atecaGarble::De(Y, get<2>(feds));
    BOOST_AUTO_TEST_CASE( encoding_choice )
    {
        BOOST_TEST(get<0>(encodingInfo[1])[0]==encodedInput[1][0]);
        encodedInput = atecaGarble::En(get<1>(feds), finput2);
        BOOST_TEST(get<1>(encodingInfo[1])[0]==encodedInput[1][0]);
    }

    BOOST_AUTO_TEST_CASE( xor_Output )
    {
        BOOST_TEST(y.size()==1);
        BOOST_TEST(y[0]==0);
    }
BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE( andTest )
    auto finput1 = vector<int>{1,1};
    auto finput2 = vector<int>{0,1};
    auto C = circuitParser::parseCircuit("../tests/circuits/andTest.txt");
    int l = 64;
    auto feds = atecaGarble::Gb(l, C,"RO");
    auto encodingInfo = get<1>(feds);
    auto encodedInput = atecaGarble::En(get<1>(feds), finput1);
    auto Y = atecaGarble::Ev(get<0>(feds), encodedInput, C, get<3>(feds), get<4>(feds));
    auto y = atecaGarble::De(Y, get<2>(feds));

    BOOST_AUTO_TEST_CASE( encoding_choice )
    {
        BOOST_TEST(get<1>(encodingInfo[0])[0]==encodedInput[0][0]);
        encodedInput = atecaGarble::En(get<1>(feds), finput2);
        BOOST_TEST(get<0>(encodingInfo[0])[0]==encodedInput[0][0]);
    }


    BOOST_AUTO_TEST_CASE( and_output )
    {
        BOOST_TEST(y.size()==1);
        BOOST_TEST(y[0]==1);
    }
BOOST_AUTO_TEST_SUITE_END()



BOOST_AUTO_TEST_SUITE( Testing_BloodComp_Alternate )
    int lInput =6; int rInput = 1;
    auto finput = vector<int>{1,1,0,0,0,1,1};
    auto C = circuitParser::parseCircuit("../tests/circuits/BloodComp.txt");
    int l = 64;
    auto feds = atecaGarble::Gb(l, C,"RO");
    auto encodedInput = atecaGarble::En(get<1>(feds), finput);
    auto Y = atecaGarble::Ev(get<0>(feds), encodedInput, C, get<3>(feds), get<4>(feds));
    auto y = atecaGarble::De(Y, get<2>(feds));
    ::uint64_t bloodCompAns = bloodcompatibility::bloodCompLookup(lInput,rInput);
    BOOST_AUTO_TEST_CASE( decoding_Y )
    {
        BOOST_TEST(y.size()==1);
        BOOST_TEST(y[0]==bloodCompAns);
    }
BOOST_AUTO_TEST_SUITE_END()


BOOST_AUTO_TEST_SUITE( adder64bit )

    BOOST_AUTO_TEST_CASE( adder64Adding1And1 )
    {
        //least significant bit first :^)
        auto finput = vector<int>{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                                  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                                  1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                                  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
        auto C = circuitParser::parseCircuit("../tests/circuits/adder64.txt");
        auto feds = atecaGarble::Gb(64, C,"RO");
        auto encodedInput = atecaGarble::En(get<1>(feds), finput);
        auto Y = atecaGarble::Ev(get<0>(feds), encodedInput, C, get<3>(feds), get<4>(feds));
        auto y = atecaGarble::De(Y, get<2>(feds));

        //util::printUintVec(y);

        BOOST_TEST(y.size()==1);
        BOOST_TEST(y[0]==2);
    }

    BOOST_AUTO_TEST_CASE( adder64AddingMaxWMax )
    {
        //least significant bit first :^)
        auto maxPlusMax = UINT64_MAX<<1;
        auto finput = vector<int>{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                                  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                                  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                                  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};
        auto C = circuitParser::parseCircuit("../tests/circuits/adder64.txt");
        auto feds = atecaGarble::Gb(64, C,"RO");
        auto encodedInput = atecaGarble::En(get<1>(feds), finput);
        auto Y = atecaGarble::Ev(get<0>(feds), encodedInput, C, get<3>(feds), get<4>(feds));
        auto y = atecaGarble::De(Y, get<2>(feds));

        //util::printUintVec(y);

        BOOST_TEST(y.size()==1);
        BOOST_TEST(y[0]==maxPlusMax);
    }

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE( sub64 )

    BOOST_AUTO_TEST_CASE( adder64_2minus1 )
    {
    //least significant bit first :^)
    auto finput = vector<int>{0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                              0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                              1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                              0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    auto C = circuitParser::parseCircuit("../tests/circuits/sub64.txt");
    auto feds = atecaGarble::Gb(64, C,"RO");
    auto encodedInput = atecaGarble::En(get<1>(feds), finput);
    auto Y = atecaGarble::Ev(get<0>(feds), encodedInput, C, get<3>(feds), get<4>(feds));
    auto y = atecaGarble::De(Y, get<2>(feds));

    util::printUintVec(y);

    BOOST_TEST(y.size()==1);
    BOOST_TEST(y[0]==1);
    }

    BOOST_AUTO_TEST_CASE( adder64_8448minus2051 )
    {
        //least significant bit first :^)
        auto finput = vector<int>{0,0,0,0,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                                  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                                  1,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                                  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
        auto C = circuitParser::parseCircuit("../tests/circuits/sub64.txt");
        auto feds = atecaGarble::Gb(64, C,"RO");
        auto encodedInput = atecaGarble::En(get<1>(feds), finput);
        auto Y = atecaGarble::Ev(get<0>(feds), encodedInput, C, get<3>(feds), get<4>(feds));
        auto y = atecaGarble::De(Y, get<2>(feds));

        util::printUintVec(y);

        BOOST_TEST(y.size()==1);
        BOOST_TEST(y[0]==6397);
    }

BOOST_AUTO_TEST_SUITE_END()