//
// Created by simon on 08/03/2023.
//
#include <boost/test/unit_test.hpp>
#include "../util/util.h"
#include "../schemes/threeHalves.h"

using namespace boost::unit_test;

BOOST_AUTO_TEST_SUITE( Testing_ThreeHalves )

    vector<string> smalltest = {"1 3", "2 1 1", "1 1", "2 1 0 1 2 XOR"};
    auto output = threeHalves::garble(128, smalltest);
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

BOOST_AUTO_TEST_SUITE( Testing_ThreeHalvesAnd )

    vector<string> smalltest = {"1 3", "2 1 1", "1 1", "2 1 0 1 2 AND"};
    auto output = threeHalves::garble(128, smalltest);
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

BOOST_AUTO_TEST_SUITE( Testing_ThreeHalvesEncode )

    vector<string> smalltest = {"1 3", "2 1 1", "1 1", "2 1 0 1 2 AND"};
    auto output = threeHalves::garble(128, smalltest);
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
    auto y = threeHalves::decode(d, Y, smalltest, 128);
    BOOST_AUTO_TEST_CASE( test_Decode )
    {
        BOOST_TEST(y.size() == 1);
    }*/
BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE( Testing_ThreeHalvesEvalXOR )

    vector<string> smalltest = {"1 3", "2 1 1", "1 1", "2 1 0 1 2 XOR"};
    auto output = threeHalves::garble(128, smalltest);
    auto F = get<0>(output);
    auto e = get<1>(output);
    auto d = get<2>(output);


    BOOST_AUTO_TEST_CASE( test_Eval1xor1 )
    {
        auto x = vector<int>{1, 1};
        auto encLabels = threeHalves::encode(e, x);
        auto Y = threeHalves::eval(F, encLabels, smalltest, 128);
        auto y = threeHalves::decode(d, Y, smalltest, 128);
        BOOST_TEST(Y.size() == 1);
        BOOST_TEST(y.size() == 1);
        BOOST_TEST(y[0] == 0);
    }
    BOOST_AUTO_TEST_CASE( test_Eval0xor1 )
    {
        auto x = vector<int>{0, 1};
        auto encLabels = threeHalves::encode(e, x);
        auto Y = threeHalves::eval(F, encLabels, smalltest, 128);
        auto y = threeHalves::decode(d, Y, smalltest, 128);
        BOOST_TEST(y.size() == 1);
        BOOST_TEST(y[0] == 1);
    }
    BOOST_AUTO_TEST_CASE( test_Eval1xor0 )
    {
        auto x = vector<int>{1, 0};
        auto encLabels = threeHalves::encode(e, x);
        auto Y = threeHalves::eval(F, encLabels, smalltest, 128);
        auto y = threeHalves::decode(d, Y, smalltest, 128);
        BOOST_TEST(y.size() == 1);
        BOOST_TEST(y[0] == 1);
    }
    BOOST_AUTO_TEST_CASE( test_Eval0xor0 )
    {
        auto x = vector<int>{0, 0};
        auto encLabels = threeHalves::encode(e, x);
        auto Y = threeHalves::eval(F, encLabels, smalltest, 128);
        auto y = threeHalves::decode(d, Y, smalltest, 128);
        BOOST_TEST(y.size() == 1);
        BOOST_TEST(y[0] == 0);
    }


BOOST_AUTO_TEST_SUITE_END()


BOOST_AUTO_TEST_SUITE( Testing_ThreeHalvesEvalAND )

    vector<string> smalltest = {"1 3", "2 1 1", "1 1", "2 1 0 1 2 AND"};

    BOOST_AUTO_TEST_CASE( test_Eval1and1 )
    {
        for (int i = 0; i < 10; ++i) {
            auto output = threeHalves::garble(128, smalltest);
            auto x = vector<int>{1, 1};
            auto F = get<0>(output);
            auto e = get<1>(output);
            auto d = get<2>(output);
            auto encLabels = threeHalves::encode(e, x);
            auto Y = threeHalves::eval(F, encLabels, smalltest, 128);
            auto y = threeHalves::decode(d, Y, smalltest, 128);
            BOOST_TEST(y.size() == 1);
            if(y.size() == 1) BOOST_TEST(y[0] == 1);
        }
    }

BOOST_AUTO_TEST_SUITE_END()
