//
// Created by a on 4/24/23.
//

#include <boost/test/unit_test.hpp>
#include "../util/util.h"
#include "../schemes/atecaGarble.h"
#include "../schemes/atecaFreeXOR.h"
#include "../schemes/baseGarble.h"
#include "../schemes/threeHalves.h"
#include "../util/circuitParser.h"

using namespace boost::unit_test;


BOOST_AUTO_TEST_SUITE( ATECA_adder_torture )
    auto C = circuitParser::parse("../tests/circuits/adder64.txt");
    int l = 128;
    BOOST_AUTO_TEST_CASE( adder_torture)
    {
        for (int i = 0; i < 100; ++i) {
            auto input = util::genFunctionInput(128);
            auto Slowfeds00 = atecaGarble::garble(C, l, util::RO);
            auto SlowX00 = atecaGarble::encode(get<1>(Slowfeds00), input);
            auto SlowY00 = atecaGarble::eval(get<0>(Slowfeds00), SlowX00, C, get<3>(Slowfeds00), get<4>(Slowfeds00), get<5>(Slowfeds00));
            auto Slowy00 = atecaGarble::decode(SlowY00, get<2>(Slowfeds00), get<5>(Slowfeds00));

            auto Fastfeds00 = atecaGarble::garble(C, l, util::fast);
            auto FastX00 = atecaGarble::encode(get<1>(Fastfeds00), input);
            auto FastY00 = atecaGarble::eval(get<0>(Fastfeds00), FastX00, C, get<3>(Fastfeds00), get<4>(Fastfeds00), get<5>(Fastfeds00));
            auto Fasty00 = atecaGarble::decode(FastY00, get<2>(Fastfeds00), get<5>(Fastfeds00));

            BOOST_TEST(Slowy00[0]==Fasty00[0]);
        }
    }
BOOST_AUTO_TEST_SUITE_END()


BOOST_AUTO_TEST_SUITE( ATECAfreexor_adder_torture )
    auto C = circuitParser::parse("../tests/circuits/adder64.txt");
    int l = 128;
    BOOST_AUTO_TEST_CASE( adder_torture)
    {
        for (int i = 0; i < 100; ++i) {
            auto input = util::genFunctionInput(128);
            auto Slowfeds00 = atecaFreeXOR::garble(C, l, util::RO);
            auto SlowX00 = atecaFreeXOR::encode(get<1>(Slowfeds00), input);
            auto SlowY00 = atecaFreeXOR::eval(get<0>(Slowfeds00), SlowX00, C, get<3>(Slowfeds00), get<4>(Slowfeds00), get<5>(Slowfeds00));
            auto Slowy00 = atecaFreeXOR::decode(SlowY00, get<2>(Slowfeds00), get<5>(Slowfeds00));

            auto Fastfeds00 = atecaFreeXOR::garble(C, l, util::fast);
            auto FastX00 = atecaFreeXOR::encode(get<1>(Fastfeds00), input);
            auto FastY00 = atecaFreeXOR::eval(get<0>(Fastfeds00), FastX00, C, get<3>(Fastfeds00), get<4>(Fastfeds00), get<5>(Fastfeds00));
            auto Fasty00 = atecaFreeXOR::decode(FastY00, get<2>(Fastfeds00), get<5>(Fastfeds00));

            BOOST_TEST(Slowy00[0]==Fasty00[0]);
        }
    }
BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE( Baseline_adder_torture )
    auto C = circuitParser::parse("../tests/circuits/adder64.txt");
    int l = 128;
    BOOST_AUTO_TEST_CASE( adder_torture)
    {
        for (int i = 0; i < 100; ++i) {
            auto input = util::genFunctionInput(128);
            auto [slowF,slowe,slowd] = baseGarble::garble(C, l, util::RO);
            auto slowhash = get<2>(slowF);
            auto SlowX00 = baseGarble::encode(slowe, input);
            auto SlowY00 = baseGarble::eval(slowF, SlowX00, C, l);
            auto Slowy00 = baseGarble::decode(slowd, SlowY00, l, slowhash);

            auto [fastF,faste,fastd] = baseGarble::garble(C, l, util::fast);
            auto fasthash = get<2>(fastF);
            auto FastX00 = baseGarble::encode(faste, input);
            auto FastY00 = baseGarble::eval(fastF, FastX00, C, l);
            auto Fasty00 = baseGarble::decode(fastd, FastY00, l, fasthash);

            BOOST_TEST(Slowy00[0]==Fasty00[0]);
        }
    }
BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE( Threehalves_adder_torture )
    auto C = circuitParser::parse("../tests/circuits/adder64.txt");
    int l = 128;
    BOOST_AUTO_TEST_CASE( adder_torture)
    {
        for (int i = 0; i < 100; ++i) {
            auto input = util::genFunctionInput(128);
            auto [slowF,slowe,slowd,slowic, slowhash] = threeHalves::garble(C, l, util::RO);
            auto SlowX00 = threeHalves::encode(slowe, input);
            auto SlowY00 = threeHalves::eval(slowF, SlowX00, C, l, slowic, slowhash, util::RO);
            auto Slowy00 = threeHalves::decode(slowd, SlowY00, C, l);

            auto [fastF,faste,fastd,fastic, fasthash] = threeHalves::garble(C, l, util::fast);
            auto FastX00 = threeHalves::encode(faste, input);
            auto FastY00 = threeHalves::eval(fastF, FastX00, C, l, fastic, fasthash, util::fast);
            auto Fasty00 = threeHalves::decode(fastd, FastY00, C, l);

            BOOST_TEST(Slowy00[0]==Fasty00[0]);
        }
    }
BOOST_AUTO_TEST_SUITE_END()
