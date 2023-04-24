//
// Created by a on 4/24/23.
//

#include <boost/test/unit_test.hpp>
#include "../util/util.h"
#include "../schemes/atecaGarble.h"
#include "../schemes/atecaFreeXOR.h"
#include "../util/circuitParser.h"

using namespace boost::unit_test;


BOOST_AUTO_TEST_SUITE( ATECA_adder_torture )
    auto C = circuitParser::parseCircuit("../tests/circuits/adder64.txt");
    int l = 128;
    BOOST_AUTO_TEST_CASE( adder_torture)
    {
        for (int i = 0; i < 100; ++i) {
            auto input = util::genFunctionInput(128);
            auto Slowfeds00 = atecaGarble::garble(C, l, util::RO);
            auto SlowX00 = atecaGarble::encode(get<1>(Slowfeds00), input);
            auto SlowY00 = atecaGarble::eval(get<0>(Slowfeds00), SlowX00, C, get<3>(Slowfeds00), get<4>(Slowfeds00), hashTCCR());
            auto Slowy00 = atecaGarble::decode(SlowY00, get<2>(Slowfeds00), hashTCCR());

            auto Fastfeds00 = atecaGarble::garble(C, l, util::fast);
            auto FastX00 = atecaGarble::encode(get<1>(Fastfeds00), input);
            auto FastY00 = atecaGarble::eval(get<0>(Fastfeds00), FastX00, C, get<3>(Fastfeds00), get<4>(Fastfeds00), get<5>(Fastfeds00));
            auto Fasty00 = atecaGarble::decode(FastY00, get<2>(Fastfeds00), get<5>(Fastfeds00));

            BOOST_TEST(Slowy00[0]==Fasty00[0]);
        }
    }
BOOST_AUTO_TEST_SUITE_END()


BOOST_AUTO_TEST_SUITE( ATECAfreexor_adder_torture )
    auto C = circuitParser::parseCircuit("../tests/circuits/adder64.txt");
    int l = 128;
    BOOST_AUTO_TEST_CASE( adder_torture)
    {
        for (int i = 0; i < 100; ++i) {
            auto input = util::genFunctionInput(128);
            auto Slowfeds00 = atecaFreeXOR::garble(C, l, util::RO);
            auto SlowX00 = atecaFreeXOR::encode(get<1>(Slowfeds00), input);
            auto SlowY00 = atecaFreeXOR::eval(get<0>(Slowfeds00), SlowX00, C, get<3>(Slowfeds00), get<4>(Slowfeds00), hashTCCR());
            auto Slowy00 = atecaFreeXOR::decode(SlowY00, get<2>(Slowfeds00), hashTCCR());

            auto Fastfeds00 = atecaFreeXOR::garble(C, l, util::fast);
            auto FastX00 = atecaFreeXOR::encode(get<1>(Fastfeds00), input);
            auto FastY00 = atecaFreeXOR::eval(get<0>(Fastfeds00), FastX00, C, get<3>(Fastfeds00), get<4>(Fastfeds00), get<5>(Fastfeds00));
            auto Fasty00 = atecaFreeXOR::decode(FastY00, get<2>(Fastfeds00), get<5>(Fastfeds00));

            BOOST_TEST(Slowy00[0]==Fasty00[0]);
        }
    }
BOOST_AUTO_TEST_SUITE_END()