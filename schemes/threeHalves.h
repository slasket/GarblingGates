//
// Created by svend on 001, 01-03-2023.
//

#ifndef GARBLINGGATES_THREEHALVES_H
#define GARBLINGGATES_THREEHALVES_H


#include <openssl/evp.h>
#include <openssl/aes.h>
#include <openssl/sha.h>
#include <random>
#include <bitset>
#include <iostream>
#include <iterator>
#include <vector>
#include <sstream>
#include <string>
#include <tuple>
#include "../util/util.h"
#include "../util/hashRTCCR.h"
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
    static tuple<Ftype, tuple<halfDelta, vector<tuple<halfLabels, int>>>, vector<halfLabels>, halfLabels, hashRTCCR>
    garble(vector<string> f, int k=128, int h = 1);
    static vector<halfLabels> encode(tuple<halfDelta, vector<tuple<halfLabels, int>>> e, vector<int> x);
    static vector<halfLabels> eval(Ftype F, vector<halfLabels> X, vector<string> f, int k, const halfLabels& invConst, hashRTCCR &hash, int h = 1);
    static vector<int> decodeBits(vector<halfLabels> d, vector<halfLabels> Y, vector<string> f, int k);
    static vint sampleR(int permuteBitA, int permuteBitB);
    static vector<int> computeT(int permuteBitA, int permuteBitB, const string& gateType);
    static vint hashPrime(const vint& input, int k, int tweak);

    static halfDelta genDeltaHalves(int k) {
        vector<uint64_t> leftDeltaHalf = util::genBitsNonCrypto(k/2);
        leftDeltaHalf[0] = leftDeltaHalf[0] | 1;
        vector<uint64_t> rightDeltaHalf = util::genBitsNonCrypto(k/2);
        return {leftDeltaHalf, rightDeltaHalf};
    }

    static halfDelta genLabelHalves(int k) {
        vector<uint64_t> leftLabelHalf = util::genBitsNonCrypto(k/2);
        leftLabelHalf[0] = leftLabelHalf[0] & (UINT64_MAX << 1);
        vector<uint64_t> rightLabelHalf = util::genBitsNonCrypto(k/2);

        return {leftLabelHalf, rightLabelHalf};
    }

    static vector<uint64_t> decode(vector<halfLabels> d, vector<halfLabels> Y, vector<string> f, int k);

private:
    static vector<halfLabels>
    calcZij(halfLabels &A0, halfLabels &B0, halfLabels &A1, halfLabels &B1, vint &rVec, int permuteBitA,
            int permuteBitB, halfLabels &delta);

    static halfLabels decodeR(vector<uint64_t> rVec, halfLabels A, halfLabels B, int Aperm, int Bperm);

    static halfLabels zeroes(uint64_t size);

};


#endif //GARBLINGGATES_THREEHALVES_H
