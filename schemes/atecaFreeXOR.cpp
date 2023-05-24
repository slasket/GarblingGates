//
// Created by a on 10/03/2023.
//

#include "atecaFreeXOR.h"
#include "atecaGarble.h"
#include <chrono>

using std::chrono::high_resolution_clock;
using std::chrono::duration_cast;
using std::chrono::duration;
using std::chrono::milliseconds;

tuple<vector<vint>, vector<tuple<vint, vint>>, vector<vint>, int, tuple<vint, vint>, hashTCCR>
    atecaFreeXOR::garble(circuit &f, int k, util::hashtype hashtype) {

    auto [e,globalDelta] = Init(f, k);
    auto invVar = genInvVar(k, globalDelta);
    auto [F,D,Invvar,c] = GarbleCircuit(k, f, e, invVar, globalDelta, hashtype);
    auto d = DecodingInfo(D, k, c);

    return {F, e, d, k, invVar, c};
}

tuple<vector<tuple<vint, vint>>, vint> atecaFreeXOR::Init(circuit &f, int k) {
    auto globalDelta = util::genBitsNonCrypto(k);
    auto inputs = circuitParser::getInputSize(f);

    vector<tuple<vint,vint>> e;
    for (int i = 0; i < inputs; ++i) {
        vint lw0 = util::genBitsNonCrypto(k);
        vint lw1 = util::vecXOR(globalDelta, lw0);
        tuple<vint,vint> ew = {lw0,lw1};
        e.emplace_back(ew);
    }

    return {e,globalDelta};
}

tuple<vint, vint> atecaFreeXOR::genInvVar(int k, vint globalDelta) {
    vint lw0 = util::genBitsNonCrypto(k);
    vint lw1 = util::vecXOR(globalDelta, lw0);
    return {lw0,lw1};
}

tuple<vector<vint>, vector<tuple<vint, vint>>, tuple<vint, vint>, hashTCCR>
atecaFreeXOR::GarbleCircuit(int k, circuit &f, vector<tuple<vint, vint>> encoding,
                            const tuple<vint, vint> &invVar, const vint &globalDelta, util::hashtype hashtype) {

    hashTCCR c;
    if (hashtype == util::fast){
        c = hashTCCR(k);
    }
    //get amount of gates, wires and output bits
    int outputBits =circuitParser::getOutBits(f);
    int amountOfWires = circuitParser::getWires(f);

    //set the input wires, as in resize the input labels vector to have room for all wires
    auto wires = std::move(encoding);
    wires.resize(amountOfWires);

    //gabled vector
    auto F = vector<vint>(amountOfWires);
    auto D = vector<tuple<vint,vint>>(outputBits);
    int firstOutputBit = amountOfWires - outputBits;
    vector<vint> garbledGate;
    int out;
    //for every Gate in circuit
    for (int i = 2; i < f.size(); ++i) {
        //find input labels
        auto[inWires, outWires, type]=f[i];

        int gateNo = (i - 2);
        if (type == "INV") {
            invGate(invVar, wires, garbledGate, out, inWires, outWires);

        } else if(type == "XOR"){
            xorGate(wires, garbledGate, out, inWires, outWires);

        }//AndGate
        else {
            andGate(k, globalDelta, wires, gateNo, c, F, garbledGate, out, inWires, outWires);

        }
        wires[out] = {garbledGate[0], garbledGate[1]};
        //if g is an output Gate, add it to D
        if (out >= firstOutputBit) {
            //add the label to D
            D[out-firstOutputBit] = {garbledGate[0],garbledGate[1]};
        }
    }

    return {F,D, invVar,c};
}

void
atecaFreeXOR::andGate(int k, const vint &globalDelta, const vector<tuple<vint, vint>> &wires, int gateNo, hashTCCR &c,
                      vector<vint> &F, vector<vint> &garbledGate, int &out, vector<int> &inWires, vector<int> &outWires) {
    int in0 = inWires[0];
    int in1 = inWires[1];
    out = outWires[0];

    //calculate garble
    garbledGate = Gate(wires[in0], wires[in1], gateNo, k, globalDelta, c);
    F[gateNo] = garbledGate[2];
}

void
atecaFreeXOR::xorGate(vector<tuple<vint, vint>> &wires, vector<vint> &garbledGate, int &out, vector<int> &inWires,
                      vector<int> &outWires) {
    int in0 = inWires[0];
    int in1 = inWires[1];
    out = outWires[0];

    auto l0= util::vecXOR(get<0>(wires[in0]),get<0>(wires[in1]));
    auto l1= util::vecXOR(get<0>(wires[in0]),get<1>(wires[in1]));
    garbledGate = {l0,l1};
}

void atecaFreeXOR::invGate(const tuple<vint, vint> &invVar, const vector<tuple<vint, vint>> &wires,
                           vector<vint> &garbledGate, int &out, vector<int> &inWires, vector<int> &outWires) {
    int in0 = inWires[0];
    //int in1 = stoi(gateInfo[3]);
    out = outWires[0];

    //calculate garble
///must return L0, L1, Delta
    auto l0= util::vecXOR(get<1>(wires[in0]),get<1>(invVar));
    auto l1= util::vecXOR(get<0>(wires[in0]),get<1>(invVar));
    garbledGate = {l0,l1};
}


vector<vint>
atecaFreeXOR::Gate(const tuple<vint, vint> &in0, const tuple<vint, vint> &in1, int gateNo, int k,
                   const vint &globalDelta, hashTCCR &c) {
    int internalParam= k * 16;
    vint X_00;vint X_01;vint X_10;vint X_11;
    //auto t1 = high_resolution_clock::now();
    if (c.getHash()==util::RO){
        vint l00 = get<0>(in0);
        l00.insert(l00.end(), get<0>(in1).begin(), get<0>(in1).end());
        //l00.push_back(gateNo);
        auto l01 = get<0>(in0);
        l01.insert(l01.end(), get<1>(in1).begin(), get<1>(in1).end());
        //l01.push_back(gateNo);
        auto l10 = get<1>(in0);
        l10.insert(l10.end(), get<0>(in1).begin(), get<0>(in1).end());
        //l10.push_back(gateNo);
        auto l11 = get<1>(in0);
        l11.insert(l11.end(), get<1>(in1).begin(), get<1>(in1).end());
        //l11.push_back(gateNo);
        //actually compute the hashes
        vint tweak {static_cast<unsigned long>(gateNo)};
        X_00 = util::hash_variable(l00,tweak, internalParam);
        X_01 = util::hash_variable(l01,tweak, internalParam);
        X_10 = util::hash_variable(l10,tweak, internalParam);
        X_11 = util::hash_variable(l11,tweak, internalParam);
    }else{
        auto [a0,a1] = in0;
        auto [b0,b1] = in1;
        X_00 = c.hash(a0,b0,{(::uint64_t)gateNo},internalParam);
        X_01 = c.hash(a0,b1,{(::uint64_t)gateNo},internalParam);
        X_10 = c.hash(a1,b0,{(::uint64_t)gateNo},internalParam);
        X_11 = c.hash(a1,b1,{(::uint64_t)gateNo},internalParam);
    }
    //auto t2 = high_resolution_clock::now();
    //duration<double, std::milli> ms_double = t2 - t1;
    //cout <<"hash;"<<ms_double.count()<<endl;

    auto delta = vint((internalParam+63)/64);
    int j =0; int deltaHW =0;

    //t1 = high_resolution_clock::now();
    auto [d0flags, d1flags] = ateFXorSlicing(X_00, X_01, X_10, X_11);
    //t2 = high_resolution_clock::now();
    //ms_double = t2 - t1;
    //cout <<"slicing;"<<ms_double.count()<<endl;

    //t1 = high_resolution_clock::now();
    do {
        int flag = ateFXORSliceCheck(globalDelta, d0flags, d1flags, deltaHW, j);
        //string slice = util::sliceVecL2RAtecaFreeXorSpecial(globalDelta, X_00, X_01, X_10, X_11, deltaHW, j);
        ///slices of importance "00000", "10001", "11110", "01111"
        if(flag){//if (slice=="00000"||slice=="10001"||slice=="11110"||slice=="01111"){//
            util::setIthBitTo1L2R(&delta,j);
            deltaHW++;
        }
        j++;
    }while(deltaHW != k);
    //t2 = high_resolution_clock::now();
    //ms_double = t2 - t1;
    //cout <<"bit_mani;"<<ms_double.count()<<endl;

    //t1 = high_resolution_clock::now();
    vint L0 = util::fastproj(X_00, delta,k);
    vint L1 = util::fastproj(X_11, delta,k);
    //t2 = high_resolution_clock::now();
    //ms_double = t2 - t1;
    //cout <<"projection;"<<ms_double.count()<<endl;

    return {L0,L1,delta};
}

vector<vint>
atecaFreeXOR::DecodingInfo(const vector<tuple<vint, vint>> &D, int k, hashTCCR &c) {
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
            di = util::genBitsNonCrypto(k);
            if (c.hashtype==util::RO){
                L0wdi.clear();
                L0wdi.insert(L0wdi.begin(), L0.begin(), L0.end());
                L0wdi.insert(L0wdi.end(), di.begin(), di.end());
                L1wdi.clear();
                L1wdi.insert(L1wdi.begin(), L1.begin(), L1.end());
                L1wdi.insert(L1wdi.end(), di.begin(), di.end());
                hashL0 = util::hash_variable(L0wdi,{0}, 128);
                hashL1 = util::hash_variable(L1wdi,{0}, 128);
            }else{
            //this is not the cleanest code ever
                hashL0 = c.hash(L0, di, {}, 128);
                hashL1 = c.hash(L1, di, {}, 128);
            }
            lsbHL0=util::checkBit(hashL0[0],0);
            lsbHL1=util::checkBit(hashL1[0],0);
        } while (!((lsbHL0 == 0) && (lsbHL1 == 1)));
        d[i] = di;
    }
    return d;
}


vector<vint>
atecaFreeXOR::encode(vector<tuple<vint, vint>> e, vector<int> x) {
    vector<vint> X;
    for (int i = 0; i < x.size(); ++i) {
        if (x[i] == 0){
            X.emplace_back(get<0>(e[i]));
        } else {
            X.emplace_back(get<1>(e[i]));
        }
    }
    return X;
}

vector<vint>
atecaFreeXOR::eval(const vector<vint> &F, const vector<vint> &X, circuit &f, int k, tuple<vint, vint> invVar,
                   hashTCCR &c) {
    int internalSecParam = 16 * k;
    int outputBits = circuitParser::getOutBits(f);
    int numberOfWires = circuitParser::getWires(f);
    int firstOutputBit = numberOfWires - outputBits;

    auto wires = X;
    wires.resize(numberOfWires);

    auto outputY = vector<vint>(outputBits);

    for (int i = 2; i < f.size(); ++i) {
        auto[inWires, outWires, type]=f[i];

        int gateNo = (i-2);
        int out;
        vint gateOut;
        if (type=="INV"){
            int in0 = inWires[0];
            out = outWires[0];
            auto labelA = wires[in0];
            auto labelB = get<1>(invVar);
            gateOut = util::vecXOR(labelA,labelB);
            //perform the gate
            wires[out]=gateOut;

        }else if(type=="XOR"){
            int in0 = inWires[0];
            int in1 = inWires[1];
            out = outWires[0];
            auto labelA = wires[in0];
            auto labelB = wires[in1];
            gateOut = util::vecXOR(labelA,labelB);
            //perform the gate
            wires[out]=gateOut;

        }else{//AND GATE
            int in0 = inWires[0];
            int in1 = inWires[1];
            out = outWires[0];
            auto labelA = wires[in0];
            auto labelB = wires[in1];
            //hash input string
            vint hash;
            if (c.hashtype==util::RO){
                labelA.insert(labelA.end(), labelB.begin(), labelB.end());
                //labelA.push_back(gateNo);
                vint tweak{static_cast<unsigned long>(gateNo)};
                //auto hashInputLabel = util::uintVec2Str(labelA);
                hash = util::hash_variable(labelA,tweak,internalSecParam);
            }else{
                hash= c.hash(labelA, labelB, {(::uint64_t)gateNo}, internalSecParam);
            }
            const auto& delta = F[gateNo];
            gateOut = util::fastproj(hash, delta,k);
            wires[out] = gateOut;
        }
        //add the output to the output vec
        if (out >= firstOutputBit){
            outputY[out - firstOutputBit] = gateOut;
        }

    }
    return outputY;
}

vint atecaFreeXOR::decode(vector<vint> Y, vector<vint> d, hashTCCR &dc) {
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
            hash= dc.hash(Y[i], d[i], {}, 128);
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

inline tuple<vint, vint> atecaFreeXOR::ateFXorSlicing(const vint& X_00, const vint& X_01, const vint& X_10, const vint& X_11) {
    //l00a0 is X_00 inverted
    auto l00a0 = util::vecInvert(X_00);
    //l01a0
    auto l01a0 = util::vecInvert(X_01);
    //l01a0
    auto l10a0 = util::vecInvert(X_10);
    //l11a0
    auto l11a0 = util::vecInvert(X_11);

    ///truthbits for Delta=0
    auto mask0000=util::vecAND(util::vecAND(util::vecAND(l00a0,l01a0),l10a0),l11a0);///0000
    auto mask1111=util::vecAND(util::vecAND(util::vecAND(X_00,X_01),X_10),X_11);///1111
    ///truthbits for delta=1
    auto mask0001=util::vecAND(util::vecAND(util::vecAND(l00a0,l01a0),l10a0),X_11);///0001
    auto mask1110=util::vecAND(util::vecAND(util::vecAND(X_00,X_01),X_10),l11a0);///1110

    auto dlt0Mask = util::vecOR(mask0000,mask1111);
    auto dlt1Mask = util::vecOR(mask0001,mask1110);
    return{dlt0Mask,dlt1Mask};

}

int atecaFreeXOR::ateFXORSliceCheck(const vint& globalDelta, const vint& d0flags, const vint& d1flags, int hw, int j) {
    int gdj = util::ithBitL2R(globalDelta,hw);
    if (gdj){
        //if gdj is 1 check d1flags for a 1
        if (util::ithBitL2R(d1flags,j)){
            return 1;
        }
    }else{
        //gdj is 0 check d0flags for a 1
        if (util::ithBitL2R(d0flags,j)){
            return 1;
        }
    }
    return 0;
}

