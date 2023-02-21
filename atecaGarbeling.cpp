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
    vector<vector<string>> e =vector<vector<string>>(inputWires);
    for (int i = 0; i < inputWires; ++i) {
        auto lw0 = util::random_bitset(externalLength);

    }


    return std::string();
}

