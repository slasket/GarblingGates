//
// Created by svend on 020, 20-02-2023.
//

#include <set>
#include <utility>
#include <cassert>
#include "baseGarble.h"


//k is security parameter and f is the function to be garbled with 3 lines of metadata
tuple<tuple<vint, vector<labelPair>, hashRTCCR>, vector<tuple<vint, vint>>, vector<tuple<vint, vint>>>
baseGarble::garble(vector<string> f, int k, util::hashtype hashtype) {
    //get number of wires and gates
    auto &wireAndGates = f[0];
    auto gatesAndWiresSplit = util::split(wireAndGates, ' ');
    int numberOfWires = stoi(gatesAndWiresSplit[1]);

    //get number of input and output bits
    int numberOfInputBits  = util::getBits(f[1]);
    int numberOfOutputBits = util::getBits(f[2]);

    //initialize variables
    auto garbledCircuit = vector<tuple< vint, vint>>();
    auto encOutputLabels = vector<labelPair>(numberOfOutputBits);

    //generate global delta
    auto globalDelta = util::genDelta(k);

    //generate invConst
    vint invConst = util::genBitsNonCrypto(k);

    //init hash
    hashRTCCR hash;
    if(hashtype == util::fast){
        vint key = util::genBitsNonCrypto(256);
        vint iv = util::genBitsNonCrypto(256);
        hash = hashRTCCR(key, iv, k);
    } else
        hash = hashRTCCR();

    //generate all input labels and insert into wireLabels
    auto inputWiresLabels = util::generateRandomLabels(k, globalDelta, numberOfInputBits);
    auto wireLabels = vector<labelPair>(numberOfWires);
    for (int i = 0; i < numberOfInputBits; ++i) {
        wireLabels[i] = inputWiresLabels[i];
    }
    //perform gate by gate garbling
    for (int i = 3; i < f.size(); ++i) {
        //////////////////////// Getting out gate from string //////////////////////////
        auto &line = f[i];
        auto [inputWires,outputWires,gateType] = util::extractGate(line);              // "2 1 0 1 2 XOR"
        //auto inputWires = get<0>(gateInfo);         // [ ..., 0, 1]
        //auto outputWires = get<1>(gateInfo);        // [..., 1, ..., 2]
        //auto gateType = get<2>(gateInfo);               // "XOR"

        ////////////////////////////// Garbling gate ///////////////////////////////////
        //vector<::uint64_t> gate0;
        //vector<::uint64_t> gate1;
        auto [gate0, gate1] = garbleGate(invConst, k, globalDelta, inputWires, gateType, wireLabels, outputWires, hash);

        ////////////////////////////// Construct output {F,e,d} ///////////////////////////////////
        //create output F with gates
        auto gate = make_tuple(gate0, gate1);
        garbledCircuit.emplace_back(gate);

        //output e is inputWireLabels

        //create output d
        if(outputWires[0] >= numberOfWires - numberOfOutputBits){
            string outputStrF = util::uintVec2Str(get<0>(wireLabels[outputWires[0]]));
            string outputStrT = util::uintVec2Str(get<1>(wireLabels[outputWires[0]]));
            tuple<vint,vint> outputLabel = {util::hash_variable(outputStrF), util::hash_variable(outputStrT)};
            encOutputLabels[outputWires[0] - (numberOfWires - numberOfOutputBits)] = outputLabel;
        }

    }
    return {{invConst, garbledCircuit, hash}, inputWiresLabels, encOutputLabels};
}

tuple<vector<::uint64_t>, vector<::uint64_t>>
baseGarble::garbleGate(const vint &invConst, int k, const vector<::uint64_t> &globalDelta, vector<int> inputWires,
                       const string& gateType, vector<labelPair> &wireLabels,
                       vector<int> &outputWires, hashRTCCR hash) {
    vector<::uint64_t> gate0; vector<::uint64_t> gate1;
    auto outputWiresLabels = vector<labelPair>();
    //get input wires
    int input0 = inputWires[0];
    int input1;
    if(inputWires.size() == 1){
        input1 = inputWires[0];
    }
    else{
        input1 = inputWires[1];
    }

    //get input labels
    auto [A0,A1] = wireLabels[input0];
    auto [B0,B1] = wireLabels[input1];
    //calculate permute bits by only getting the least significant bit
    int permuteBitA = util::checkIthBit(A0, 0);
    int permuteBitB = util::checkIthBit(B0, 0);
    vint AF; vint AT;
    vint BF; vint BT;
    //set permuted labels to T and F
    if(permuteBitA == 1){
        AF = A1;
        AT = A0;
    } else {
        AF = A0;
        AT = A1;
    }
    if(permuteBitB == 1){
        BF = B1;
        BT = B0;
    } else {
        BF = B0;
        BT = B1;
    }

    //garble gate by calculating ciphertext(false) and gate ciphertexts
    vint ciphertext;
    if (gateType == "AND")
        andGate(globalDelta, permuteBitA, permuteBitB, AF, AT, BF, BT, ciphertext, gate0, gate1, k, std::move(hash));
    else if (gateType == "XOR")  //free XOR
        xorGate(globalDelta, permuteBitA, permuteBitB, AF, BF, ciphertext);
    else if (gateType == "INV") //XOR with const
        invGate(permuteBitA, AF, AT, ciphertext, invConst);
    //insert output labels into wireLabels
    wireLabels[outputWires[0]] = make_tuple(ciphertext, util::vecXOR(ciphertext, globalDelta));
    return {gate0, gate1};
}


void
baseGarble::andGate(const vint &globalDelta, int permuteBitA, int permuteBitB, vint &A0,
                    vint &A1, vint &B0, vint &B1, vint &ciphertext,
                    vint &gate0, vint &gate1, int k, hashRTCCR hash) {
    if(hash.hashtype == util::RO){
        ciphertext = hashXOR(A0, B0, k);
        gate1 = util::vecXOR(util::vecXOR(hashXOR(A0, B1, k), ciphertext), A0);
        gate0 = util::vecXOR(hashXOR(A1, B0, k), ciphertext);
    } else {
        ciphertext = hashXORfast(A0, B0, k, hash);
        gate1 = util::vecXOR(util::vecXOR(hashXORfast(A0, B1, k, hash), ciphertext), A0);
        gate0 = util::vecXOR(hashXORfast(A1, B0, k, hash), ciphertext);
    }

    if(permuteBitA == 1){
        gate1 = util::vecXOR(gate1, globalDelta);
    } else if(permuteBitB == 1){
        gate0 = util::vecXOR(gate0, globalDelta);
    }
    if(permuteBitA == 1 & permuteBitB == 1){
        ciphertext = util::vecXOR(ciphertext, globalDelta);
        gate0 = util::vecXOR(gate0, globalDelta);
    }

}


void
baseGarble::invGate(int permuteBitA, vint &A0, vint &A1, vint &ciphertext, const vint &invConst) {
    if(permuteBitA == 0){
        ciphertext = util::vecXOR(A1, invConst);
    }   else{
        ciphertext = util::vecXOR(A0, invConst);
    }
}

void
baseGarble::xorGate(const vint &globalDelta, int permuteBitA, int permuteBitB, vint &A0,
                    vint &B0, vint &ciphertext) {
    ciphertext = util::vecXOR(A0, B0);

    if((permuteBitA == 0 & permuteBitB == 1) | (permuteBitA == 1 & permuteBitB == 0) ){
        ciphertext = util::vecXOR(ciphertext,globalDelta);
    }
}



vector<vint> baseGarble::encode(vector<labelPair> e, vector<int> x) {
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

vector<vint> baseGarble::eval(tuple<vint, vector<labelPair>, hashRTCCR> F,
                              vector<vint> X, vector<string> f, int k) {
    //garbled circuit
    auto [invConst, garbledCircuit, hash] = std::move(F);
    //auto inputLabels = get<1>(F);
    //auto outputLabels = get<2>(F);

    //get number of wires and gates from non-garbled circuit
    string &wireAndGates = f[0]; //number of wires and gates
    auto gatesAndWiresSplit = util::split(wireAndGates, ' ');
    int numberOfGates = stoi(gatesAndWiresSplit[0]);
    int numberOfWires = stoi(gatesAndWiresSplit[1]);
    int numberOfInputBits = util::getBits(f[1]);
    auto &outputs = f[2]; //number of outputs and how many bits each output is
    auto outputSplit = util::split(outputs, ' ');
    int numberOfOutputBits = util::getBits(f[2]);

    //get input labels from e and put them in wireValues
    auto wireValues = vector< vint>(numberOfWires);
    for (int i = 0; i < X.size(); ++i) {
        wireValues[i] = X[i];
    }

    //check if the number of gates in the garbled circuit is the same as the number of gates in the non-garbled circuit
    if(garbledCircuit.size() != f.size()-3 & f.size()-3 != numberOfGates){
        cout << "garbledCircuit.size() != f.size()-3 != " << numberOfGates  << " != " << garbledCircuit.size() << " != " << f.size()-3 << endl;
        //exit(1);
    }

    //result vector
    vector<vint> Y = vector<vint>(numberOfOutputBits);

    //evaluate the garbled circuit
    for (int i = 0; i < garbledCircuit.size(); ++i) {
        //////////////////////// Getting out gate from string //////////////////////////
        auto &line = f[i+3];
        auto [inputWires, outputWires, gateType] = util::extractGate(line);              // "2 1 0 1 2 XOR"
        //auto inputWires = get<0>(gateInfo);         // [ ..., 0, 1]
        //auto outputWires = get<1>(gateInfo);        // [..., 1, ..., 2]
        //auto gateType = get<2>(gateInfo);               // "XOR"
        //////////////////////// Evaluation of gate //////////////////////////

        //check if input labels for gate is calculated
        for (int inputWire : inputWires) {
            if(wireValues[inputWire].empty()){
                cout << "wireValues[inputWires[" << inputWire << "]].empty()" << endl;
                //exit(1);
            }
        }
        auto cipher = evalGate(invConst, k, garbledCircuit, wireValues, i, inputWires, gateType, hash);

        //save output value in wireValues
        wireValues[outputWires[0]] = cipher;
        //save output value in Y if output wire
        if (outputWires[0] >= numberOfWires - numberOfOutputBits) {
            Y[outputWires[0] - (numberOfWires - numberOfOutputBits)] = cipher;
        }
    }
    return Y;
}

vint baseGarble::evalGate(const vint &invConst, int k,
                          const vector<labelPair> &garbledCircuit,
                          const vector<vint> &wireValues, int i, vector<int> inputWires,
                          const string& gateType,
                          hashRTCCR hash) {//get input values
    int input0 = inputWires[0];
    int input1;
    if(gateType == "INV"){
        input1 = inputWires[0];
    } else {
        input1 = inputWires[1];
    }

    //get input labels
    auto A = wireValues[input0];
    auto B = wireValues[input1];
    vint cipher;

    //evaluate gate
    if (gateType == "XOR"){
        cipher = util::vecXOR(A, B);
    } else
    if(gateType == "INV"){
        cipher = util::vecXOR(A, invConst);
    } else
    if(gateType == "AND"){
        //get color bits
        auto colorBitA = util::checkIthBit(A,0);//A[0]) & 1;
        auto colorBitB = util::checkIthBit(B, 0);//B[0]) & 1;
        //get gate ciphertexts
        auto [gate0,gate1] = garbledCircuit[i];
        //evaluate gate
        if(hash.hashtype == util::RO) {
            if (colorBitA == 0 && colorBitB == 0) {
                cipher = hashXOR(A, B, k);
            } else if (colorBitA == 0 && colorBitB == 1) {
                cipher = util::vecXOR(util::vecXOR(hashXOR(A, B, k), gate1), A);
            } else if (colorBitA == 1 && colorBitB == 0) {
                cipher = util::vecXOR(hashXOR(A, B, k), gate0);
            } else if (colorBitA == 1 && colorBitB == 1) {
                cipher = util::vecXOR(util::vecXOR(util::vecXOR(hashXOR(A, B, k), gate0), gate1), A);
            }
        }
        else {
            if (colorBitA == 0 && colorBitB == 0) {
                cipher = hashXORfast(A, B, k, hash);
            } else if (colorBitA == 0 && colorBitB == 1) {
                cipher = util::vecXOR(util::vecXOR(hashXORfast(A, B, k, hash), gate1), A);
            } else if (colorBitA == 1 && colorBitB == 0) {
                cipher = util::vecXOR(hashXORfast(A, B, k, hash), gate0);
            } else if (colorBitA == 1 && colorBitB == 1) {
                cipher = util::vecXOR(util::vecXOR(util::vecXOR(hashXORfast(A, B, k, hash), gate0), gate1), A);
            }
        }
    }
    return cipher;
}

vint baseGarble::decode(vector<labelPair> d, vector<vint> Y) {
    auto outbits = Y.size();
    auto unit64sNeeded = outbits/64 + ((outbits%64!=0) ? 1 : 0);
    auto outputSets =  vector<bitset<64>>(unit64sNeeded);

    for (int i = 0; i < outbits; ++i) {
        //get the lsb of the hash
        string ystring = util::uintVec2Str(Y[i]);
        auto yhash = util::hash_variable(ystring, 128);
        if (yhash == get<0>(d[i])) {
            outputSets = util::insertBitVecBitset(outputSets,0,i);
        } else if (yhash == get<1>(d[i])){
            outputSets = util::insertBitVecBitset(outputSets,1,i);
        } else {
            cout << "Could not decodeBits as encrypted output was invalid" << endl;
        }
    }
    auto y = vint(unit64sNeeded);
    for (int i = 0; i < unit64sNeeded; ++i) {
        y[i] = outputSets[i].to_ullong();
    }
    return y;
}
vector<int> baseGarble::decodeBits(vector<labelPair> d, vector<vint> Y) {
    vector<int> y;
    for (int i = 0; i < Y.size(); ++i) {
        string ystring = util::uintVec2Str(Y[i]);
        auto yhash = util::hash_variable(ystring, 128);
        if (yhash == get<0>(d[i])) {
            y.push_back(0);
        } else if (yhash == get<1>(d[i])) {
            y.push_back(1);
        } else {
            y.push_back(2);
            cout << "Could not decodeBits as encrypted output was invalid" << endl;
        }
    }
    return y;
}

vint baseGarble::hashFunc(vint x, int k) {
    auto xstring = util::printBitsetofVectorofUints(std::move(x));
    return util::hash_variable(xstring, k);
}



vint baseGarble::hashXOR(vint &labelA, vint &labelB, int k){
    return util::vecXOR(hashFunc(labelA, k), hashFunc(labelB, k));
}

vint baseGarble::hashXORfast(vint &labelA, vint &labelB, int k, hashRTCCR &fh){
    auto a = fh.hash(labelA, {static_cast<unsigned long long>(k)}, fh.getKey(), fh.getIv(), fh.getE(), fh.getAlpha(), fh.getU1(), fh.getU2());
    auto b = fh.hash(labelA, {static_cast<unsigned long long>(k)}, fh.getKey(), fh.getIv(), fh.getE(), fh.getAlpha(), fh.getU1(), fh.getU2());
    return util::vecXOR(a, b);
}


