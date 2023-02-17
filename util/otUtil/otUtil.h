//
// Created by a on 15/02/2023.
//

#ifndef GARBLINGGATES_OTUTIL_H
#define GARBLINGGATES_OTUTIL_H

#include <string>
#include <bitset>
#include <random>

#include <cryptoTools/Common/Defines.h>
using namespace std;

class otUtil {

public:
    static int findithBit(vector<uint64_t> ui, int i);
    static vector<oc::u64> genBitsNonCrypto(int bits);

    //taken from https://www.appsloveworld.com/cplus/100/112/c-efficient-way-to-generate-random-bitset-with-configurable-mean-1s-to-0s-r
    template< size_t size>
    static typename std::bitset<size> random_bitset( double p = 0.5) {

        typename std::bitset<size> bits;
        std::random_device rd;
        std::mt19937 gen( rd());
        std::bernoulli_distribution d( p);

        for( int n = 0; n < size; ++n) {
            bits[ n] = d( gen);
        }

        return bits;
    }
    static string printBitsetofVectorofUints(vector<uint64_t> uints);
    static string AESHash(vector<uint64_t> input);
    static string SHA256Hash(vector<uint64_t> input);
};



#endif //GARBLINGGATES_OTUTIL_H
