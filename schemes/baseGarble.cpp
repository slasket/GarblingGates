//
// Created by svend on 020, 20-02-2023.
//

#include <set>
#include <utility>
#include "baseGarble.h"
#include "../util/util.h"

//k is security parameter and f is the function to be garbled with 3 lines of metadata
tuple<vector<labelPair>,
        vector<labelPair>,
        vector<labelPair>>
baseGarble::garble(vector<string> f, const vint& invConst, int k) {
    //get number of wires and gates
    auto &wireAndGates = f[0];
    auto gatesAndWiresSplit = util::split(wireAndGates, ' ');
    int numberOfWires = stoi(gatesAndWiresSplit[1]);

    //get number of input and output bits
    int numberOfInputBits;
    getBits(f[1], numberOfInputBits);
    int numberOfOutputBits;
    getBits(f[2], numberOfOutputBits);

    //initialize variables
    auto garbledCircuit = vector<tuple< vint, vint>>();
    auto encOutputLabels = vector<labelPair>();

    //generate global delta
    auto globalDelta = util::genDelta(k);

    //generate all input labels and insert into wireLabels
    auto inputWiresLabels = vector<labelPair>(numberOfInputBits);
    auto deltaAndLabels = util::generateRandomLabels(k, globalDelta, inputWiresLabels);
    auto wireLabels = vector<labelPair>(numberOfWires);
    for (int i = 0; i < numberOfInputBits; ++i) {
        wireLabels[i] = inputWiresLabels[i];
    }
    //perform gate by gate garbling
    for (int i = 3; i < f.size(); ++i) {
        //////////////////////// Getting out gate from string //////////////////////////
        auto &line = f[i];
        auto gateInfo = extractGate(line);              // "2 1 0 1 2 XOR"
        auto inputWires = get<0>(gateInfo);         // [ ..., 0, 1]
        auto outputWires = get<1>(gateInfo);        // [..., 1, ..., 2]
        auto gateType = get<2>(gateInfo);               // "XOR"

        ////////////////////////////// Garbling gate ///////////////////////////////////
        vector<::uint64_t> gate0;
        vector<::uint64_t> gate1;
        garbleGate(invConst, k, globalDelta, inputWires, gateType, wireLabels, outputWires, gate0, gate1);

        ////////////////////////////// Construct output {F,e,d} ///////////////////////////////////
        //create output F with gates
        auto gate = make_tuple(gate0, gate1);
        garbledCircuit.emplace_back(gate);

        //output e is inputWireLabels

        //create output d //todo change to how ateca does it
        if(outputWires[0] >= numberOfWires - numberOfOutputBits){
            auto outputLabel = wireLabels[outputWires[0]];
            encOutputLabels.emplace_back(outputLabel);
        }

    }
    return {garbledCircuit, inputWiresLabels, encOutputLabels};
}

void
baseGarble::garbleGate(const vint &invConst, int k, const vector<::uint64_t> &globalDelta, vector<int> inputWires,
                       const string& gateType, vector<labelPair> &wireLabels,
                       vector<int> &outputWires, vector<::uint64_t> &gate0, vector<::uint64_t> &gate1) {
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
    auto ALabels = wireLabels[input0];
    auto BLabels = wireLabels[input1];
    //calculate permute bits
    int permuteBitA = (get<0>(ALabels)[0]) & 1;
    int permuteBitB = (get<0>(BLabels)[0]) & 1;
    vint AF; vint AT;
    vint BF; vint BT;
    //set permuted labels to T and F
    if(permuteBitA == 1){
        AF = (get<1>(ALabels));
        AT = (get<0>(ALabels));
    } else {
        AF = (get<0>(ALabels));
        AT = (get<1>(ALabels));
    }
    if(permuteBitB == 1){
        BF = (get<1>(BLabels));
        BT = (get<0>(BLabels));
    } else {
        BF = (get<0>(BLabels));
        BT = (get<1>(BLabels));
    }

    //garble gate by calculating ciphertext(false) and gate ciphertexts
    vint ciphertext;
    if (gateType == "AND")
        andGate(globalDelta, permuteBitA, permuteBitB, AF, AT, BF, BT, ciphertext, gate0, gate1, k);
    else if (gateType == "XOR")  //free XOR
        xorGate(globalDelta, permuteBitA, permuteBitB, AF, BF, ciphertext);
    else if (gateType == "INV") //XOR with const
        invGate(permuteBitA, AF, AT, ciphertext, invConst);
    //insert output labels into wireLabels
    wireLabels[outputWires[0]] = make_tuple(ciphertext, util::vecXOR(ciphertext, globalDelta));
}


void
baseGarble::andGate(const vint &globalDelta, int permuteBitA, int permuteBitB, vint &A0,
                    vint &A1, vint &B0, vint &B1, vint &ciphertext,
                    vint &gate0, vint &gate1, int k) {
    ciphertext = hashXOR(A0, B0, k);
    gate1 = util::vecXOR(util::vecXOR(hashXOR(A0, B1, k), ciphertext), A0);
    gate0 = util::vecXOR(hashXOR(A1, B0, k), ciphertext);
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

vector<vint> baseGarble::eval(tuple<vector<labelPair>, vector<labelPair>, vector<labelPair>> F,
                              vector<vint> X, vector<string> f, const vint& invConst, int k) {
    //result vector
    vector<vint> Y;
    //garbled circuit
    auto garbledCircuit = get<0>(F);
    auto inputLabels = get<1>(F);
    auto outputLabels = get<2>(F);

    //get number of wires and gates from non-garbled circuit
    string &wireAndGates = f[0]; //number of wires and gates
    auto gatesAndWiresSplit = util::split(wireAndGates, ' ');
    int numberOfGates = stoi(gatesAndWiresSplit[0]);
    int numberOfWires = stoi(gatesAndWiresSplit[1]);
    int numberOfInputBits;
    getBits(f[1], numberOfInputBits);
    auto &outputs = f[2]; //number of outputs and how many bits each output is
    auto outputSplit = util::split(outputs, ' ');
    int numberOfOutputBits;
    getBits(f[2], numberOfOutputBits);

    //get input labels from X and put them in wireValues
    auto wireValues = vector< vint>(numberOfWires);
    for (int i = 0; i < X.size(); ++i) {
        wireValues[i] = X[i];
    }

    //check if the number of gates in the garbled circuit is the same as the number of gates in the non-garbled circuit
    if(garbledCircuit.size() != f.size()-3 & f.size()-3 != numberOfGates){
        cout << "garbledCircuit.size() != f.size()-3 != " << numberOfGates  << " != " << garbledCircuit.size() << " != " << f.size()-3 << endl;
        //exit(1);
    }

    //evaluate the garbled circuit
    for (int i = 0; i < garbledCircuit.size(); ++i) {
        //////////////////////// Getting out gate from string //////////////////////////
        auto &line = f[i+3];
        auto gateInfo = extractGate(line);              // "2 1 0 1 2 XOR"
        auto inputWires = get<0>(gateInfo);         // [ ..., 0, 1]
        auto outputWires = get<1>(gateInfo);        // [..., 1, ..., 2]
        auto gateType = get<2>(gateInfo);               // "XOR"
        //////////////////////// Evaluation of gate //////////////////////////

        //check if input labels for gate is calculated
        for (int inputWire : inputWires) {
            if(wireValues[inputWire].empty()){
                cout << "wireValues[inputWires[" << inputWire << "]].empty()" << endl;
                //exit(1);
            }
        }
        auto cipher = evalGate(invConst, k, garbledCircuit, wireValues, i, inputWires, gateType);

        //save output value in wireValues
        wireValues[outputWires[0]] = cipher;
        //save output value in Y if output wire
        if (outputWires[0] >= numberOfWires - numberOfOutputBits) {
            Y.push_back(cipher);
        }
    }
    return Y;
}

vint baseGarble::evalGate(const vint &invConst, int k,
                          const vector<labelPair> &garbledCircuit,
                          const vector<vint> &wireValues, int i, vector<int> inputWires,
                          const string& gateType) {//get input values
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
        auto colorBitA = (A[0]) & 1;
        auto colorBitB = (B[0]) & 1;
        //get gate ciphertexts
        vint gate0 = get<0>(garbledCircuit[i]);
        vint gate1 = get<1>(garbledCircuit[i]);
        //evaluate gate
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
    return cipher;
}

vector<int> baseGarble::decode(vector<labelPair> d, vector<vint> Y) {
    vector<int> y;
    for (int i = 0; i < Y.size(); ++i) {
        if (Y[i] == get<0>(d[i])) {
            y.push_back(0);
        } else if (Y[i] == get<1>(d[i])){
            y.push_back(1);
        } else {
            cout << "Could not decode as encrypted output was invalid" << endl;
        }
    }
    return y;
}

vint baseGarble::hashFunc(vint x, int k) {
    auto xstring = otUtil::printBitsetofVectorofUints(std::move(x));
    return util::hash_variable(xstring, k);
}

void baseGarble::getBits(string &f, int &numberOfInputBits) {
    numberOfInputBits= 0;
    auto split = util::split(f, ' ');
    int numberOfInputWires = stoi(split[0]);
    for (int i = 0; i < numberOfInputWires; ++i) {
        int numberOfBits = stoi(split[i + 1]);
        numberOfInputBits += numberOfBits;
    }
}

vint baseGarble::hashXOR(vint &labelA, vint &labelB, int k){
    return util::vecXOR(hashFunc(labelA, k), hashFunc(labelB, k));
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
