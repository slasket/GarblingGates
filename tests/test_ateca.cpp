//
// Created by a on 27/02/2023.
//
//
#include <boost/test/unit_test.hpp>
#include "../util/util.h"
#include "../schemes/atecaGarble.h"
#include "../bloodcompatibility.h"
#include "../util/circuitParser.h"

using namespace boost::unit_test;


BOOST_AUTO_TEST_SUITE( ATECA_garbling_values )
    auto C = circuitParser::parseCircuit("../tests/circuits/BloodComp.txt");
    int l = 64;
    auto feds = atecaGarble::garble(C, l, util::RO);
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

BOOST_AUTO_TEST_SUITE( ATECA_Test_projection )

    auto b = vector<::uint64_t>{15,0};
    auto a = vector<::uint64_t>{9,0};
    int hwb= util::vecHW(b);


    BOOST_AUTO_TEST_CASE( simpleProjection )
    {
        auto res = util::projection(a,b);
        auto res2 = util::fastproj(a,b,hwb);
        BOOST_TEST(res.size()==1);
        // this is equivilant to 1001 with 60 zeros after
        BOOST_TEST(res[0]==10376293541461622784);
        BOOST_TEST(res2[0]==10376293541461622784);
    }

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE( ATECA_Test_large_projection )
    auto b = vector<::uint64_t>{152109056,0};
    auto a = vector<::uint64_t>{134283264,0};
    int hwb= util::vecHW(b);


    auto b1 = vector<::uint64_t>{4755837490663786591,0};
    auto a1 = vector<::uint64_t>{5908722711386916953,0};
    int hwb1= util::vecHW(b1);
    BOOST_AUTO_TEST_CASE( largerVec )
    {
        auto res = util::projection(a,b);
        auto res2 = util::fastproj(a,b,hwb);
        BOOST_TEST(res.size()==1);
        BOOST_TEST(res[0]==10376293541461622784);
        BOOST_TEST(res2[0]==10376293541461622784);
        res = util::projection(a1,b1);
        res2 = util::fastproj(a1,b1,hwb1);
        BOOST_TEST(res[0]==14972216961193213952);
        BOOST_TEST(res2[0]==14972216961193213952);

    }
BOOST_AUTO_TEST_SUITE_END()


BOOST_AUTO_TEST_SUITE( ATECA_input_encoding_choice )
    auto finput = vector<int>{0,0,0,0,0,0,1};
    auto C = circuitParser::parseCircuit("../tests/circuits/BloodComp.txt");
    int l = 128;
    auto feds = atecaGarble::garble(C, l, util::RO);
    auto F = get<0>(feds);
    auto encodingInfo = get<1>(feds);
    auto decoding = get<2>(feds);
    BOOST_AUTO_TEST_CASE( encoding_choice )
    {
        //compare chosen input with encoding
        auto encodedInput = atecaGarble::encode(get<1>(feds), finput);
        BOOST_TEST(get<0>(encodingInfo[5])[0]==encodedInput[5][0]);
        BOOST_TEST(get<0>(encodingInfo[6])[0]!=encodedInput[6][0]);
    }

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE( ATECA_garbled_Output )
    auto finput = vector<int>{0,0,0,0,0,0,1};
    auto C = circuitParser::parseCircuit("../tests/circuits/BloodComp.txt");
    int l = 128;
    auto feds = atecaGarble::garble(C, l, util::RO);
    auto encodedInput = atecaGarble::encode(get<1>(feds), finput);
    auto Y = atecaGarble::eval(get<0>(feds), encodedInput, C, get<3>(feds), get<4>(feds), hashTCCR());
    BOOST_AUTO_TEST_CASE( garbled_output )
    {
        BOOST_TEST(Y.size()==1);
        BOOST_TEST(Y[0][0]!=0);
        BOOST_TEST(Y[0].size() ==2);
    }
BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE( ATECA_decoding_garbled_Y )
    auto finput = vector<int>{0,0,0,0,0,0,1};
    auto C = circuitParser::parseCircuit("../tests/circuits/BloodComp.txt");
    int l = 128;
    auto feds = atecaGarble::garble(C, l, util::RO);
    auto encodedInput = atecaGarble::encode(get<1>(feds), finput);
    auto Y = atecaGarble::eval(get<0>(feds), encodedInput, C, get<3>(feds), get<4>(feds), hashTCCR());
    auto y = atecaGarble::decode(Y, get<2>(feds), hashTCCR());
    BOOST_AUTO_TEST_CASE( decoding_Y )
    {
        BOOST_TEST(y.size()==1);
        //BOOST_TEST(y[0]==1);
    }
BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE( ATECA_xorTest )
    auto finput1 = vector<int>{0,0};
    auto finput2 = vector<int>{0,1};
    auto C = circuitParser::parseCircuit("../tests/circuits/xorTest.txt");
    int l = 128;
    auto feds = atecaGarble::garble(C, l, util::RO);
    auto encodingInfo = get<1>(feds);
    auto encodedInput = atecaGarble::encode(get<1>(feds), finput1);
    auto Y = atecaGarble::eval(get<0>(feds), encodedInput, C, get<3>(feds), get<4>(feds), hashTCCR());
    auto y = atecaGarble::decode(Y, get<2>(feds), hashTCCR());
    BOOST_AUTO_TEST_CASE( encoding_choice )
    {
        BOOST_TEST(get<0>(encodingInfo[1])[0]==encodedInput[1][0]);
        encodedInput = atecaGarble::encode(get<1>(feds), finput2);
        BOOST_TEST(get<1>(encodingInfo[1])[0]==encodedInput[1][0]);
    }

    BOOST_AUTO_TEST_CASE( xor_Output )
    {
        BOOST_TEST(y.size()==1);
        BOOST_TEST(y[0]==0);
    }
BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE( ATECA_andTest )
    auto finput1 = vector<int>{1,1};
    auto finput2 = vector<int>{0,1};
    auto C = circuitParser::parseCircuit("../tests/circuits/andTest.txt");
    int l = 128;
    auto feds = atecaGarble::garble(C, l, util::RO);
    auto encodingInfo = get<1>(feds);
    auto encodedInput = atecaGarble::encode(get<1>(feds), finput1);
    auto Y = atecaGarble::eval(get<0>(feds), encodedInput, C, get<3>(feds), get<4>(feds), hashTCCR());
    auto y = atecaGarble::decode(Y, get<2>(feds), hashTCCR());

    BOOST_AUTO_TEST_CASE( encoding_choice )
    {
        BOOST_TEST(get<1>(encodingInfo[0])[0]==encodedInput[0][0]);
        encodedInput = atecaGarble::encode(get<1>(feds), finput2);
        BOOST_TEST(get<0>(encodingInfo[0])[0]==encodedInput[0][0]);
    }


    BOOST_AUTO_TEST_CASE( and_output )
    {
        BOOST_TEST(y.size()==1);
        BOOST_TEST(y[0]==1);
    }
BOOST_AUTO_TEST_SUITE_END()



BOOST_AUTO_TEST_SUITE( ATECA_BloodComp_Alternate )
    int lInput =6; int rInput = 1;
    auto finput = vector<int>{1,1,0,0,0,1,1};
    auto C = circuitParser::parseCircuit("../tests/circuits/BloodComp.txt");
    int l = 128;
    auto feds = atecaGarble::garble(C, l, util::RO);
    auto encodedInput = atecaGarble::encode(get<1>(feds), finput);
    auto Y = atecaGarble::eval(get<0>(feds), encodedInput, C, get<3>(feds), get<4>(feds), hashTCCR());
    auto y = atecaGarble::decode(Y, get<2>(feds), hashTCCR());
    ::uint64_t bloodCompAns = bloodcompatibility::bloodCompLookup(lInput,rInput);
    BOOST_AUTO_TEST_CASE( decoding_Y )
    {
        BOOST_TEST(y.size()==1);
        BOOST_TEST(y[0]==bloodCompAns);
    }
BOOST_AUTO_TEST_SUITE_END()


BOOST_AUTO_TEST_SUITE( ATECA_adder64bit )

    BOOST_AUTO_TEST_CASE( adder64Adding1And1 )
    {
        //least significant bit first :^)
        auto finput = vector<int>{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                                  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                                  1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                                  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
        auto C = circuitParser::parseCircuit("../tests/circuits/adder64.txt");
        auto feds = atecaGarble::garble(C, 128, util::RO);
        auto encodedInput = atecaGarble::encode(get<1>(feds), finput);
        auto Y = atecaGarble::eval(get<0>(feds), encodedInput, C, get<3>(feds), get<4>(feds), hashTCCR());
        auto y = atecaGarble::decode(Y, get<2>(feds), hashTCCR());

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
        auto feds = atecaGarble::garble(C, 128, util::RO);
        auto encodedInput = atecaGarble::encode(get<1>(feds), finput);
        auto Y = atecaGarble::eval(get<0>(feds), encodedInput, C, get<3>(feds), get<4>(feds), hashTCCR());
        auto y = atecaGarble::decode(Y, get<2>(feds), hashTCCR());

        //util::printUintVec(y);

        BOOST_TEST(y.size()==1);
        BOOST_TEST(y[0]==maxPlusMax);
    }

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE( ATECA_sub64 )

    BOOST_AUTO_TEST_CASE( adder64_2minus1 )
    {
    //least significant bit first :^)
    auto finput = vector<int>{0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                              0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                              1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                              0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    auto C = circuitParser::parseCircuit("../tests/circuits/sub64.txt");
    auto feds = atecaGarble::garble(C, 128, util::RO);
    auto encodedInput = atecaGarble::encode(get<1>(feds), finput);
    auto Y = atecaGarble::eval(get<0>(feds), encodedInput, C, get<3>(feds), get<4>(feds), hashTCCR());
    auto y = atecaGarble::decode(Y, get<2>(feds), hashTCCR());


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
        auto feds = atecaGarble::garble(C, 128, util::RO);
        auto encodedInput = atecaGarble::encode(get<1>(feds), finput);
        auto Y = atecaGarble::eval(get<0>(feds), encodedInput, C, get<3>(feds), get<4>(feds), hashTCCR());
        auto y = atecaGarble::decode(Y, get<2>(feds), hashTCCR());

        BOOST_TEST(y.size()==1);
        BOOST_TEST(y[0]==6397);
    }

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE( ATECA_fast_hash_simple )
    int lInput =6; int rInput = 1;
    auto finput = vector<int>{1,1,0,0,0,1,1};
    auto C = circuitParser::parseCircuit("../tests/circuits/BloodComp.txt");
    int l = 128;
    auto feds = atecaGarble::garble(C, l, util::fast);
    auto encodedInput = atecaGarble::encode(get<1>(feds), finput);
    auto Y = atecaGarble::eval(get<0>(feds), encodedInput, C, get<3>(feds), get<4>(feds), get<5>(feds));
    auto y = atecaGarble::decode(Y, get<2>(feds), get<5>(feds));
    ::uint64_t bloodCompAns = bloodcompatibility::bloodCompLookup(lInput,rInput);
    BOOST_AUTO_TEST_CASE( decoding_Y )
    {
        BOOST_TEST(y.size()==1);
        BOOST_TEST(y[0]==bloodCompAns);
    }
BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE( ATECA_fasthash_adder64bit )
    int k = 128;
    BOOST_AUTO_TEST_CASE( fast_adder64Adding1And1 )
    {
        //least significant bit first :^)
        auto finput = vector<int>{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                                  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                                  1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                                  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
        auto C = circuitParser::parseCircuit("../tests/circuits/adder64.txt");
        auto feds = atecaGarble::garble(C, k, util::fast);
        auto encodedInput = atecaGarble::encode(get<1>(feds), finput);
        auto Y = atecaGarble::eval(get<0>(feds), encodedInput, C, get<3>(feds), get<4>(feds), get<5>(feds));
        auto y = atecaGarble::decode(Y, get<2>(feds), get<5>(feds));

        //util::printUintVec(y);

        BOOST_TEST(y.size()==1);
        BOOST_TEST(y[0]==2);
    }

    BOOST_AUTO_TEST_CASE( fast_adder64AddingMaxWMax )
    {
        //least significant bit first :^)
        auto maxPlusMax = UINT64_MAX<<1;
        auto finput = vector<int>{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                                  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                                  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                                  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};
        auto C = circuitParser::parseCircuit("../tests/circuits/adder64.txt");
        auto feds = atecaGarble::garble(C, k, util::fast);
        auto encodedInput = atecaGarble::encode(get<1>(feds), finput);
        auto Y = atecaGarble::eval(get<0>(feds), encodedInput, C, get<3>(feds), get<4>(feds), get<5>(feds));
        auto y = atecaGarble::decode(Y, get<2>(feds), get<5>(feds));

        //util::printUintVec(y);

        BOOST_TEST(y.size()==1);
        BOOST_TEST(y[0]==maxPlusMax);
    }

BOOST_AUTO_TEST_SUITE_END()


BOOST_AUTO_TEST_SUITE( ATECA_fast_sub64 )

    BOOST_AUTO_TEST_CASE( adder64_2minus1 )
    {
        //least significant bit first :^)
        auto finput = vector<int>{0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                                  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                                  1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                                  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
        auto C = circuitParser::parseCircuit("../tests/circuits/sub64.txt");
        auto feds = atecaGarble::garble(C, 128, util::fast);
        auto encodedInput = atecaGarble::encode(get<1>(feds), finput);
        auto Y = atecaGarble::eval(get<0>(feds), encodedInput, C, get<3>(feds), get<4>(feds), get<5>(feds));
        auto y = atecaGarble::decode(Y, get<2>(feds), get<5>(feds));


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
        auto feds = atecaGarble::garble(C, 128, util::fast);
        auto encodedInput = atecaGarble::encode(get<1>(feds), finput);
        auto Y = atecaGarble::eval(get<0>(feds), encodedInput, C, get<3>(feds), get<4>(feds), get<5>(feds));
        auto y = atecaGarble::decode(Y, get<2>(feds), get<5>(feds));

        BOOST_TEST(y.size()==1);
        BOOST_TEST(y[0]==6397);
    }

BOOST_AUTO_TEST_SUITE_END()