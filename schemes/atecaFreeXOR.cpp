//
// Created by a on 10/03/2023.
//

#include "atecaFreeXOR.h"
#include "atecaGarble.h"

tuple<vector<vint>, vector<tuple<vint, vint>>, vector<vint>, int, tuple<vint, vint>,string>
    atecaFreeXOR::Gb(int l, const vector<std::string> &C, string hashtype) {


    auto [encodingInfo,globalDelta] = Init(C, l);
    auto invVar= genInvVar(l, globalDelta);
    cout<<"garblefunc"<<endl;
    auto [F,D,Invvar] = GarbleCircuit(l, C, encodingInfo, invVar, globalDelta);
    cout<<"decoding"<<endl;
    auto decoding = DecodingInfo(D, l);


    return {F,encodingInfo,decoding,l,invVar,hashtype};
}

tuple<vector<tuple<vint, vint>>, vint> atecaFreeXOR::Init(vector<std::string> C, int l) {
    auto globalDelta = util::genBitsNonCrypto(l);

    auto inputs = util::split(C[1], ' ');
    int inputWires = 0;
    for (int i = 1; i < inputs.size(); ++i) {
        inputWires+= stoi(inputs[i]);
    }
    vector<tuple<vint,vint>> e =vector<tuple<vint,vint>>(inputWires);
    for (int i = 0; i < inputWires; ++i) {
        vint lw0 = util::genBitsNonCrypto(l);
        vint lw1 = util::vecXOR(globalDelta, lw0);
        tuple<vint,vint> ew = {lw0,lw1};
        e[i] = ew;
    }

    return {e,globalDelta};
}

tuple<vint, vint> atecaFreeXOR::genInvVar(int l, vint globalDelta) {
    vint lw0 = util::genBitsNonCrypto(l);
    vint lw1 = util::vecXOR(globalDelta, lw0);
    return {lw0,lw1};
}

tuple<vector<vint>, vector<tuple<vint, vint>>, tuple<vint, vint>>
atecaFreeXOR::GarbleCircuit(int l, vector<std::string> C, vector<tuple<vint, vint>> encoding,
                            const tuple<vint, vint> &invVar, vint globalDelta) {
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
        ///CHECK FOR XOR OR INVERSE
        if (gateInfo[4] == "INV") {
            int in0 = stoi(gateInfo[2]);
            //int in1 = stoi(gateInfo[3]);
            out = stoi(gateInfo[3]);
            string type = gateInfo[4];

            //calculate Gb
            ///must return L0, L1, Delta

            auto l0= util::vecXOR(get<1>(wires[in0]),get<1>(invVar));
            auto l1= util::vecXOR(get<0>(wires[in0]),get<1>(invVar));
            garbledGate = {l0,l1};

        } else if(gateInfo[5] == "XOR"){
            int in0 = stoi(gateInfo[2]);
            int in1 = stoi(gateInfo[3]);
            out = stoi(gateInfo[4]);
            string type = gateInfo[5];

            auto l0= util::vecXOR(get<0>(wires[in0]),get<0>(wires[in1]));
            auto l1= util::vecXOR(get<0>(wires[in0]),get<1>(wires[in1]));
            garbledGate = {l0,l1};
        }//AndGate
        else {
            int in0 = stoi(gateInfo[2]);
            int in1 = stoi(gateInfo[3]);
            out = stoi(gateInfo[4]);
            string type = gateInfo[5];

            //calculate Gb
            garbledGate = Gate(wires[in0], wires[in1], type, gateNo, l, globalDelta);
        }
        //add Delta to F set for a Gate
        if (gateInfo[4]=="INV" ||gateInfo[5]=="XOR"){
            F[gateNo] = {};
        }else{
            F[gateNo] = garbledGate[2];
        }
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

vector<vint>
atecaFreeXOR::Gate(const tuple<vint, vint> &in0, const tuple<vint, vint> &in1, const string &typ, int gateNo, int l,
                   vint globalDelta) {
    int internalParam= l * 16;
    //the random oracles lol
    //THIS IS THE WRONG WAY OF TWEAKING!=!=!?!??!!
    vint l00 = get<0>(in0);
    l00.insert(l00.end(), get<0>(in1).begin(), get<0>(in1).end());
    auto l01 = get<0>(in0);
    l01.insert(l01.end(), get<1>(in1).begin(), get<1>(in1).end());
    auto l10 = get<1>(in0);
    l10.insert(l10.end(), get<0>(in1).begin(), get<0>(in1).end());
    auto l11 = get<1>(in0);
    l11.insert(l11.end(), get<1>(in1).begin(), get<1>(in1).end());
    //actually compute the hashes
    vint X_00 = util::hash_variable(util::uintVec2Str(l00) + to_string(gateNo), internalParam);
    vint X_01 = util::hash_variable(util::uintVec2Str(l01) + to_string(gateNo), internalParam);
    vint X_10 = util::hash_variable(util::uintVec2Str(l10) + to_string(gateNo), internalParam);
    vint X_11 = util::hash_variable(util::uintVec2Str(l11) + to_string(gateNo), internalParam);
    auto delta = vint(internalParam/ 64);

    int j =0; int deltaHW =0;
    while (deltaHW!=l) {
        string slice = util::sliceVecL2RAtecaFreeXorSpecial(globalDelta, X_00, X_01, X_10, X_11,deltaHW, j);
        ///slices of importance "00000", "10001", "11110", "01111"
        if (slice=="00000"||slice=="10001"||slice=="11110"||slice=="01111"){
            delta=util::setIthBitTo1L2R(delta,j);
            deltaHW++;
        }
        j++;
    };

    vint L0 = atecaGarble::projection(X_00, delta);
    vint L1 = atecaGarble::projection(X_11, delta);

    //auto test = util::vecXOR(L0, L1);
    //if (test != delta) {
    //    cout << "ERROR: X_00 XOR X_01 != delta" << endl;
    //}

    return {L0,L1,delta};
}

vector<vint> atecaFreeXOR::DecodingInfo(const vector<tuple<vint, vint>> &D, int l) {
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


vector<vint>
atecaFreeXOR::En(vector<tuple<vint, vint>> encoding,
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
atecaFreeXOR::Ev(const vector<vint> &F, const vector<vint> &X, vector<string> C, int l, tuple<vint, vint> invVar) {
    int internalSecParam = 16 * l;
    int outputBits =stoi( util::split(C[2], ' ')[1]);
    int amountOfWires = stoi(util::split(C[0], ' ')[1]);
    int firstOutputBit = amountOfWires - outputBits;

    auto wires = X;
    wires.resize(amountOfWires);

    auto outputY = vector<vint>(outputBits);

    for (int i = 3; i < C.size(); ++i) {
        auto gateInfo = util::split(C[i], ' ');

        int gateNo = (i-3);
        int out;
        vint gateOut;
        if (gateInfo[4]=="INV"){
            int in0 = stoi(gateInfo[2]);
            out = stoi(gateInfo[3]);
            auto labelA = wires[in0];
            auto labelB = get<1>(invVar);
            gateOut = util::vecXOR(labelA,labelB);
            //perform the gate
            wires[out]=gateOut;

        }else if(gateInfo[5]=="XOR"){
            int in0 = stoi(gateInfo[2]);
            int in1 = stoi(gateInfo[3]);
            out = stoi(gateInfo[4]);
            auto labelA = wires[in0];
            auto labelB = wires[in1];
            gateOut = util::vecXOR(labelA,labelB);
            //perform the gate
            wires[out]=gateOut;

        }else{//AND GATE
            int in0 = stoi(gateInfo[2]);
            int in1 = stoi(gateInfo[3]);
            out = stoi(gateInfo[4]);
            auto labelA = wires[in0];
            auto labelB = wires[in1];
            //hash input string
            labelA.insert(labelA.end(), labelB.begin(), labelB.end());
            auto hashInputLabel = util::uintVec2Str(labelA);
            auto hashOut = util::hash_variable(hashInputLabel+ to_string(gateNo),internalSecParam);
            const auto& delta = F[gateNo];
            gateOut = atecaGarble::projection(hashOut, delta);
            wires[out] = gateOut;
        }
        //add the output to the output vec
        if (out >= firstOutputBit){
            outputY[out - firstOutputBit] = gateOut;
        }

    }
    return outputY;
}

vint atecaFreeXOR::De(vector<vint> outputY, vector<vint> d) {
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

