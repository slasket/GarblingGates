//
// Created by svend on 020, 20-02-2023.
//

#include "baseGarble.h"
#include "../util/util.h"


//k is security parameter and f is the function to be garbled with 3 lines of metadata
int baseGarble::garble(int k, vector<string> f) {
    string &metadata1 = f[0];
    string &metadata2 = f[1];
    string &metadata3 = f[2];
    vector<::uint64_t> globalDelta = vector<::uint64_t>(k);
    //perform gate by gate garbling
    for (int i = 3; i < f.size(); ++i) {
        ////////////////////////////// Getting out gate from string ////////////////////
        string &line = f[i];
        auto gateInfo = extractGate(line);
        auto inputWires = get<0>(gateInfo);
        auto outputWires = get<1>(gateInfo);
        auto gateType = get<2>(gateInfo);
        ////////////////////////////// Garbling gate ///////////////////////////////////
        //garble gate
        //TODO needs delta/label generation
        //vector<tuple<int, int>> inputWiresLabels = vector<tuple<int, int>>(inputWires.size());
        //vector<tuple<int, int>> outputWiresLabels = vector<tuple<int, int>>(outputWires.size());
        vector<tuple<vector<uint64_t>, vector<uint64_t>>> inputWiresLabels = vector<tuple<vector<uint64_t>, vector<uint64_t>>>(inputWires.size());
        vector<tuple<vector<uint64_t>, vector<uint64_t>>> outputWiresLabels = vector<tuple<vector<uint64_t>, vector<uint64_t>>>(outputWires.size());
        util::generateRandomLabels(k, globalDelta, inputWiresLabels, outputWiresLabels);
        //TEMP generating random labels for input wires (todo: with length k)
        //for (int j = 0; j < inputWires.size(); ++j) {
        //    inputWiresLabels[j] = make_tuple(rand(), rand());
        //}
        //for (int j = 0; j < outputWires.size(); ++j) {
        //    outputWiresLabels[j] = make_tuple(rand(), rand());
        //}
        //TODO: permute gate truth table

        hash<size_t> hashFunction; //TODO: change out hash function
        //auto A0 = (size_t) (get<0>(inputWiresLabels[0]));
        //auto B0 = (size_t) (get<1>(inputWiresLabels[0]));
        //auto A1 = (size_t) (get<0>(inputWiresLabels[1]));
        //auto B1 = (size_t) (get<1>(inputWiresLabels[1]));
        //auto delta = (size_t) (rand());
        //size_t cipherText = hashFunction(A0) ^ hashFunction(B0);
        //size_t gate1 = hashFunction(A0) ^ hashFunction(B1) ^ cipherText ^ A0;
        //size_t gate0 = hashFunction(A1) ^ hashFunction(B0) ^ cipherText ^ B0;


    }
    return 0;
}

tuple<vector<int>, vector<int>, string> baseGarble::extractGate(const string &line) {
    vector<int> inputWires;
    vector<int> outputWires;
    //split line into space separated values
    vector<string> lineSplit = util::split(line, ' ');
    int numInputWires = stoi(lineSplit[0]);
    int numOutputWires = stoi(lineSplit[1]);
    //handle input wires
    for (int j = 2; j < numInputWires + 2; ++j) { //index names start at 2
        //get next input wire label/index
        inputWires.push_back(stoi(lineSplit[j]));
    }
    //handle output wires
    for (int j = numInputWires + 2; j < numInputWires + numOutputWires + 2; ++j) {
        //get next output wire label/index
        outputWires.push_back(stoi(lineSplit[j]));
    }
    //handle gate type
    string gateType = lineSplit[numInputWires + numOutputWires + 2];
    return make_tuple(inputWires, outputWires, gateType);
}

int baseGarble::encode(int e, int x) {
    return 0;
}

int baseGarble::eval(int F, int X) {
    return 0;
}

int baseGarble::decode(int d, int Y) {
    return 0;
}