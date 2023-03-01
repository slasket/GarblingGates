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

BOOST_AUTO_TEST_SUITE( Test_projection )

    auto b = vector<::uint64_t>{15, 1};
    auto a = vector<::uint64_t>{9,1};

    BOOST_AUTO_TEST_CASE( simpleProjection )
    {
        auto res = atecaGarble::projection(a,b);
        BOOST_TEST(res.size()==1);
        BOOST_TEST(res[0]==9);
    }

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE( Test_large_projection )
    auto a = vector<::uint64_t>{2459528346497712128,0};
    auto b = vector<::uint64_t>{153685337284018176,0};

    BOOST_AUTO_TEST_CASE( largerVec )
    {
        auto res = atecaGarble::projection(a,b);
        BOOST_TEST(res.size()==1);
        BOOST_TEST(res[0]==3);
    }
BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE( Test_proj2 )
    auto a00 = vector<::uint64_t>{11345453849178678406,2414643011882828174,410223933184385734,2900740621283656324,6384287341940815535,7007689069576576446,1625778069175449138,221916738628169630};
    auto a01 = vector<::uint64_t>{5303979585819517795,14061915153323311439,17591262356517225162,8332875961431080939,16424615137417519418,2793831725793832469,4376373128166291593,11841307970501769865};
    auto delta = vector<::uint64_t>{4825893990096960,2660501997484054568,14857412833632876144,92359793590308,0,0,0,0};

    BOOST_AUTO_TEST_CASE( largetest2 )
    {
        auto res0 = atecaGarble::projection(a00,delta);
        auto res1 = atecaGarble::projection(a01,delta);
        BOOST_TEST(res0[0]==9873663595387038319);
        BOOST_TEST(res1[0]==6513909213479470619);
        cout << res0[0] << endl;
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

BOOST_AUTO_TEST_SUITE( xorTest )
    auto finput1 = vector<int>{0,0};
    auto finput2 = vector<int>{0,1};
    auto C = circuitParser::parseCircuit("../tests/circuits/xorTest.txt");
    int l = 64;
    auto feds = atecaGarble::GbLEAK(l, C);
    auto encodingInfo = get<1>(feds);
    auto encodedInput = atecaGarble::En(get<1>(feds), finput1);
    auto Y = atecaGarble::Ev(get<0>(feds), encodedInput, C, get<3>(feds));
    auto y = atecaGarble::De(Y, get<2>(feds));
    BOOST_AUTO_TEST_CASE( encoding_choice )
    {
        BOOST_TEST(get<0>(encodingInfo[1])[0]==encodedInput[1][0]);
        encodedInput = atecaGarble::En(get<1>(feds), finput2);
        BOOST_TEST(get<1>(encodingInfo[1])[0]==encodedInput[1][0]);
    }
    BOOST_AUTO_TEST_CASE( evalXor )
    {
        auto outputLabels= get<4>(feds);
        BOOST_TEST(Y[0]==get<0>(outputLabels[0]));
        encodedInput = atecaGarble::En(get<1>(feds), finput2);
        Y = atecaGarble::Ev(get<0>(feds), encodedInput, C, get<3>(feds));
        BOOST_TEST(Y[0]==get<1>(outputLabels[0]));
    }
    BOOST_AUTO_TEST_CASE( xor_Output )
    {
        auto encodedInput = atecaGarble::En(get<1>(feds), finput1);
        auto Y = atecaGarble::Ev(get<0>(feds), encodedInput, C, get<3>(feds));
        auto y = atecaGarble::De(Y, get<2>(feds));
        BOOST_TEST(y.size()==1);
        BOOST_TEST(y[0]==0);
    }
BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE( andTest )
    auto finput1 = vector<int>{1,1};
    auto finput2 = vector<int>{0,1};
    auto C = circuitParser::parseCircuit("../tests/circuits/andTest.txt");
    int l = 64;
    auto feds = atecaGarble::GbLEAK(l, C);
    auto encodingInfo = get<1>(feds);
    auto encodedInput = atecaGarble::En(get<1>(feds), finput1);
    auto Y = atecaGarble::Ev(get<0>(feds), encodedInput, C, get<3>(feds));
    auto y = atecaGarble::De(Y, get<2>(feds));

    BOOST_AUTO_TEST_CASE( encoding_choice )
    {
        BOOST_TEST(get<1>(encodingInfo[0])[0]==encodedInput[0][0]);
        encodedInput = atecaGarble::En(get<1>(feds), finput2);
        BOOST_TEST(get<0>(encodingInfo[0])[0]==encodedInput[0][0]);
    }
    BOOST_AUTO_TEST_CASE( evalAND )
    {
        auto outputLabels= get<4>(feds);
        BOOST_TEST(Y[0]==get<1>(outputLabels[0]));
        encodedInput = atecaGarble::En(get<1>(feds), finput2);
        Y = atecaGarble::Ev(get<0>(feds), encodedInput, C, get<3>(feds));
        BOOST_TEST(Y[0]==get<0>(outputLabels[0]));
    }


    BOOST_AUTO_TEST_CASE( and_output )
    {
        BOOST_TEST(y.size()==1);
        BOOST_TEST(y[0]==0);
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
