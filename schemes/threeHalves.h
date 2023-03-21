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

    constexpr static uint8_t V[4][2][5] =
            {
                    {
                        {1,0,0,0,0},
                        {0,1,0,0,0}},
                    {
                        {1,0,0,0,1},
                        {0,1,0,1,1}},
                    {
                        {1,0,1,0,1},
                        {0,1,0,0,1}},
                    {
                        {1,0,1,0,0},
                        {0,1,0,1,0}}
            };

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
    static tuple<Ftype, tuple<halfDelta, vector<tuple<halfLabels, int>>>, vector<halfLabels>, halfLabels>  garble(int k, vector<string> f);
    static vector<halfLabels> encode(tuple<halfDelta, vector<tuple<halfLabels, int>>> e, vector<int> x);
    static vector<halfLabels> eval(Ftype F, vector<halfLabels> X, vector<string> f, int k, const halfLabels& invConst);
    static vector<int> decode(vector<halfLabels> d, vector<halfLabels> Y, vector<string> f, int k);
    static vint sampleR(int permuteBitA, int permuteBitB);
    static vint hashPrime(const vint& input, int k, int tweak);
private:
    static vector<halfLabels>
    calcZij(halfLabels &A0, halfLabels &B0, halfLabels &A1, halfLabels &B1, vint &rVec, int permuteBitA,
            int permuteBitB, halfLabels &delta);

    static halfLabels decodeR(vector<uint64_t> rVec, halfLabels A, halfLabels B, int Aperm, int Bperm);
};


#endif //GARBLINGGATES_THREEHALVES_H
