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
#include <cryptoTools/Common/Defines.h>
#include "otUtil/otUtil.h"
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

    static vector <oc::u64> genBitsNonCrypto(int bits) {
        auto res = vector<oc::u64>((bits+64-1)/64);
        for (int blockNum = 0; blockNum <(bits+64-1)/64; ++blockNum) {
            res[blockNum] = random_bitset<64>().to_ullong();
            //for (int i = 0; i < 64; ++i) {bitset[i]=(prng.GenerateBit());}
            //res[blockNum]=bitset.to_ullong();
        }
        return res;
    }

    static vector<oc::u64> bitVecXOR(vector<oc::u64>left, const vector<oc::u64>& right){
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
            bits[n] = d( gen);
        }

        return bits;
    }


};



#endif //GARBLINGGATES_UTIL_H
