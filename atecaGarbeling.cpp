//
// Created by a on 21/02/2023.
//

#include "atecaGarbeling.h"

#include <utility>
#include "util/util.h"

string atecaGarbeling::scheme::garble(int secParam, const vector<std::string>& circuit) {
    int externalLength = secParam;
    int internalLengh = 8 * secParam;

    auto encodingInfo = generateLabels(circuit,externalLength);
    auto GarbledFAndD = garbleCircuit(externalLength,circuit,encodingInfo);
    //auto DecodingInfo = decodingInfo(D,externalLength);
    return std::string();
}



vector<tuple<vector<oc::u64>,vector<oc::u64>>> atecaGarbeling::scheme::generateLabels(vector<std::string> circuit, int externalLength) {
    auto inputs = util::split(circuit[1], ' ');
    int inputWires = 0;
    for (int i = 1; i < inputs.size(); ++i) {
        inputWires+= stoi(inputs[i]);
    }
    vector<tuple<vector<oc::u64>,vector<oc::u64>>> e =vector<tuple<vector<oc::u64>,vector<oc::u64>>>(inputWires);
    for (int i = 0; i < inputWires; ++i) {
        vector<oc::u64> lw0 = util::genBitsNonCrypto(externalLength);
        vector<oc::u64> lw1 = util::bitVecXOR(util::genBitsNonCrypto(externalLength), lw0);
        tuple<vector<oc::u64>,vector<oc::u64>> ew = {lw0,lw1};
        e[i] = ew;
    }

    return e;
}

vector<string> atecaGarbeling::scheme::garbleCircuit(int externalParam, vector<std::string> circuit,
                                                     vector<tuple<vector<oc::u64>, vector<oc::u64>>> inputLabels) {
    //internal param is 8x external param

    //random oracle instantiation
    //lol i didnt do :)
    //get amount of gates and wires
    auto gatesAndWires=util::split(circuit[0],' ');
    //get amount of output bits
    int outputBits =stoi( util::split(circuit[2],' ')[1]);

    int amountOfWires = stoi(gatesAndWires[1]);

    //set inputwires in W
    auto wires = inputLabels;
    wires.resize(amountOfWires);

    //gabled vector
    auto F = vector<tuple<vector<int>,vector<oc::u64>>>(amountOfWires);
    //decoding vector
    auto D= vector<tuple<vector<oc::u64>,vector<oc::u64>>>(outputBits);

    //output bits are defined as the last k wires, where k is the amount of output bits
    int firstOutputBit = amountOfWires - outputBits;

    //for every gate in circuit
    for (int i = 3; i < circuit.size(); ++i) {
        //find input labels
        auto gateInfo = util::split(circuit[i],' ');
        int inAmount = stoi(gateInfo[0]); int outAmount = stoi(gateInfo[1]);
        //this always happen xd
        if(inAmount == 2 & outAmount == 1){
            int gateNo = (i-3);
            int in0 = stoi(gateInfo[2]);
            int in1 = stoi(gateInfo[3]);
            int out = stoi(gateInfo[4]);
            string type = gateInfo[5];

            //calculate garble
            auto garbledGate = gate(wires[in0],wires[in1], type, gateNo, externalParam);


            //make output labels

            //add Delta to F set for a gate + its inputs and outputs
            //add output labels if outputgates
        }

    }


    return vector<string>();
}

tuple<int, int, int>
atecaGarbeling::scheme::gate(tuple<vector<oc::u64>, vector<oc::u64>> in0, tuple<vector<oc::u64>, vector<oc::u64>> in1,
                             string type, int gateNo, int externalParam) {
    int internalParam= externalParam*8;
    //the random oracles lol

    //make internal param length zer0 string
    auto Delta = vector<oc::u64>((internalParam+64-1)/64);



    return tuple<int, int, int>();
}

