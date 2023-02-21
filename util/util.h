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
            vector<tuple<vector<uint64_t>, vector<uint64_t>>>,
            vector<tuple<vector<uint64_t>, vector<uint64_t>>>>
            generateRandomLabels(int k, vector<uint64_t> globalDelta,
                                     vector<tuple<vector<uint64_t>, vector<uint64_t>>> inputWiresLabels,
                                     vector<tuple<vector<uint64_t>, vector<uint64_t>>> outputWiresLabels) {
                //generate new global delta if non is given
                if(globalDelta[0]==0){
                    globalDelta = otUtil::genBitsNonCrypto(k);
                    globalDelta[0] = globalDelta[0] | 281474976710655; //1111 1111 1111 1111 1111 1111 1111 1111 1111 1111 1111 1111
                }
                //generate input labels
                for(auto & inputWiresLabel : inputWiresLabels){
                    labelPointAndPermute(k, inputWiresLabel, globalDelta);
                }
                //generate output labels
                for(auto & outputWiresLabel : outputWiresLabels){
                    labelPointAndPermute(k, outputWiresLabel, globalDelta);
                }
                return make_tuple(globalDelta, inputWiresLabels, outputWiresLabels);
    }

    static void labelPointAndPermute(int k,
                                     tuple<vector<uint64_t>, vector<uint64_t>> &wiresLabel,
                                        const vector<uint64_t>& globalDelta,
                                     int permuteBit = 2) {//generate random permute bit
        if (permuteBit >= 2) permuteBit = rand() % 2;
        vector<uint64_t> a0 = otUtil::genBitsNonCrypto(k);
        //add permute bit
        a0[0] = a0[0] & 281474976710654; //1111 1111 1111 1111 1111 1111 1111 1111 1111 1111 1111 1110
        a0[0] = a0[0] | 281474976710654 + permuteBit; //can be hardcoded as it is one block
        auto a1 = vector<uint64_t>(a0.size());
        for (int i = 0; i < a0.size(); ++i) {
            a1[i] = a0[i] ^ globalDelta[i];
        }
        wiresLabel = make_pair(a0, a1);
    }
};


#endif //GARBLINGGATES_UTIL_H
