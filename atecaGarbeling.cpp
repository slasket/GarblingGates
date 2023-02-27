//
// Created by a on 21/02/2023.
//

#include "atecaGarbeling.h"

#include <utility>
#include <boost/dynamic_bitset.hpp>
#include "util/util.h"

string atecaGarbeling::scheme::garble(int secParam, const vector<std::string>& circuit) {
    int externalLength = secParam;
    int internalLengh = 8 * secParam;

    auto encodingInfo = generateLabels(circuit,externalLength);
    auto GarbledFAndD = garbleCircuit(externalLength,circuit,encodingInfo);
    //auto DecodingInfo = decodingInfo(D,externalLength);
    return std::string();
}



vector<tuple<vector<::uint64_t>,vector<::uint64_t>>> atecaGarbeling::scheme::generateLabels(vector<std::string> circuit, int externalLength) {
    auto inputs = util::split(circuit[1], ' ');
    int inputWires = 0;
    for (int i = 1; i < inputs.size(); ++i) {
        inputWires+= stoi(inputs[i]);
    }
    vector<tuple<vector<::uint64_t>,vector<::uint64_t>>> e =vector<tuple<vector<::uint64_t>,vector<::uint64_t>>>(inputWires);
    for (int i = 0; i < inputWires; ++i) {
        vector<::uint64_t> lw0 = util::genBitsNonCrypto(externalLength);
        vector<::uint64_t> lw1 = util::VecXOR(util::genBitsNonCrypto(externalLength), lw0);
        tuple<vector<::uint64_t>,vector<::uint64_t>> ew = {lw0,lw1};
        e[i] = ew;
    }

    return e;
}

vector<string> atecaGarbeling::scheme::garbleCircuit(int externalParam, vector<std::string> circuit,
                                                     vector<tuple<vector<::uint64_t>, vector<::uint64_t>>> inputLabels) {
    //get amount of gates, wires and output bits
    auto gatesAndWires=util::split(circuit[0],' ');
    int outputBits =stoi( util::split(circuit[2],' ')[1]);
    int amountOfWires = stoi(gatesAndWires[1]);

    //set the input wires, as in resize the input labels vector to have room for all wires
    auto wires = std::move(inputLabels);
    wires.resize(amountOfWires);

    //gabled vector
    auto F = vector<tuple<vector<int>,vector<::uint64_t>>>(amountOfWires);
    //decoding vector
    auto D= vector<tuple<vector<::uint64_t>,vector<::uint64_t>>>(outputBits);

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
            //add Delta to F set for a gate + its inputs and outputs


            //make output labels
            //add output labels if outputgates

        }

    }

    return {};
}

vector<vector<uint64_t>>
atecaGarbeling::scheme::gate(const tuple<vector<::uint64_t>, vector<::uint64_t>>& in0, const tuple<vector<::uint64_t>, vector<::uint64_t>>& in1,
                             const string& typ, int gateNo, int externalParam) {
    int internalParam= externalParam*8;
    //the random oracles lol
    //THIS IS THE WRONG WAY OF TWEAKING!=!=!?!??!!
    auto l00 = get<0>(in0);
    l00.insert(l00.end(), get<0>(in1).begin(), get<0>(in1).end());
    l00.push_back(gateNo);
    auto l01 = get<0>(in0);
    l01.insert(l01.end(), get<1>(in1).begin(), get<1>(in1).end());
    l01.push_back(gateNo);
    auto l10 = get<1>(in0);
    l10.insert(l10.end(), get<0>(in1).begin(), get<0>(in1).end());
    l10.push_back(gateNo);
    auto l11 = get<1>(in0);
    l11.insert(l11.end(), get<1>(in1).begin(), get<1>(in1).end());
    l11.push_back(gateNo);
    //actually compute the hashes
    vector<::uint64_t> X_00 = util::hash_variable(util::uintVec2Str(l00), internalParam);
    vector<::uint64_t> X_01 = util::hash_variable(util::uintVec2Str(l01), internalParam);
    vector<::uint64_t> X_10 = util::hash_variable(util::uintVec2Str(l10), internalParam);
    vector<::uint64_t> X_11 = util::hash_variable(util::uintVec2Str(l11), internalParam);

    //and all labels with 0 and uint_max
    //l00a0 is l00 nored with 0 and l00a1 is l00 anded with 1
    auto l00a0 = util::vecNorStatic(X_00, 0);//NOR operation lol
    auto l00a1 = util::vecAndStatic(X_00, UINT64_MAX);
    //l01a0 and l01a1
    auto l01a0 = util::vecNorStatic(X_01, 0);
    auto l01a1 = util::vecAndStatic(X_01, UINT64_MAX);
    //l01a0 and l01a1
    auto l10a0 = util::vecNorStatic(X_10, 0);
    auto l10a1 = util::vecAndStatic(X_10, UINT64_MAX);
    //l11a0 and l11a1
    auto l11a0 = util::vecNorStatic(X_11, 0);
    auto l11a1 = util::vecAndStatic(X_11, UINT64_MAX);


    //now that you have vectors with 1 for each 0 and 1 for each 1
    //compute mask 0000 =  l00a0 ^ l01a0 ^ l10a0 ^ l11a0
    //masks that and, xor share
    auto mask0000 =  util::vecAND(util::vecAND(util::vecAND(l00a0, l01a0), l10a0), l11a0);
    auto mask1111 =  util::vecAND(util::vecAND(util::vecAND(l00a1, l01a1), l10a1), l11a1);
    //and specific masks
    auto mask0001 =  util::vecAND(util::vecAND(util::vecAND(l00a0, l01a0), l10a0), l11a1);
    auto mask1110 =  util::vecAND(util::vecAND(util::vecAND(l00a1, l01a1), l10a1), l11a0);
    //xor specific masks
    auto mask1001 =  util::vecAND(util::vecAND(util::vecAND(l00a1, l01a0), l10a0), l11a1);
    auto mask0110 =  util::vecAND(util::vecAND(util::vecAND(l00a0, l01a1), l10a1), l11a0);

    vector<::uint64_t> masksORed;
    //or with 0000 0001 1110 or 1111
    //if theres a one in this vector one of the masks had an 1 in the ith bit
    if (typ=="AND"){
        masksORed = util::vecOR(util::vecOR(util::vecOR(mask0000,mask1111),mask0001),mask1110);
    //or with 0000 1001 0110 1111
    }else if (typ=="XOR"){
        masksORed =util::vecOR(util::vecOR(util::vecOR(mask0000,mask1111),mask1001),mask0110);
    }
    //make internal param length zer0 string
    auto delta = vector<::uint64_t>((internalParam+64-1)/64);
    auto deltaHW =0;
    int j =0;
    do {
        if (typ == "AND"){
            if (util::ithBitL2R(masksORed,j)){
                //or the ith bit with 1
                delta= util::setIthBitTo1L2R(delta,j);
                deltaHW ++;
            }
        }else if(typ == "XOR"){
            //if true
            if (util::ithBitL2R(masksORed,j)){
                //update j'th bit of delta to 1
                delta= util::setIthBitTo1L2R(delta,j);
                deltaHW ++;
            }
        }
        j++;
    } while (deltaHW < externalParam);

    vector<::uint64_t> L0; vector<::uint64_t>L1;

    if (typ=="AND"){
        //L0 = projection(Label00, delta)
        //L1 = projection(l11,delta)
    }else if (typ=="XOR"){
        //L0 = projection(Label00, delta)
        //L1 = projection(l01,delta)
    }

    return {L0, L1, delta};
}

vector<::uint64_t> projection(vector<::uint64_t> a,vector<::uint64_t> b){
    //projection A o B means take the bit A[i] if B[i]=1
    int l = util::vecHW(b);
    boost::dynamic_bitset<> projection(l);
    int bitsProjected=0; int j;
    while (bitsProjected<l){
        if (util::ithBitL2R(b,j)){
            //copy the bit of a
        }
        //do nothing
        j++;
    }

    return {};
}

