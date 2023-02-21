//
// Created by a on 21/02/2023.
//

#include "atecaGarbeling.h"
#include "util/util.h"

string atecaGarbeling::scheme::garble(int secParam, vector<std::string> circuit, vector<::uint64_t> input) {
    int externalLength = secParam;
    int internalLengh = 8 * secParam;

    string encodingInfo = init(circuit,externalLength);
    return std::string();
}

string atecaGarbeling::scheme::init(vector<std::string> circuit, int externalLength) {
    auto inputs = util::split(circuit[1], ' ');
    int inputWires = 0;
    for (int i = 1; i < inputs.size(); ++i) {
        inputWires+= stoi(inputs[i]);
    }
    vector<tuple<vector<oc::u64>>> e =vector<tuple<vector<oc::u64>>>(inputWires);
    for (int i = 0; i < inputWires; ++i) {
        vector<oc::u64> lw0 = util::genBitsNonCrypto(externalLength);
        vector<oc::u64> lw1 = util::bitVecXOR(util::genBitsNonCrypto(externalLength), lw0);
        tuple<vector<oc::u64>,vector<oc::u64>> ew = {lw0,lw1};

        e[i] = ew;
    }


    return std::string();
}

