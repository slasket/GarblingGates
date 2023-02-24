//
// Created by a on 17/02/2023.
//

#ifndef GARBLINGGATES_UTIL_H
#define GARBLINGGATES_UTIL_H
#include <string>
#include <sstream>
#include <vector>
#include <iterator>
#include <iostream>
#include "circuitParser.h"
#include <bitset>
#include <random>
#include "otUtil/otUtil.h"
#include <openssl/sha.h>
#include <openssl/aes.h>
#include <openssl/evp.h>
using namespace std;

class util {

public:
    //template for splitting strings taken from:
    // https://stackoverflow.com/questions/236129/how-do-i-iterate-over-the-words-of-a-string
    template <typename Out>
    static void split(const std::string &s, char delim, Out result) {
        std::istringstream iss(s);
        std::string item;
        while (std::getline(iss, item, delim)) {
            *result++ = item;
        }
    }
    static std::vector<std::string> split(const std::string &s, char delim) {
        std::vector<std::string> elems;
        split(s, delim, std::back_inserter(elems));
        return elems;
    }
    static void printUintVec(const std::vector<uint64_t>& vec){
        for (auto const& i: vec) {
        std::cout<< i << ' ';
        }
        std::cout<< std::endl;
    }
    static void printStrVec(const std::vector<std::string>& vec){
        for (auto const& i: vec) {
            std::cout<< i << std::endl;
        }
    }

    static void printCircuit(const std::string& path){
        auto res = circuitParser::parseCircuit(path);
        util::printStrVec(res);
    }

    static tuple<vector<uint64_t>,
            vector<tuple<vector<uint64_t>, vector<uint64_t>>>>
            generateRandomLabels(int k, vector<uint64_t>& globalDelta,
                                     vector<tuple<vector<uint64_t>, vector<uint64_t>>>& wiresLabels) {
                //generate new global delta if non is given
                if(globalDelta[0]==0){
                    globalDelta = otUtil::genBitsNonCrypto(k);
                    globalDelta[0] = globalDelta[0] | 1;
                }
                //generate input labels
                for(auto & wiresLabel : wiresLabels){
                    labelPointAndPermute(k, wiresLabel, globalDelta);
                }
                return make_tuple(globalDelta, wiresLabels);
    }

    static void labelPointAndPermute(int k,
                                     tuple<vector<uint64_t>, vector<uint64_t>> &wiresLabel,
                                        const vector<uint64_t>& globalDelta,
                                     int permuteBit = 2) {//generate random permute bit
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, 1);
        if (permuteBit >= 2) permuteBit = dis(gen);
        vector<uint64_t> a0 = otUtil::genBitsNonCrypto(k);
        //add permute bit
        a0[0] = a0[0] & -2; //1111 ... 1111 1110 THE COLOR BIT IS ON THE LEAST SIGNIFICANT BIT ON BLOCK ZERO
        a0[0] = a0[0] | permuteBit; //can be hardcoded as it is one block
        auto a1 = vector<uint64_t>(a0.size());
        for (int i = 0; i < a0.size(); ++i) {
            a1[i] = a0[i] ^ globalDelta[i];
        }
        wiresLabel = make_pair(a0, a1);
    }

    static vector <::uint64_t> genBitsNonCrypto(int bits) {
        auto res = vector<::uint64_t>((bits+64-1)/64);
        for (int blockNum = 0; blockNum <(bits+64-1)/64; ++blockNum) {
            res[blockNum] = random_bitset<64>().to_ullong();
            //for (int i = 0; i < 64; ++i) {bitset[i]=(prng.GenerateBit());}
            //res[blockNum]=bitset.to_ullong();
        }
        return res;
    }

    static vector<::uint64_t> bitVecXOR(vector<::uint64_t>left, const vector<::uint64_t>& right){
        for (int i = 0; i < left.size(); ++i) {
            left[i] = left[i] ^right[i];
        }
        return left;
    }
    //taken from https://www.appsloveworld.com/cplus/100/112/c-efficient-way-to-generate-random-bitset-with-configurable-mean-1s-to-0s-r
    template< size_t size>
    static typename std::bitset<size> random_bitset( double p = 0.5) {

        typename std::bitset<size> bits;
        std::random_device rd;
        std::mt19937 gen( rd());
        std::bernoulli_distribution d( p);

        for( int n = 0; n < size; ++n) {
            bits[n] = d(gen);
        }

        return bits;
    }
    //taken from https://helloacm.com/c-coding-exercise-number-of-1-bits-revisited/
    static int hammingWeight(uint64_t x) {
        x -= (x >> 1) & 0x5555555555555555;             //put count of each 2 bits into those 2 bits
        x = (x & 0x3333333333333333) + ((x >> 2) & 0x3333333333333333); //put count of each 4 bits into those 4 bits
        x = (x + (x >> 4)) & 0x0f0f0f0f0f0f0f0f;        //put count of each 8 bits into those 8 bits
        return (x * 0x0101010101010101) >> 56;  //returns left 8 bits of x + (x<<8) + (x<<16) + (x<<24) + ...
    }

    static inline int bitCount2(::uint64_t x) {

        const uint64_t m1  = 0x5555555555555555; //binary: 0101...
        const uint64_t m2  = 0x3333333333333333; //binary: 00110011..
        const uint64_t m4  = 0x0f0f0f0f0f0f0f0f; //binary:  4 zeros,  4 ones ...
        const uint64_t m8  = 0x00ff00ff00ff00ff; //binary:  8 zeros,  8 ones ...
        const uint64_t m16 = 0x0000ffff0000ffff; //binary: 16 zeros, 16 ones ...
        const uint64_t m32 = 0x00000000ffffffff; //binary: 32 zeros, 32 ones
        const uint64_t h01 = 0x0101010101010101; //the sum of 256 to the power of 0,1,2,3...

        x = (x & m1 ) + ((x >>  1) & m1 ); //put count of each  2 bits into those  2 bits
        x = (x & m2 ) + ((x >>  2) & m2 ); //put count of each  4 bits into those  4 bits
        x = (x & m4 ) + ((x >>  4) & m4 ); //put count of each  8 bits into those  8 bits
        x = (x & m8 ) + ((x >>  8) & m8 ); //put count of each 16 bits into those 16 bits
        x = (x & m16) + ((x >> 16) & m16); //put count of each 32 bits into those 32 bits
        x = (x & m32) + ((x >> 32) & m32); //put count of each 64 bits into those 64 bits
        return x;
    }

    //perform variable output length hash
    static vector<uint64_t> hash_variable(const std::string& input, int output_length_bits = 128)
    {
        size_t output_length_bytes = output_length_bits / 8;
        EVP_MD_CTX* ctx = EVP_MD_CTX_create();
        EVP_DigestInit_ex(ctx, EVP_shake256(), NULL);
        EVP_DigestUpdate(ctx, input.c_str(), input.size());
        std::string output(output_length_bytes, '\0');
        EVP_DigestFinalXOF(ctx, reinterpret_cast<unsigned char *>(&output[0]), output_length_bytes);
        EVP_MD_CTX_destroy(ctx);
        //convert output to vector<uint64_t>
        vector<uint64_t> output_vector;
        for (int i = 0; i < output_length_bytes; i+=8) {
            uint64_t temp = 0;
            for (int j = 0; j < 8; ++j) {
                temp += (uint64_t)output[i+j] << (j*8);
            }
            output_vector.push_back(temp);
        }
        return output_vector;
    }


};



#endif //GARBLINGGATES_UTIL_H
