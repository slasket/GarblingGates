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
    int tweak = 15;
    int internalLength = 8*128;
    auto cipher = hashTCCR::hash(x,y,c.getIv(),c.e,c.getU1(),c.getU2(),tweak,internalLength);
    auto plain = hashTCCR::decypthash(x,y,c.getIv(),c.e,c.getU1(),c.getU2(),cipher);
    BOOST_AUTO_TEST_CASE( test_decryption )
    {
        vint input;
        //create the counters as 64 bit blocks
        for (int i = 0; i < (internalLength/64); ++i) {
            if (i==0){
                input.emplace_back(i^tweak);
            }else{
                input.emplace_back(i);
            }
        }
        BOOST_TEST(plain==input);
    }
BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE( TCCR_Encryption_128_16internal )
    auto c = hashTCCR(128);
    vint x = util::genBitsNonCrypto(128);
    vint y = util::genBitsNonCrypto(128);
    int tweak = 450;
    int internalLength = 16*128;
    auto cipher = hashTCCR::hash(x,y,c.getIv(),c.e,c.getU1(),c.getU2(),tweak,internalLength);
    auto plain = hashTCCR::decypthash(x,y,c.getIv(),c.e,c.getU1(),c.getU2(),cipher);
    BOOST_AUTO_TEST_CASE( test_decryption_16internal )
    {
        vint input;
        //create the counters as 64 bit blocks
        for (int i = 0; i < (internalLength/64); ++i) {
            if (i==0){
                input.emplace_back(i^tweak);
            }else{
                input.emplace_back(i);
            }
        }
        BOOST_TEST(plain==input);
    }
BOOST_AUTO_TEST_SUITE_END()


BOOST_AUTO_TEST_SUITE( TCCR_arbitrary_label_length )
    auto c = hashTCCR(384);
    vint x = util::genBitsNonCrypto(384);
    vint y = util::genBitsNonCrypto(384);
    int tweak = 420;
    int internalLength = 8*384;
    auto cipher = hashTCCR::hash(x,y,c.getIv(),c.e,c.getU1(),c.getU2(),tweak,internalLength);
    auto plain = hashTCCR::decypthash(x,y,c.getIv(),c.e,c.getU1(),c.getU2(),cipher);
    BOOST_AUTO_TEST_CASE( test_decryption )
    {
        vint input;
        //create the counters as 64 bit blocks
        for (int i = 0; i < (internalLength/64); ++i) {
            if (i==0){
                input.emplace_back(i^tweak);
            }else{
                input.emplace_back(i);
            }
        }
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
            int tweak = 15;
            int internalLength = 8*bitsize;
            auto cipher = hashTCCR::hash(x,y,c.getIv(),c.e,c.getU1(),c.getU2(),tweak,internalLength);
            auto plain = hashTCCR::decypthash(x,y,c.getIv(),c.e,c.getU1(),c.getU2(),cipher);
            vint input;
            //create the counters as 64 bit blocks
            for (int i = 0; i < (internalLength/64); ++i) {
                if (i==0){
                    input.emplace_back(i^tweak);
                }else{
                    input.emplace_back(i);
                }
            }
            BOOST_TEST(plain==input);
        }
    }
BOOST_AUTO_TEST_SUITE_END()

