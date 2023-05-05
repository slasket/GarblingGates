//
// Created by a on 17/04/2023.
//


#include <boost/test/unit_test.hpp>
#include "../util/util.h"
#include "../util/hashTCCR.h"

using namespace boost::unit_test;

BOOST_AUTO_TEST_SUITE( TCCR_Encryption_128 )
    auto c = hashTCCR(128);
    vint x = {10,20};
    vint y = {2,4};
    vint tweak = {15};
    int internalLength = 8*128;
    auto cipher = c.hash(x,y,tweak,internalLength);
    auto plain = c.decypthash(cipher);


    BOOST_AUTO_TEST_CASE( test_decryption )
    {

        vint yFirstHalf(y.begin(),y.begin()+(y.size()/2));
        vint ySecondHalf(y.begin()+(y.size()/2),y.end());
        auto y0 = hashTCCR::gfmulPCF(c.getU1(), yFirstHalf);
        auto y1 = hashTCCR::gfmulPCF(c.getU2(), ySecondHalf);
        //compute e ^ U(Y)
        y0.insert(y0.end(), y1.begin(),y1.end());
        vint block0 = util::vecXOR(x,y0);
        //block0.emplace_back(tweak);
        if (!tweak.empty()){
            block0.insert(block0.end(),tweak.begin(),tweak.end());
        }
        //create the counters as 64 bit blocks
        vint input(internalLength/64);
        std::iota(input.begin(), input.end(), 1);
        input = util::vecXOR(input,block0);

        BOOST_TEST(plain==input);
    }
BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE( TCCR_Encryption_128_16internal )
    auto c = hashTCCR(128);
    vint x = util::genBitsNonCrypto(128);
    vint y = util::genBitsNonCrypto(128);
    vint tweak = {450};
    int internalLength = 16*128;
    auto cipher = c.hash(x,y,tweak,internalLength);
    auto plain = c.decypthash(cipher);
    BOOST_AUTO_TEST_CASE( test_decryption_16internal )
    {

        vint yFirstHalf(y.begin(),y.begin()+(y.size()/2));
        vint ySecondHalf(y.begin()+(y.size()/2),y.end());
        auto y0 = hashTCCR::gfmulPCF(c.getU1(), yFirstHalf);
        auto y1 = hashTCCR::gfmulPCF(c.getU2(), ySecondHalf);
        //compute e ^ U(Y)
        y0.insert(y0.end(), y1.begin(),y1.end());
        vint block0 = util::vecXOR(x,y0);
        //block0.emplace_back(tweak);
        if (!tweak.empty()){
            block0.insert(block0.end(),tweak.begin(),tweak.end());
        }
        //create the counters as 64 bit blocks
        vint input(internalLength/64);
        std::iota(input.begin(), input.end(), 1);
        input = util::vecXOR(input,block0);

        BOOST_TEST(plain==input);
    }

BOOST_AUTO_TEST_SUITE_END()


BOOST_AUTO_TEST_SUITE( TCCR_arbitrary_label_length )
    auto c = hashTCCR(384);
    vint x = util::genBitsNonCrypto(384);
    vint y = util::genBitsNonCrypto(384);
    vint tweak = {420};
    int internalLength = 8*384;
    auto cipher = c.hash(x,y,tweak,internalLength);
    auto plain = c.decypthash(cipher);
    BOOST_AUTO_TEST_CASE( test_decryption_arbitrary_length )
    {
        vint yFirstHalf(y.begin(),y.begin()+(y.size()/2));
        vint ySecondHalf(y.begin()+(y.size()/2),y.end());
        auto y0 = hashTCCR::gfmulPCF(c.getU1(), yFirstHalf);
        auto y1 = hashTCCR::gfmulPCF(c.getU2(), ySecondHalf);
        //compute e ^ U(Y)
        y0.insert(y0.end(), y1.begin(),y1.end());
        vint block0 = util::vecXOR(x,y0);
        //block0.emplace_back(tweak);
        if (!tweak.empty()){
            block0.insert(block0.end(),tweak.begin(),tweak.end());
        }
        //create the counters as 64 bit blocks
        vint input(internalLength/64);
        std::iota(input.begin(), input.end(), 1);
        input = util::vecXOR(input,block0);

        BOOST_TEST(plain==input);
    }
BOOST_AUTO_TEST_SUITE_END()


BOOST_AUTO_TEST_SUITE( TCCR_torture_test_128bit )
    int bitsize = 128;
    BOOST_AUTO_TEST_CASE( test_decryption )
    {
        for (int i = 0; i < 100; ++i) {
            auto c = hashTCCR(bitsize);
            vint x = util::genBitsNonCrypto(bitsize);
            vint y = util::genBitsNonCrypto(bitsize);
            vint tweak = util::genBitsNonCrypto(64);
            int internalLength = 8*bitsize;
            auto cipher = c.hash(x,y,tweak,internalLength);
            auto plain = c.decypthash(cipher);
            //create the counters as 64 bit blocks
            vint yFirstHalf(y.begin(),y.begin()+(y.size()/2));
            vint ySecondHalf(y.begin()+(y.size()/2),y.end());
            auto y0 = hashTCCR::gfmulPCF(c.getU1(), yFirstHalf);
            auto y1 = hashTCCR::gfmulPCF(c.getU2(), ySecondHalf);
            //compute e ^ U(Y)
            y0.insert(y0.end(), y1.begin(),y1.end());
            vint block0 = util::vecXOR(x,y0);
            //block0.emplace_back(tweak);
            if (!tweak.empty()){
                block0.insert(block0.end(),tweak.begin(),tweak.end());
            }
            //create the counters as 64 bit blocks
            vint input(internalLength/64);
            std::iota(input.begin(), input.end(), 1);
            input = util::vecXOR(input,block0);
            BOOST_TEST(plain==input);
        }
    }
BOOST_AUTO_TEST_SUITE_END()

