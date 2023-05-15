//
// Created by svend on 020, 20-02-2023.
//

#ifndef GARBLINGGATES_BASEGARBLE_H
#define GARBLINGGATES_BASEGARBLE_H

#include <vector>
#include <string>
#include "../util/util.h"
#include "../util/circuitParser.h"

#include "../util/hashRTCCR.h"
using namespace customTypeSpace;
using namespace std;

class baseGarble {
public:
    static
    tuple<tuple<vint, vector<labelPair>, hashRTCCR>, vector<labelPair>, vector<labelPair>>
    garble(circuit f, int k, util::hashtype hashtype);
    static vector<vint> encode(vector<labelPair> e, vector<int> x);
    static vector<vint> eval(tuple<vint, vector<labelPair>, hashRTCCR> F,
                             vector<vint> X, circuit f, int k);
    static vector<int> decodeBits(vector<labelPair> d, vector<vint> Y, int k, hashRTCCR hash);
    static vint hashFunc(vint x, int k);

    static void
    andGate(const vector<::uint64_t> &globalDelta, int permuteBitA, int permuteBitB, vint &A0,
            vint &A1,
            vint &B0, vint &B1, vector<::uint64_t> &ciphertext, vector<::uint64_t> &gate0,
            vector<::uint64_t> &gate1, int k, hashRTCCR hash);
    static void
    xorGate(const vector<::uint64_t> &globalDelta, int permuteBitA, int permuteBitB,
            vint &A0, vint &B0, vector<::uint64_t> &ciphertext);
    static void
    invGate(int permuteBitA, vint &A0, vint &A1, vint &ciphertext, const vint &invConst);

    static vint hashXOR(vint &labelA, vint &labelB, int k);


    static tuple<vector<::uint64_t>, vector<::uint64_t>>
    garbleGate(const vint &invConst, int k, const vector<::uint64_t> &globalDelta, vector<int> inputWires,
               const string& gateType, vector<labelPair> &wireLabels,
               vector<int> &outputWires, hashRTCCR hash);

    static vint evalGate(const vint &invConst, int k,
                         const vector<labelPair> &garbledCircuit,
                         const vector<vint> &wireValues, int i, vector<int> inputWires,
                         const string& gateType,
                         hashRTCCR hash);

    static vint decode(vector<labelPair> d, vector<vint> Y, int k, hashRTCCR hash);

    static vint hashXORfast(vint labelA, vint labelB, int k, hashRTCCR &fh);
};


#endif //GARBLINGGATES_BASEGARBLE_H
