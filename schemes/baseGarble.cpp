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
        vector<tuple<vector<uint64_t>, vector<uint64_t>>> inputWiresLabels = vector<tuple<vector<uint64_t>, vector<uint64_t>>>(inputWires.size());
        vector<tuple<vector<uint64_t>, vector<uint64_t>>> outputWiresLabels = vector<tuple<vector<uint64_t>, vector<uint64_t>>>(outputWires.size());
        auto deltaAndLabels = util::generateRandomLabels(k, globalDelta, inputWiresLabels);

        //TODO: permute gate truth table

        //TODO: change out hash function
        //calculate permute bits
        int permuteBitA = (get<0>(inputWiresLabels[0])[0]) & 1;
        int permuteBitB = (get<0>(inputWiresLabels[1])[0]) & 1;
        vector<uint64_t> A0;
        vector<uint64_t> A1;
        vector<uint64_t> B0;
        vector<uint64_t> B1;
        if(permuteBitA == 1){
            A0 = (get<1>(inputWiresLabels[0]));
            A1 = (get<0>(inputWiresLabels[0]));
        } else {
            A0 = (get<0>(inputWiresLabels[0]));
            A1 = (get<1>(inputWiresLabels[0]));
        }
        if(permuteBitB == 1){
            B0 = (get<1>(inputWiresLabels[1]));
            B1 = (get<0>(inputWiresLabels[1]));
        } else {
            B0 = (get<0>(inputWiresLabels[1]));
            B1 = (get<1>(inputWiresLabels[1]));
        }
        vector<::uint64_t> ciphertext;
        vector<::uint64_t> gate0;
        vector<::uint64_t> gate1;
        foo(globalDelta, permuteBitA, permuteBitB, A0, A1, B0, B1, ciphertext, gate0, gate1);
    }
    return 0;
}

tuple<vector<::uint64_t>, vector<uint64_t>, vector<uint64_t>>
baseGarble::foo(const vector<::uint64_t> &globalDelta, int permuteBitA, int permuteBitB, vector<uint64_t> &A0,
                vector<uint64_t> &A1, vector<uint64_t> &B0, vector<uint64_t> &B1, vector<::uint64_t> &ciphertext,
                vector<::uint64_t> &gate0, vector<::uint64_t> &gate1) {
    if(permuteBitA == 1 & permuteBitB == 1){
        ciphertext =
                util::bitVecXOR(
                        util::bitVecXOR(qouteUnqouteHashFunction(A0),qouteUnqouteHashFunction(B0)),
                        globalDelta);
        gate1 = util::bitVecXOR(
                util::bitVecXOR(
                        util::bitVecXOR(qouteUnqouteHashFunction(A0),qouteUnqouteHashFunction(B1)),
                        ciphertext),A0);
        gate0 = util::bitVecXOR(
                        util::bitVecXOR(qouteUnqouteHashFunction(A1),qouteUnqouteHashFunction(B0)),
                        ciphertext);

    }
    else if(permuteBitA == 1 & permuteBitB == 0){
        ciphertext = util::bitVecXOR(qouteUnqouteHashFunction(A0),qouteUnqouteHashFunction(B0));
        gate1 = util::bitVecXOR(util::bitVecXOR(
                util::bitVecXOR(
                        util::bitVecXOR(qouteUnqouteHashFunction(A0),qouteUnqouteHashFunction(B1)),
                        ciphertext),A0), globalDelta);
        gate0 = util::bitVecXOR(
                util::bitVecXOR(qouteUnqouteHashFunction(A1),qouteUnqouteHashFunction(B0)),
                ciphertext);
    }
    else if(permuteBitA == 0 & permuteBitB == 1){
        ciphertext = util::bitVecXOR(qouteUnqouteHashFunction(A0),qouteUnqouteHashFunction(B0));
        gate1 = util::bitVecXOR(
                util::bitVecXOR(
                        util::bitVecXOR(qouteUnqouteHashFunction(A0),qouteUnqouteHashFunction(B1)),
                        ciphertext),A0);
        gate0 = util::bitVecXOR(util::bitVecXOR(
                util::bitVecXOR(qouteUnqouteHashFunction(A1),qouteUnqouteHashFunction(B0)),
                ciphertext), globalDelta);
    }
    else {
        ciphertext = util::bitVecXOR(qouteUnqouteHashFunction(A0),qouteUnqouteHashFunction(B0));
        gate1 = util::bitVecXOR(
                util::bitVecXOR(
                        util::bitVecXOR(qouteUnqouteHashFunction(A0),qouteUnqouteHashFunction(B1)),
                        ciphertext),A0);
        gate0 = util::bitVecXOR(
                util::bitVecXOR(qouteUnqouteHashFunction(A1),qouteUnqouteHashFunction(B0)),
                ciphertext);
    }
    return make_tuple(ciphertext, gate0, gate1);
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

vector<uint64_t> baseGarble::qouteUnqouteHashFunction(vector<uint64_t> x) {
    return x;
}
