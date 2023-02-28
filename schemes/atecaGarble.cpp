//
// Created by a on 21/02/2023.
//

#include "atecaGarble.h"

#include <utility>
#include "../util/util.h"

tuple<vector<vector<::uint64_t>>,vector<tuple<vector<::uint64_t>,vector<::uint64_t>>>,vector<vector<uint64_t>>,int>
    atecaGarble::Gb(int l, const vector<std::string>& C) {

    auto encodingInfo = Init(C, l);
    auto garbledFAndD = GarbleCircuit(l, C, encodingInfo);
    auto decoding = DecodingInfo(get<1>(garbledFAndD), l);

    return {get<0>(garbledFAndD), encodingInfo, decoding, l};
}



vector<tuple<vector<::uint64_t>,vector<::uint64_t>>> atecaGarble::Init(vector<std::string> C, int l) {
    auto inputs = util::split(C[1], ' ');
    int inputWires = 0;
    for (int i = 1; i < inputs.size(); ++i) {
        inputWires+= stoi(inputs[i]);
    }
    vector<tuple<vector<::uint64_t>,vector<::uint64_t>>> e =vector<tuple<vector<::uint64_t>,vector<::uint64_t>>>(inputWires);
    for (int i = 0; i < inputWires; ++i) {
        vector<::uint64_t> lw0 = util::genBitsNonCrypto(l);
        vector<::uint64_t> lw1 = util::vecXOR(util::genBitsNonCrypto(l), lw0);
        tuple<vector<::uint64_t>,vector<::uint64_t>> ew = {lw0,lw1};
        e[i] = ew;
    }

    return e;
}

tuple<vector<vector<::uint64_t>>,vector<tuple<vector<::uint64_t>,vector<::uint64_t>>>>
        atecaGarble::GarbleCircuit(int l, vector<std::string> C,vector<tuple<vector<::uint64_t>, vector<::uint64_t>>> encoding) {
    //get amount of gates, wires and output bits
    auto gatesAndWires=util::split(C[0], ' ');
    int outputBits =stoi( util::split(C[2], ' ')[1]);
    int amountOfWires = stoi(gatesAndWires[1]);

    //set the input wires, as in resize the input labels vector to have room for all wires
    auto wires = std::move(encoding);
    wires.resize(amountOfWires);

    //gabled vector
    auto F = vector<vector<::uint64_t>>(amountOfWires);//too large remove input wires
    //decoding vector
    auto D = vector<tuple<vector<::uint64_t>,vector<::uint64_t>>>(outputBits);

    //output bits are defined as the last k wires, where k is the amount of output bits
    int firstOutputBit = amountOfWires - outputBits;

    //for every Gate in circuit
    for (int i = 3; i < C.size(); ++i) {
        //find input labels
        auto gateInfo = util::split(C[i], ' ');
        int inAmount = stoi(gateInfo[0]); int outAmount = stoi(gateInfo[1]);
        //this always happen xd
        if(inAmount == 2 & outAmount == 1){
            int gateNo = (i-3);
            int in0 = stoi(gateInfo[2]);
            int in1 = stoi(gateInfo[3]);
            int out = stoi(gateInfo[4]);
            string type = gateInfo[5];

            //calculate Gb
            auto garbledGate = Gate(wires[in0], wires[in1], type, gateNo, l);
            //add Delta to F set for a Gate
            F[gateNo]= garbledGate[2];
            //the gates output labels
            wires[out] = {garbledGate[0],garbledGate[1]};

            //if g is an output Gate, add it to D
            if (out >= firstOutputBit){
             //add the label to D
             D[out-firstOutputBit] = {garbledGate[0],garbledGate[1]};
            }
        }
    }
    return {F,D};
}


vector<vector<uint64_t>>
atecaGarble::Gate(const tuple<vector<::uint64_t>, vector<::uint64_t>>& in0, const tuple<vector<::uint64_t>, vector<::uint64_t>>& in1,
                          const string& typ, int gateNo, int l) {
    int internalParam= l * 8;
    //the random oracles lol
    //THIS IS THE WRONG WAY OF TWEAKING!=!=!?!??!!
    vector<::uint64_t> l00 = get<0>(in0);
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
    auto delta = vector<::uint64_t>((internalParam+64-1)/64);
    auto deltaHW =0;

    int j =0;
    do {
        string slice = util::sliceVecL2R(X_00,X_01,X_10,X_11,j);
        if (typ == "AND"){
            //util::ithBitL2R(masksORed,j)
            if (slice =="0000"|| slice =="0001"||slice=="1110"||slice=="1111"){//(util::ithBitL2R(masksORed,j)){
                //or the ith bit with 1
                delta= util::setIthBitTo1L2R(delta,j);
                deltaHW ++;
            }
        }else if(typ == "XOR"){
            //if true
            ///1001
            ///0110
            if (slice =="0000"|| slice =="1001"||slice=="0110"||slice=="1111"){//(util::ithBitL2R(masksORed,j)){
                //update j'th bit of delta to 1
                delta= util::setIthBitTo1L2R(delta,j);
                deltaHW ++;
            }
        }else if(typ == "INV"){
            throw invalid_argument("NOT IMPlEMENTED YET");
        }
        j++;
    } while (deltaHW < l);

    vector<::uint64_t> L0; vector<::uint64_t>L1;

    if (typ=="AND"){
        L0 = projection(X_00, delta);
        L1 = projection(X_11, delta);
    }else if (typ=="XOR"){
        L0 = projection(X_00, delta);
        L1 = projection(X_01,delta);
    }
    //cout << "projects"<<endl;
    //cout<< projection(X_00, delta)[0]<<endl;
    //cout<< projection(X_01, delta)[0]<<endl;
    //cout<< projection(X_10, delta)[0]<<endl;
    //cout<< projection(X_11, delta)[0]<<endl;

    return {L0, L1, delta};
}



vector<uint64_t> atecaGarble::projection(const vector<::uint64_t>& a, const vector<::uint64_t>& b) {
    //projection A o B means take the bit A[i] if B[i]=1
    int l = util::vecHW(b);
    int uintsNeeded = l/64 + ((l%64!=0) ? 1 : 0);
    auto projection = bitset<64>(0);
    auto res = vector<::uint64_t>(uintsNeeded);
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




vector<vector<uint64_t>> atecaGarble::DecodingInfo(const vector<tuple<vector<::uint64_t>, vector<::uint64_t>>>& D, int l) {
    //RO from 2l->1
    vector<vector<::uint64_t>> d(D.size());
    for (int i = 0; i < D.size(); ++i) {
        auto L0 = get<0>(D[i]);
        auto L1 = get<1>(D[i]);
        vector<::uint64_t> L0wdi;
        vector<::uint64_t> L1wdi;
        vector<::uint64_t> di;
        vector<::uint64_t> hashL0;
        vector<::uint64_t> hashL1;
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
        //cout<< "hashL0 in " << L0wdi[0]<<" "<<L0wdi[1]<<endl;
        //cout << "hashL0 " <<hashL0[0] <<endl;
        //cout<< "hashL1 in " << L1wdi[0]<<" "<<L1wdi[1]<<endl;
        //cout << "hashL1 " <<hashL1[0]<<endl;
        d[i] = di;
    }
    return d;
}

//Evaluator functions

vector<vector<::uint64_t>>
atecaGarble::En(vector<tuple<vector<::uint64_t>, vector<::uint64_t>>> encoding,
                        vector<int> input) {
    auto X = vector<vector<::uint64_t>>(input.size());
    for (int i = 0; i < input.size(); ++i) {
        if (input[i]== 0){
        X[i]=get<0>(encoding[i]);
        } else{
            X[i]=get<1>(encoding[i]);
        }
    }
    return X;
}

vector<vector<::uint64_t>>
atecaGarble::Ev(const vector<vector<::uint64_t>>& F, const vector<vector<::uint64_t>>& X,
                        vector<string> C, int l){
    int internalSecParam = 8 * l;
    int outputBits =stoi( util::split(C[2], ' ')[1]);
    int amountOfWires = stoi(util::split(C[0], ' ')[1]);
    int firstOutputBit = amountOfWires - outputBits;

    auto wires = X;
    wires.resize(amountOfWires);

    auto outputY = vector<vector<::uint64_t>>(outputBits);

    for (int i = 3; i < C.size(); ++i) {
        auto gateInfo = util::split(C[i], ' ');
        int inAmount = stoi(gateInfo[0]); int outAmount = stoi(gateInfo[1]);
        if(inAmount == 2 & outAmount == 1){
            int gateNo = (i-3);
            int in0 = stoi(gateInfo[2]);
            int in1 = stoi(gateInfo[3]);
            int out = stoi(gateInfo[4]);
            auto labelA = wires[in0];
            auto labelB = wires[in1];
            //hash input string
            labelA.insert(labelA.end(), labelB.begin(), labelB.end());
            labelA.push_back(gateNo);
            auto hashInputLabel = util::uintVec2Str(labelA);
            auto hashOut = util::hash_variable(hashInputLabel,internalSecParam);
            auto delta = F[gateNo];
            auto gateOut = projection(hashOut, delta);
            //cout<< "ev proj"<<endl;
            //cout << gateOut[0] <<endl;
            wires[out] = gateOut;
            if (out >= firstOutputBit){
                outputY[out - firstOutputBit] = gateOut;
            }
        }
    }
    return outputY;
}

vector<::uint64_t> atecaGarble::De(vector<vector<::uint64_t>> outputY, vector<vector<uint64_t>> d) {
    auto outputBits = outputY.size();
    auto outputSets =  vector<bitset<64>>((outputBits+64-1)/64);

    for (int i = 0; i < outputBits; ++i) {
        //get the lsb of the hash shit
        outputY[i].insert(outputY[i].end(), d[i].begin(), d[i].end());
        auto hash = util::hash_variable(util::uintVec2Str(outputY[i]), 64);
        //cout<< "Dehashin " <<outputY[i][0] << " "<< outputY[i][1]<<endl;
        //cout<< "Dehash "<<hash[0]<<endl;
        int lsbHash=util::checkBit(hash[0],0);
        outputSets = util::insertBitVecBitset(outputSets,lsbHash,i);
    }
    auto y = vector<::uint64_t>(outputY.size());
    for (int i = 0; i < outputSets.size(); ++i) {
        y[i] = outputSets[i].to_ullong();
    }

    return y;
}
