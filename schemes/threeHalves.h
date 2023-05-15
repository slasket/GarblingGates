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
#include "../util/circuitParser.h"
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
    garble(circuit f, int k, util::hashtype);
    static vector<halfLabels> encode(tuple<halfDelta, vector<tuple<halfLabels, int>>> e, vector<int> x);
    static vector<halfLabels> eval(Ftype F, vector<halfLabels> X, circuit f, int k, const halfLabels& invConst, hashRTCCR &hash, util::hashtype);
    static vector<int> decodeBits(vector<halfLabels> d, vector<halfLabels> Y, circuit f, int k);
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

    static vector<uint64_t> decode(vector<halfLabels> d, vector<halfLabels> Y, circuit f, int k);

private:
    static vector<halfLabels>
    calcZij(halfLabels &A0, halfLabels &B0, halfLabels &A1, halfLabels &B1, vint &rVec, int permuteBitA,
            int permuteBitB, halfLabels &delta);

    static halfLabels decodeR(vector<uint64_t> rVec, halfLabels A, halfLabels B, int Aperm, int Bperm);

    static halfLabels zeroes(uint64_t size);

    static void initInputLabels(int k, int numberOfInputBits, vector<tuple<halfLabels, int>> &labelAndPermuteBitPairs,
                                vector<tuple<halfLabels, int>> &inputLabelAndPermuteBitPairs);

    static void gateXOR(const vector<int> &inputWires, const vector<int> &outputWires,
                        vector<tuple<halfLabels, int>> &labelAndPermuteBitPairs,
                        tuple<halfLabels, int> &A0AndPermuteBit,
                        tuple<halfLabels, int> &B0AndPermuteBit);

    static void gateINV(halfLabels &invConst, const vector<int> &outputWires, const vector<int> &inputWires,
                        vector<tuple<halfLabels, int>> &labelAndPermuteBitPairs,
                        tuple<halfLabels, int> &A0AndPermuteBit);

    static void
    gateAND(int k, const util::hashtype &h, hashRTCCR &hashRTCCR, const vector<int> &inputWires,
            const vector<int> &outputWires,
            halfDelta &delta, vector<tuple<halfLabels, int>> &labelAndPermuteBitPairs, Ftype &F,
            tuple<halfLabels, int> &A0AndPermuteBit, tuple<halfLabels, int> &B0AndPermuteBit);


    static vector<vint> calcVZ(const vector<halfLabels> &Zij);

    static vector<vint>
    calcVRP(const vint &A0Left, const vint &A0Right, const vint &B0Left, const vint &deltaLeft,
            const vint &deltaRight);

    static vector<vint> &hashRO(int k, vector<vint> &hashes, const vector<halfLabels> &inputs);

    static vector<vint> &
    hashFast(int k, hashRTCCR &hashRTCCR, halfLabels &A0, halfLabels &A1, halfLabels &B0, halfLabels &B1,
             const vint &A0Left, vector<vint> &hashes, halfLabels &A0xorB0, halfLabels &A0xorB0xorDelta,
             vector<halfLabels> &inputs);

    static void sliceHashes(vint &HA0, vint &HA1, vint &HB0, vint &HB1, vint &HA0xorB0, vint &HA0xorB0xorDelta);

    static vector<vint>
    calcHprime(vint &HA0, vint &HA1, vint &HB0, vint &HB1, const vint &HA0xorB0, vint &HA0xorB0xorDelta);

    static void
    calcZCG(const vector<vint> &VInvZ, const vector<vint> &VInvRpABDelta, const vector<vint> &HVecPrime,
            vector<vint> &CG,
            vector<uint8_t> &zVec);

    static vector<halfLabels> &calcRZ(vint &rVec, vector<halfLabels> &Zij);
};


#endif //GARBLINGGATES_THREEHALVES_H
