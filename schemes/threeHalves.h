//
// Created by svend on 001, 01-03-2023.
//

#ifndef GARBLINGGATES_THREEHALVES_H
#define GARBLINGGATES_THREEHALVES_H

#include <tuple>
#include "../util/util.h"
using namespace std;
using namespace customTypeSpace;
typedef vector<tuple<vint, vint, vint, vector<uint8_t>>> Ftype;
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

    constexpr static uint8_t VInv[5][8] =
            {
                    {1,0,0,0,0,0,0,0},
                    {0,1,0,0,0,0,0,0},
                    {1,1,0,0,1,1,0,0},
                    {1,1,1,1,0,0,0,0},
                    {0,0,0,0,1,0,1,0}
            };
    constexpr static uint8_t VInvRp[5][6] =
            {
                    {0,0,1,0,0,0},
                    {0,1,0,0,0,0},
                    {0,0,1,0,0,1},
                    {0,1,0,0,1,0},
                    {0,0,0,0,0,0}
            };
    constexpr static uint8_t VInvM[5][6] =
            {
                    {1,0,0,0,1,0},
                    {0,0,1,0,1,0},
                    {1,1,0,0,0,0},
                    {0,0,1,1,0,0},
                    {0,0,0,0,1,1}
            };
    static tuple<Ftype, tuple<halfDelta, vector<tuple<halfLabels, int>>>, vector<vint>>  garble(int k, vector<string> f);
    static int encode(int e, int x);
    static int eval(Ftype F, int X);
    static int decode(int d, int Y);
    static vint sampleR(int permuteBitA, int permuteBitB);
    static vector<int> computeT(int permuteBitA, int permuteBitB, const string& gateType);
    static vint hashPrime(const vint& input, int k, int tweak);
private:
    static vector<halfLabels> calcZij(halfLabels &A0, halfLabels &B0, halfLabels &A1, halfLabels &B1, vint &rVec, int permuteBitA, int permuteBitB, halfLabels &delta, int i);

};


#endif //GARBLINGGATES_THREEHALVES_H
