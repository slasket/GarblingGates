//
// Created by svend on 001, 01-03-2023.
//

#ifndef GARBLINGGATES_THREEHALVES_H
#define GARBLINGGATES_THREEHALVES_H

#include <tuple>
#include "../util/util.h"
using namespace std;
using namespace customTypeSpace;
class threeHalves {
public:
    /*
     * [1 1 1 0
     *  1 0 0 1] = 151 reversed
     *
     */
    const static uint8_t S1 = 151;
    /*
     * [1 0 0 1
     *  0 1 1 1] = 233 reversed;
     */
    const static uint8_t S2 = 233;
    static tuple<int, tuple<halfDelta, vector<tuple<halfLabels, int>>>, vector<vint>>  garble(int k, vector<string> f);
    static int encode(int e, int x);
    static int eval(int F, int X);
    static int decode(int d, int Y);
    static vint sampleR(int permuteBitA, int permuteBitB);
    static vint hashPrime(const vint& input, int k, int tweak);


};


#endif //GARBLINGGATES_THREEHALVES_H
