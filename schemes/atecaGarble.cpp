//
// Created by a on 21/02/2023.
//

#include "atecaGarble.h"
#include <utility>
#include <chrono>

using std::chrono::high_resolution_clock;
using std::chrono::duration_cast;
using std::chrono::duration;
using std::chrono::milliseconds;

tuple<vector<vint>, vector<tuple<vint, vint>>, vector<vint>, int, tuple<vint, vint>, hashTCCR>
atecaGarble::garble(circuit &f, int k, util::hashtype hashtype) {

    auto e = Init(f, k);
    auto invVar= genInvVar(k);
    auto [F,D,Invvar,c] = GarbleCircuit(k, f, e, invVar, hashtype);
    auto d = DecodingInfo(D, k, c);
    return {F, e, d, k, invVar, c};
}



vector<tuple<vint,vint>> atecaGarble::Init(circuit &f, int k) {
    int inputWires = circuitParser::getInputSize(f);
    vector<tuple<vint,vint>> e;
    for (int i = 0; i < inputWires; ++i) {
        vint lw0 = util::genBitsNonCrypto(k);
        vint lw1 = util::vecXOR(util::genBitsNonCrypto(k), lw0);
        tuple<vint,vint> ew = {lw0,lw1};
        e.emplace_back(ew);
    }
    return e;
}

tuple<vector<vint>, vector<tuple<vint, vint>>, tuple<vint, vint>, hashTCCR>
atecaGarble::GarbleCircuit(int k, circuit &f, vector<tuple<vint, vint>> e,
                           const tuple<vint, vint> &invVar,
                           util::hashtype hashtype) {
    //my hash struct
    hashTCCR c;
    if (hashtype == util::fast){
        c = hashTCCR(k);
    }
    //get amount of gates, wires and output bits

    int outputBits =circuitParser::getOutBits(f);
    int amountOfWires = circuitParser::getWires(f);

    //set the input wires, as in resize the input labels vector to have room for all wires
    auto wires = std::move(e);
    wires.resize(amountOfWires);

    //gabled vector
    auto F = vector<vint>(amountOfWires);//too large remove input wires
    //decoding vector
    auto D = vector<tuple<vint,vint>>(outputBits);
    //output bits are defined as the last k wires, where k is the amount of output bits
    int firstOutputBit = amountOfWires - outputBits;
    vector<vint> garbledGate;
    int out;

    double gate_time =0;
    //for every Gate in circuit
    for (int i = 2; i < f.size(); ++i) {
        //find input labels
        auto[inWires, outWires, type]=f[i];

        int gateNo = (i - 2);
        //inverse gate hack
        if (type == "INV") {
            int in0 = inWires[0];
            //int in1 = stoi(gateInfo[3]);
            out = outWires[0];

            //calculate garble
            //auto t1 = high_resolution_clock::now();
            garbledGate = Gate(wires[in0], invVar, type, gateNo, k, c);
            //auto t2 = high_resolution_clock::now();
            //duration<double, std::milli> ms_double = t2 - t1;
            //gate_time += ms_double.count();
        }//normal gate
        else {
            int in0 = inWires[0];
            int in1 = inWires[1];
            out = outWires[0];

            //calculate garble
            //auto t1 = high_resolution_clock::now();
            garbledGate = Gate(wires[in0], wires[in1], type, gateNo, k, c);
            //auto t2 = high_resolution_clock::now();
            //duration<double, std::milli> ms_double = t2 - t1;
            //gate_time += ms_double.count();
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
    //cout<< "gate_time,"<< gate_time <<" ms"<<endl;
    return {F,D, invVar,c};
}


vector<vint>
atecaGarble::Gate(const tuple<vint, vint> &in0, const tuple<vint, vint> &in1, const string &typ, int gateNo, int k,
                  hashTCCR &c) {
    int internalParam= k * 8;
    //actually compute the hashes
    vint X_00;vint X_01;vint X_10;vint X_11;
    //auto t1 = high_resolution_clock::now();
    if (c.getHash()==util::RO){
        auto [l00,l11] = in0;
        auto [l_0,l_1] = in1;
        l00.insert(l00.end(), l_0.begin(), l_0.end());
        auto l01 = get<0>(in0);
        l01.insert(l01.end(), l_1.begin(), l_1.end());
        auto l10 = get<1>(in0);
        l10.insert(l10.end(), l_0.begin(), l_0.end());
        l11.insert(l11.end(), l_1.begin(), l_1.end());
        vint tweak {static_cast<unsigned long>(gateNo)};
        X_00 = util::hash_variable(l00, tweak, internalParam);
        X_01 = util::hash_variable(l01, tweak, internalParam);
        X_10 = util::hash_variable(l10, tweak, internalParam);
        X_11 = util::hash_variable(l11, tweak, internalParam);

    }else{
        //do fast stuff
        auto [a0,a1] = in0;
        auto [b0,b1] = in1;
        X_00 = c.prfHash(a0,b0,{(::uint64_t)gateNo},internalParam);
        X_01 = c.prfHash(a0,b1,{(::uint64_t)gateNo},internalParam);
        X_10 = c.prfHash(a1,b0,{(::uint64_t)gateNo},internalParam);
        X_11 = c.prfHash(a1,b1,{(::uint64_t)gateNo},internalParam);

    }
    //auto t2 = high_resolution_clock::now();
    //duration<double, std::milli> ms_double = t2 - t1;
    //cout <<"hash;"<<ms_double.count()<<endl;
    auto delta = vint((internalParam+64-1)/64);
    auto deltaHW =0;

    //t1 = high_resolution_clock::now();
    vint mask = masksForSlices(X_00,X_01,X_10,X_11,typ);
    //t2 = high_resolution_clock::now();
    //ms_double = t2 - t1;
    //cout <<"slicing;"<<ms_double.count()<<endl;
    int j =0;
    //t1 = high_resolution_clock::now();
    do {
        //the choice of masks defines what type of gate were working with
        auto index = j/64;
        auto index2 = (63-(j%64));
        if (util::checkBitL2R(mask[index],index2)){//if (slice =="0000"|| slice =="0001"||slice=="1110"||slice=="1111"){//
            //or the ith bit with 1
            util::setIthBitTo1L2R(&delta,j);
            deltaHW ++;
        }
        j++;
    } while (deltaHW < k);
    //t2 = high_resolution_clock::now();
    //ms_double = t2 - t1;
    //cout <<"bit_mani;"<<ms_double.count()<<endl;

    vint L0; vint L1;
    //t1 = high_resolution_clock::now();
    if (typ=="AND"){
        L0 = util::fastproj(X_00, delta,k);
        L1 = util::fastproj(X_11, delta,k);
    }else if (typ=="XOR"||typ=="INV"){
        L0 = util::fastproj(X_00, delta,k);
        L1 = util::fastproj(X_01,delta,k);
    }
    //t2 = high_resolution_clock::now();
    //ms_double = t2 - t1;
    //cout <<"projection;"<<ms_double.count()<<endl;
    return {L0, L1, delta};
}





vector<vint> atecaGarble::DecodingInfo(const vector<tuple<vint, vint>> &D, int k, hashTCCR &c) {
    //RO from 2l->1
    vector<vint> d(D.size());
    for (int i = 0; i < D.size(); ++i) {
        auto[L0,L1] =D[i];
        vint L0wdi;
        vint L1wdi;
        vint di;
        vint hashL0;
        vint hashL1;
        int lsbHL0;
        int lsbHL1;
        int looped =0;
        do {
            di = util::genBitsNonCrypto(k);
            if (c.hashtype == util::RO){

                //shitty hack with a tweak set to 2
                L0wdi.clear();
                L0wdi.insert(L0wdi.begin(), L0.begin(), L0.end());
                L0wdi.insert(L0wdi.end(), di.begin(), di.end());
                L1wdi.clear();
                L1wdi.insert(L1wdi.begin(), L1.begin(), L1.end());
                L1wdi.insert(L1wdi.end(), di.begin(), di.end());
                hashL0 = util::hash_variable(L0wdi,{0}, 128);
                hashL1 = util::hash_variable(L1wdi,{0}, 128);
            }else{
                hashL0 = c.prfHash(L0, di, {}, 128);
                hashL1 = c.prfHash(L1, di, {}, 128);
            }
            lsbHL0=util::checkBit(hashL0[0],0);
            lsbHL1=util::checkBit(hashL1[0],0);
            looped+=1;
            if (looped>100){
                ::exit(14);
            }
        } while (!((lsbHL0 == 0) && (lsbHL1 == 1)));
        d[i] = di;
    }
    return d;
}

//Evaluator functions

vector<vint>
atecaGarble::encode(vector<tuple<vint, vint>> e,
                    vector<int> x) {
    vector<vint> X;
    for (int i = 0; i < x.size(); ++i) {
        if (x[i] == 0) {
            X.emplace_back(get<0>(e[i]));
        } else {
            X.emplace_back(get<1>(e[i]));
        }
    }
    return X;
}

vector<vint>
atecaGarble::eval(const vector<vint> &F, const vector<vint> &X, circuit &C, int k, tuple<vint, vint> invVar,
                  hashTCCR &dc) {
    int internalSecParam = 8 * k;
    int outputBits =circuitParser::getOutBits(C);
    int amountOfWires = circuitParser::getWires(C);
    int firstOutputBit = amountOfWires - outputBits;
    auto [unused, invTrue] = std::move(invVar);
    auto wires = X;
    wires.resize(amountOfWires);

    auto outputY = vector<vint>(outputBits);

    for (int i = 2; i < C.size(); ++i) {
        auto[inWires, outWires, type]=C[i];
        //int inAmount = stoi(gateInfo[0]); int outAmount = stoi(gateInfo[1]);
        int gateNo = (i-2);
        int out;
        vint labelA; vint labelB;
        string hashInputLabel;
        if (type=="INV"){
            int in0 = inWires[0];
            out = outWires[0];
            labelA = wires[in0];
            labelB = invTrue;
        }else {
            int in0 = inWires[0];
            int in1 = inWires[1];
            out = outWires[0];
            labelA = wires[in0];
            labelB = wires[in1];
        }
        //hash input string
        vint hashout;
        if (dc.hashtype==util::RO){
            labelA.insert(labelA.end(), labelB.begin(), labelB.end());
            vint tweak{static_cast<unsigned long>(gateNo)};
            //hashInputLabel = util::uintVec2Str(labelA);
            hashout = util::hash_variable(labelA,tweak,internalSecParam);
        }else{
            hashout= dc.prfHash(labelA, labelB, {(::uint64_t)gateNo}, internalSecParam);

        }
        const auto& delta = F[gateNo];
        auto gateOut = util::fastproj(hashout, delta,k);
        wires[out] = gateOut;
        if (out >= firstOutputBit){
            outputY[out - firstOutputBit] = gateOut;
        }
    }
    return outputY;
}

vint atecaGarble::decode(vector<vint> Y, vector<vint> d, hashTCCR &dc) {
    auto outbits = Y.size();
    auto unit64sNeeded = outbits/64 + ((outbits%64!=0) ? 1 : 0);
    auto outputSets =  vector<bitset<64>>(unit64sNeeded);

    for (int i = 0; i < outbits; ++i) {
        //get the lsb of the hash
        vint hash;
        if (dc.hashtype==util::RO){
            Y[i].insert(Y[i].end(), d[i].begin(), d[i].end());
            hash = util::hash_variable(Y[i],{0}, 128);
        }else{
            hash= dc.prfHash(Y[i], d[i], {}, 128);
        }
        int lsbHash=util::checkBit(hash[0],0);
        outputSets = util::insertBitVecBitset(outputSets,lsbHash,i);
    }
    auto y = vint(unit64sNeeded);
    for (int i = 0; i < unit64sNeeded; ++i) {
        y[i] = outputSets[i].to_ullong();
    }
    return y;
}

tuple<vint, vint> atecaGarble::genInvVar(int k) {
    vint lw0 = util::genBitsNonCrypto(k);
    vint lw1 = util::vecXOR(util::genBitsNonCrypto(k), lw0);
    return {lw0,lw1};
}

vint atecaGarble::masksForSlices(const vint& X_00, const vint& X_01, const vint& X_10, const vint& X_11, const string& typ) {
    //invX_00 is X_00 inverted
    auto invX_00 = util::vecInvert(X_00);
    //invX_01
    auto invX_01 = util::vecInvert(X_01);
    //invX_01
    auto invX_10 = util::vecInvert(X_10);

     vector<::uint64_t> masksORed(X_00.size());
    //or with 0000 0001 1110 or 1111
    //if there is a one in this vector one of the masks had an 1 in the ith bit
    if (typ=="AND"){
        //and specific masks
        auto mask000x =  util::vecAND(util::vecAND(invX_00, invX_01), invX_10);///000x
        auto mask111x =  util::vecAND(util::vecAND(X_00, X_01), X_10);///111x

        masksORed = util::vecOR(mask000x,mask111x);
        //or with 0000 1001 0110 1111
    }else{//if (typ=="XOR"| typ=="INV")
        //xor and inv specific masks
        //invX_11
        auto invX_11 = util::vecInvert(X_11);
        auto mx00x = util::vecAND(invX_01, invX_10);
        auto mx11x = util::vecAND(X_01,X_10);
        auto m0xx0=util::vecAND(invX_00, invX_11);
        auto m1xx1=util::vecAND(X_00,X_11);
        //auto mask0000 =  util::vecAND(util::vecAND(util::vecAND(invX_00, invX_01), invX_10), invX_11);///0000
        //auto mask1111 =  util::vecAND(util::vecAND(util::vecAND(X_00, X_01), X_10), X_11);///1111
        //auto mask1001 =  util::vecAND(util::vecAND(util::vecAND(X_00, invX_01), invX_10), X_11);///1001
        //auto mask0110 =  util::vecAND(util::vecAND(util::vecAND(invX_00, X_01), X_10), invX_11);///0110

        //masksORed =util::vecOR(util::vecOR(util::vecOR(mask0000,mask1111),mask1001),mask0110);
        masksORed =util::vecAND(util::vecOR(mx00x,mx11x), util::vecOR(m0xx0,m1xx1));
    }
    return masksORed;
}
