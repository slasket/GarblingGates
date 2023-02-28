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
    static
    tuple<vector<tuple<vector<int>, vector<uint64_t>, vector<uint64_t>>>, vector<tuple<vector<uint64_t >, vector<uint64_t >>>, vector<tuple<vector<uint64_t >, vector<uint64_t >>>>
    garble(int k, vector<string> f);
    static vector<vector<uint64_t>> encode(vector<tuple<vector<uint64_t>, vector<uint64_t>>> e, vector<int> x);
    static vector<vector<uint64_t>> eval(
            tuple<vector<tuple<vector<int>, vector<uint64_t>, vector<uint64_t>>>,
                    vector<tuple<vector<uint64_t>, vector<uint64_t>>>,
                            vector<tuple<vector<uint64_t>, vector<uint64_t>>>> F,
                            vector<vector<uint64_t>> X , vector<string> f);
    static vector<int> decode(vector<tuple<vector<uint64_t>, vector<uint64_t>>> d, vector<vector<uint64_t>> Y);
    static tuple<vector<int>, vector<int>, string> extractGate(const string &line);
    static vector<uint64_t> HashFunction(vector<uint64_t> x, int k);

    static tuple<vector<::uint64_t>, vector<::uint64_t>, vector<::uint64_t>>
    andGate(const vector<::uint64_t> &globalDelta, int permuteBitA, int permuteBitB, vector<uint64_t> &A0,
            vector<uint64_t> &A1,
            vector<uint64_t> &B0, vector<uint64_t> &B1, vector<::uint64_t> &ciphertext, vector<::uint64_t> &gate0,
            vector<::uint64_t> &gate1, int k);
    static tuple<vector<::uint64_t>, vector<::uint64_t>, vector<::uint64_t>>
    xorGate(const vector<::uint64_t> &globalDelta, int permuteBitA, int permuteBitB, vector<uint64_t> &A0,
            vector<uint64_t> &A1,
            vector<uint64_t> &B0, vector<uint64_t> &B1, vector<::uint64_t> &ciphertext, vector<::uint64_t> &gate0,
            vector<::uint64_t> &gate1, int k);

    static vector<uint64_t> XORHashpart(vector<uint64_t> &labelA, vector<uint64_t> &labelB, int k);

    static void getBits(string &f, int &numberOfInputBits);
};


#endif //GARBLINGGATES_BASEGARBLE_H
