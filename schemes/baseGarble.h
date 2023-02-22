//
// Created by svend on 020, 20-02-2023.
//

#ifndef GARBLINGGATES_BASEGARBLE_H
#define GARBLINGGATES_BASEGARBLE_H

#include <vector>
#include <string>

using namespace std;
class baseGarble {
public:
    static int garble(int k, vector<string> f);
    static int encode(int e, int x);
    static int eval(int F, int X);
    static int decode(int d, int Y);
    static tuple<vector<int>, vector<int>, string> extractGate(const string &line);
    static vector<uint64_t> qouteUnqouteHashFunction(vector<uint64_t> x);

    static tuple<vector<::uint64_t>, vector<::uint64_t>, vector<::uint64_t>>
    andGate(const vector<::uint64_t> &globalDelta, int permuteBitA, int permuteBitB, vector<uint64_t> &A0,
            vector<uint64_t> &A1,
            vector<uint64_t> &B0, vector<uint64_t> &B1, vector<::uint64_t> &ciphertext, vector<::uint64_t> &gate0,
            vector<::uint64_t> &gate1);

    static vector<uint64_t> XORHashpart(vector<uint64_t> &labelA, vector<uint64_t> &labelB);
};


#endif //GARBLINGGATES_BASEGARBLE_H