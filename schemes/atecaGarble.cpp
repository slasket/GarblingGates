//
// Created by a on 21/02/2023.
//

#include "atecaGarble.h"

#include <utility>


tuple<vector<vint>,vector<tuple<vint,vint>>,vector<vint>,int,tuple<vint,vint>>
    atecaGarble::Gb(int l, const vector<std::string>& C) {

    auto encodingInfo = Init(C, l);
    auto invVar= genInvVar(l);
    auto garbledFnDnInvVar = GarbleCircuit(l, C, encodingInfo, invVar);
    auto decoding = DecodingInfo(get<1>(garbledFnDnInvVar), l, invVar);

    return {get<0>(garbledFnDnInvVar), encodingInfo, decoding, l, invVar};
}



vector<tuple<vint,vint>> atecaGarble::Init(vector<std::string> C, int l) {
    auto inputs = util::split(C[1], ' ');
    int inputWires = 0;
    for (int i = 1; i < inputs.size(); ++i) {
        inputWires+= stoi(inputs[i]);
    }
    vector<tuple<vint,vint>> e =vector<tuple<vint,vint>>(inputWires);
    for (int i = 0; i < inputWires; ++i) {
        vint lw0 = util::genBitsNonCrypto(l);
        vint lw1 = util::vecXOR(util::genBitsNonCrypto(l), lw0);
        tuple<vint,vint> ew = {lw0,lw1};
        e[i] = ew;
    }

    return e;
}

tuple<vector<vint>,vector<tuple<vint,vint>>, tuple<vint,vint>>
        atecaGarble::GarbleCircuit(int l, vector<std::string> C,vector<tuple<vint, vint>> encoding, const tuple<vint,vint>& invVar) {
    //get amount of gates, wires and output bits
    auto gatesAndWires=util::split(C[0], ' ');
    int outputBits =stoi( util::split(C[2], ' ')[1]);
    int amountOfWires = stoi(gatesAndWires[1]);

    //set the input wires, as in resize the input labels vector to have room for all wires
    auto wires = std::move(encoding);
    wires.resize(amountOfWires);

    //gabled vector
    auto F = vector<vint>(amountOfWires);//too large remove input wires
    //decoding vector
    auto D = vector<tuple<vint,vint>>(outputBits);

    //output bits are defined as the last k wires, where k is the amount of output bits
    int firstOutputBit = amountOfWires - outputBits;
    vector<vint> garbledGate;
    int out;
    //for every Gate in circuit
    for (int i = 3; i < C.size(); ++i) {
        //find input labels
        auto gateInfo = util::split(C[i], ' ');
        //int inAmount = stoi(gateInfo[0]);
        //int outAmount = stoi(gateInfo[1]);

        int gateNo = (i - 3);
        //inverse gate hack
        if (gateInfo[4] == "INV") {
            int in0 = stoi(gateInfo[2]);
            //int in1 = stoi(gateInfo[3]);
            out = stoi(gateInfo[3]);
            string type = gateInfo[4];

            //calculate Gb
            garbledGate = Gate(wires[in0], invVar, type, gateNo, l);

        }//normal gate
        else {
            int in0 = stoi(gateInfo[2]);
            int in1 = stoi(gateInfo[3]);
            out = stoi(gateInfo[4]);
            string type = gateInfo[5];

            //calculate Gb
            garbledGate = Gate(wires[in0], wires[in1], type, gateNo, l);
        }
        //add Delta to F set for a Gate
        F[gateNo] = garbledGate[2];
        //the gates output labels
        wires[out] = {garbledGate[0], garbledGate[1]};
        //if g is an output Gate, add it to D
        if (out >= firstOutputBit) {
            //add the label to D
            D[out-firstOutputBit] = {garbledGate[0],garbledGate[1]};
        }
    }
    return {F,D, invVar};
}


vector<vint> atecaGarble::Gate(const tuple<vint, vint>& in0, const tuple<vint, vint>& in1,
                          const string& typ, int gateNo, int l) {
    int internalParam= l * 8;
    //the random oracles lol
    //THIS IS THE WRONG WAY OF TWEAKING!=!=!?!??!!
    vint l00 = get<0>(in0);
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
    vint X_00 = util::hash_variable(util::uintVec2Str(l00), internalParam);
    vint X_01 = util::hash_variable(util::uintVec2Str(l01), internalParam);
    vint X_10 = util::hash_variable(util::uintVec2Str(l10), internalParam);
    vint X_11 = util::hash_variable(util::uintVec2Str(l11), internalParam);
    auto delta = vint((internalParam+64-1)/64);
    auto deltaHW =0;

    //vint mask = masksForSlices(X_00,X_01,X_10,X_11,typ);

    int j =0;
    do {
        string slice = util::sliceVecL2R(X_00,X_01,X_10,X_11,j);
        if (typ == "AND"){
            if (slice =="0000"|| slice =="0001"||slice=="1110"||slice=="1111"){//if (util::ithBitL2R(mask,j)){
                //or the ith bit with 1
                delta= util::setIthBitTo1L2R(delta,j);
                deltaHW ++;
            }
        }else if(typ == "XOR"|| typ=="INV"){
            if (slice =="0000"|| slice =="1001"||slice=="0110"||slice=="1111"){//if (util::ithBitL2R(mask,j)){
                //update j'th bit of delta to 1
                delta= util::setIthBitTo1L2R(delta,j);
                deltaHW ++;
            }
        }else{
            string a = "Gate not implemented: ";
            a.append(typ);
            throw invalid_argument(a);
        }
        j++;
    } while (deltaHW < l);

    vint L0; vint L1;

    if (typ=="AND"){
        L0 = projection(X_00, delta);
        L1 = projection(X_11, delta);
    }else if (typ=="XOR"||typ=="INV"){
        L0 = projection(X_00, delta);
        L1 = projection(X_01,delta);
    }
    return {L0, L1, delta};
}



vint atecaGarble::projection(const vint& a, const vint& b) {
    //projection A o B means take the bit A[i] if B[i]=1
    int l = util::vecHW(b);
    int uintsNeeded = l/64 + ((l%64!=0) ? 1 : 0);
    auto projection = bitset<64>(0);
    auto res = vint(uintsNeeded);
    int bitsProjected =0; int j =0; int blockNum =0;
    do {
        if (util::ithBitL2R(b,j)==1){
            auto ithBitA = util::ithBitL2R(a,j);
            projection[bitsProjected]= ithBitA;
            bitsProjected++;
        }
        j++;

        if (bitsProjected%64==0 && bitsProjected !=0|| bitsProjected==l){
            ::uint64_t projUint = projection.to_ullong();
            res[blockNum] = projUint;
            blockNum++;
        }
    }while(bitsProjected!=l);

    return res;
}




vector<vint> atecaGarble::DecodingInfo(const vector<tuple<vint, vint>>& D, int l, const tuple<vint,vint>& invVar) {
    //RO from 2l->1
    vector<vint> d(D.size());
    for (int i = 0; i < D.size(); ++i) {
        auto L0 = get<0>(D[i]);
        auto L1 = get<1>(D[i]);
        vint L0wdi;
        vint L1wdi;
        vint di;
        vint hashL0;
        vint hashL1;
        int lsbHL0;
        int lsbHL1;
        do {
            di = util::genBitsNonCrypto(l);
            //this is not the cleanest code ever
            L0wdi.clear();
            L0wdi.insert(L0wdi.begin(), L0.begin(), L0.end());
            L0wdi.insert(L0wdi.end(), di.begin(), di.end());
            L1wdi.clear();
            L1wdi.insert(L1wdi.begin(), L1.begin(), L1.end());
            L1wdi.insert(L1wdi.end(), di.begin(), di.end());
            hashL0 = util::hash_variable(util::uintVec2Str(L0wdi),l);
            hashL1 = util::hash_variable(util::uintVec2Str(L1wdi),l);
            lsbHL0=util::checkBit(hashL0[0],0);
            lsbHL1=util::checkBit(hashL1[0],0);

        } while (!((lsbHL0 == 0) && (lsbHL1 == 1)));
        d[i] = di;
    }
    return d;
}

//Evaluator functions

vector<vint>
atecaGarble::En(vector<tuple<vint, vint>> encoding,
                        vector<int> input) {
    auto X = vector<vint>(input.size());
    for (int i = 0; i < input.size(); ++i) {
        if (input[i]== 0){
        X[i]=get<0>(encoding[i]);
        } else{
            X[i]=get<1>(encoding[i]);
        }
    }
    return X;
}

vector<vint>
atecaGarble::Ev(const vector<vint>& F, const vector<vint>& X,
                        vector<string> C, int l, tuple<vint,vint> invVar){
    int internalSecParam = 8 * l;
    int outputBits =stoi( util::split(C[2], ' ')[1]);
    int amountOfWires = stoi(util::split(C[0], ' ')[1]);
    int firstOutputBit = amountOfWires - outputBits;

    auto wires = X;
    wires.resize(amountOfWires);

    auto outputY = vector<vint>(outputBits);

    for (int i = 3; i < C.size(); ++i) {
        auto gateInfo = util::split(C[i], ' ');
        //int inAmount = stoi(gateInfo[0]); int outAmount = stoi(gateInfo[1]);
        int gateNo = (i-3);
        int out;
        string hashInputLabel;
        if (gateInfo[4]=="INV"){
            int in0 = stoi(gateInfo[2]);
            out = stoi(gateInfo[3]);
            auto labelA = wires[in0];
            auto labelB = get<1>(invVar);
            //hash input string
            labelA.insert(labelA.end(), labelB.begin(), labelB.end());
            labelA.push_back(gateNo);
            hashInputLabel = util::uintVec2Str(labelA);
        }else {
            int in0 = stoi(gateInfo[2]);
            int in1 = stoi(gateInfo[3]);
            out = stoi(gateInfo[4]);
            auto labelA = wires[in0];
            auto labelB = wires[in1];
            //hash input string
            labelA.insert(labelA.end(), labelB.begin(), labelB.end());
            labelA.push_back(gateNo);
            hashInputLabel = util::uintVec2Str(labelA);
        }
            auto hashOut = util::hash_variable(hashInputLabel,internalSecParam);
            const auto& delta = F[gateNo];
            auto gateOut = projection(hashOut, delta);
            wires[out] = gateOut;
            if (out >= firstOutputBit){
                outputY[out - firstOutputBit] = gateOut;
            }

    }
    return outputY;
}

vint atecaGarble::De(vector<vint> outputY, vector<vint> d) {
    auto outbits = outputY.size();
    auto unit64sNeeded = outbits/64 + ((outbits%64!=0) ? 1 : 0);
    auto outputSets =  vector<bitset<64>>(unit64sNeeded);

    for (int i = 0; i < outbits; ++i) {
        //get the lsb of the hash
        outputY[i].insert(outputY[i].end(), d[i].begin(), d[i].end());
        auto hash = util::hash_variable(util::uintVec2Str(outputY[i]), 64);
        int lsbHash=util::checkBit(hash[0],0);
        outputSets = util::insertBitVecBitset(outputSets,lsbHash,i);
    }
    auto y = vint(unit64sNeeded);
    for (int i = 0; i < unit64sNeeded; ++i) {
        y[i] = outputSets[i].to_ullong();
    }

    return y;
}

tuple<vint, vint> atecaGarble::genInvVar(int l) {
    vint lw0 = util::genBitsNonCrypto(l);
    vint lw1 = util::vecXOR(util::genBitsNonCrypto(l), lw0);
    return {lw0,lw1};
}

vint atecaGarble::masksForSlices(vint X_00, vint X_01, vint X_10, vint X_11, string typ) {
//and all labels with 0 and uint_max
    //l00a0 is X_00 inverted and l00a1 is X_00 anded with 1
    auto l00a0 = util::vecInvert(X_00);
    auto l00a1 = util::vecAndStatic(X_00, UINT64_MAX);
    //l01a0 and l01a1
    auto l01a0 = util::vecInvert(X_01);
    auto l01a1 = util::vecAndStatic(X_01, UINT64_MAX);
    //l01a0 and l01a1
    auto l10a0 = util::vecInvert(X_10);
    auto l10a1 = util::vecAndStatic(X_10, UINT64_MAX);
    //l11a0 and l11a1
    auto l11a0 = util::vecInvert(X_11);
    auto l11a1 = util::vecAndStatic(X_11, UINT64_MAX);

    //now that you have vectors with 1 for each 0 and 1 for each 1
    //compute mask 0000 =  l00a0 ^ l01a0 ^ l10a0 ^ l11a0
    //masks that and, xor share
    auto mask0000 =  util::vecAND(util::vecAND(util::vecAND(l00a0, l01a0), l10a0), l11a0);///0000
    auto mask1111 =  util::vecAND(util::vecAND(util::vecAND(l00a1, l01a1), l10a1), l11a1);///1111
    //and specific masks
    auto mask0001 =  util::vecAND(util::vecAND(util::vecAND(l00a0, l01a0), l10a0), l11a1);///0001
    auto mask1110 =  util::vecAND(util::vecAND(util::vecAND(l00a1, l01a1), l10a1), l11a0);///1110
    //xor specific masks
    auto mask1001 =  util::vecAND(util::vecAND(util::vecAND(l00a1, l01a0), l10a0), l11a1);///1001
    auto mask0110 =  util::vecAND(util::vecAND(util::vecAND(l00a0, l01a1), l10a1), l11a0);///0110

    vector<::uint64_t> masksORed;
    //or with 0000 0001 1110 or 1111
    //if there is a one in this vector one of the masks had an 1 in the ith bit
    if (typ=="AND"){
        masksORed = util::vecOR(util::vecOR(util::vecOR(mask0000,mask1111),mask0001),mask1110);
        //or with 0000 1001 0110 1111
    }else if (typ=="XOR"| typ=="INV"){
        masksORed =util::vecOR(util::vecOR(util::vecOR(mask0000,mask1111),mask1001),mask0110);
    }
    return masksORed;
}
