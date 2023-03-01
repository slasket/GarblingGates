//
// Created by svend on 020, 20-02-2023.
//

#include <set>
#include "baseGarble.h"
#include "../util/util.h"

//k is security parameter and f is the function to be garbled with 3 lines of metadata
tuple<vector<tuple<vector<uint64_t>, vector<uint64_t>>>,
        vector<tuple<vector<uint64_t >, vector<uint64_t >>>,
                vector<tuple<vector<uint64_t >, vector<uint64_t >>>>
baseGarble::garble(int k, vector<string> f) {
    string &wireAndGates = f[0]; //number of wires and gates
    auto gatesAndWiresSplit = util::split(wireAndGates, ' ');
    int numberOfGates = stoi(gatesAndWiresSplit[0]);
    int numberOfWires = stoi(gatesAndWiresSplit[1]);
    int numberOfInputBits;
    getBits(f[1], numberOfInputBits);
    string &outputs = f[2]; //number of outputs and how many bits each output is
    auto outputSplit = util::split(outputs, ' ');
    int numberOfOutputBits;
    getBits(f[2], numberOfOutputBits);
    vector<::uint64_t> globalDelta = vector<uint64_t>(k);
    vector<tuple< vector<uint64_t>, vector<uint64_t>>> garbledCircuit = vector<tuple< vector<uint64_t>, vector<uint64_t>>>();
    vector<tuple<vector<uint64_t >, vector<uint64_t >>> encInputLabels = vector<tuple<vector<uint64_t >, vector<uint64_t >>>(numberOfInputBits);
    vector<tuple<vector<uint64_t >, vector<uint64_t >>> encOutputLabels = vector<tuple<vector<uint64_t >, vector<uint64_t >>>();

    //perform gate by gate garbling
    vector<tuple<vector<uint64_t>, vector<uint64_t>>> inputWiresLabels = vector<tuple<vector<uint64_t>, vector<uint64_t>>>(numberOfInputBits);
    auto deltaAndLabels = util::generateRandomLabels(k, globalDelta, inputWiresLabels);
    auto wireLabels = vector<tuple<vector<uint64_t>, vector<uint64_t>>>(numberOfWires);
    for (int i = 0; i < numberOfInputBits; ++i) {
        wireLabels[i] = inputWiresLabels[i];
    }

    for (int i = 3; i < f.size(); ++i) {
        //////////////////////// Getting out gate from string //////////////////////////
        string &line = f[i];
        auto gateInfo = extractGate(line);              // "2 1 0 1 2 XOR"
        auto inputWires = get<0>(gateInfo);         // [ ..., 0, 1]
        auto outputWires = get<1>(gateInfo);        // [..., 1, ..., 2]
        auto gateType = get<2>(gateInfo);               // "XOR"
        ////////////////////////////// Garbling gate ///////////////////////////////////
        //garble gate
        vector<tuple<vector<uint64_t>, vector<uint64_t>>> outputWiresLabels = vector<tuple<vector<uint64_t>, vector<uint64_t>>>();
        int input0;
        int input1;
        if(inputWires.size() == 1){
            input0 = inputWires[0];
            input1 = inputWires[0];
        }
        else{
            input0 = inputWires[0];
            input1 = inputWires[1];
        }


        //calculate permute bits
        int permuteBitA = (get<0>(wireLabels[input0])[0]) & 1;
        int permuteBitB = (get<0>(wireLabels[input1])[0]) & 1;
        vector<uint64_t> A0;
        vector<uint64_t> A1;
        vector<uint64_t> B0;
        vector<uint64_t> B1;
        if(permuteBitA == 1){
            A0 = (get<1>(wireLabels[input0]));
            A1 = (get<0>(wireLabels[input0]));
        } else {
            A0 = (get<0>(wireLabels[input0]));
            A1 = (get<1>(wireLabels[input0]));
        }
        if(permuteBitB == 1){
            B0 = (get<1>(wireLabels[input1]));
            B1 = (get<0>(wireLabels[input1]));
        } else {
            B0 = (get<0>(wireLabels[input1]));
            B1 = (get<1>(wireLabels[input1]));
        }
        vector<::uint64_t> ciphertext;
        vector<::uint64_t> gate0;
        vector<::uint64_t> gate1;
        if (gateType == "AND")
            andGate(globalDelta, permuteBitA, permuteBitB, A0, A1, B0, B1, ciphertext, gate0, gate1, k);
        else if (gateType == "XOR")  //free XOR
            xorGate(globalDelta, permuteBitA, permuteBitB, A0, A1, B0, B1, ciphertext, gate0, gate1, k);
        else if (gateType == "INV") //HANDLE INV AS AND WITH DOUBLE A INPUT //todo!!!! NOT SECURE !!!!
            invGate(globalDelta, permuteBitA, permuteBitB, A0, A1, B0, B1, ciphertext, gate0, gate1, k);

        //if(gateType == "INV"){
        //    if(globalDelta == util::vecXOR(gate0, gate1) ||
        //       globalDelta == util::vecXOR(gate0, ciphertext) ||
        //       globalDelta == util::vecXOR(gate1, ciphertext)){
        //        cout << "Hit shit the fan" << endl;
        //        exit(1);
        //    }
        //}

        wireLabels[outputWires[0]] = make_tuple(ciphertext, util::vecXOR(ciphertext, globalDelta));

        //create output F
        //wire labels
        auto gate = make_tuple(gate0, gate1);
        garbledCircuit.emplace_back(gate);

        //create output e
        if((input0 <= numberOfInputBits -1)){
            encInputLabels[input0] = inputWiresLabels[input0];
        }
        if((input1 <= numberOfInputBits -1)){
            encInputLabels[input1] = inputWiresLabels[input1];
        }

        //create output d
        if(outputWires[0] >= numberOfWires - numberOfOutputBits){
            auto outputLabel = wireLabels[outputWires[0]];
            encOutputLabels.emplace_back(outputLabel);
        }

    }
    //type tuple<vector<tuple<vector<int>, vector<uint64_t>, vector<uint64_t>>>, vector<tuple<vector<uint64_t >, vector<uint64_t >>>, vector<tuple<vector<uint64_t >, vector<uint64_t >>>>
    tuple<
            vector<tuple<vector<uint64_t>, vector<uint64_t>>>,
            vector<tuple<vector<uint64_t >, vector<uint64_t >>>,
                    vector<tuple<vector<uint64_t >, vector<uint64_t >>>>
            output = make_tuple(garbledCircuit, inputWiresLabels, encOutputLabels);
    return output;
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

tuple<vector<uint64_t>, vector<uint64_t>, vector<uint64_t>>
baseGarble::andGate(const vector<uint64_t> &globalDelta, int permuteBitA, int permuteBitB, vector<uint64_t> &A0,
                    vector<uint64_t> &A1, vector<uint64_t> &B0, vector<uint64_t> &B1, vector<uint64_t> &ciphertext,
                    vector<uint64_t> &gate0, vector<uint64_t> &gate1, int k) {
    ciphertext = XORHashpart(A0, B0, k);
    gate1 = util::vecXOR(util::vecXOR(XORHashpart(A0, B1, k), ciphertext), A0);
    gate0 = util::vecXOR(XORHashpart(A1, B0, k), ciphertext);
    if(permuteBitA == 1){
        gate1 = util::vecXOR(gate1, globalDelta);
    } else if(permuteBitB == 1){
        gate0 = util::vecXOR(gate0, globalDelta);
    }
    if(permuteBitA == 1 & permuteBitB == 1){
        ciphertext = util::vecXOR(ciphertext, globalDelta);
        gate0 = util::vecXOR(gate0, globalDelta);
    }
    return make_tuple(ciphertext, gate0, gate1);
}


tuple<vector<::uint64_t>, vector<uint64_t>, vector<uint64_t>>
baseGarble::invGate(const vector<uint64_t> &globalDelta, int permuteBitA, int permuteBitB, vector<uint64_t> &A0,
                    vector<uint64_t> &A1, vector<uint64_t> &B0, vector<uint64_t> &B1, vector<uint64_t> &ciphertext,
                    vector<uint64_t> &gate0, vector<uint64_t> &gate1, int k) {
    //auto bWire = vector<tuple<vector<uint64_t>, vector<uint64_t>>>(1);
    //auto res = util::generateRandomLabels(k, const_cast<vector<uint64_t> &>(globalDelta), bWire);
    //bWire = get<1>(res);
    //B0 = get<0>(bWire[0]);
    //B1 = get<1>(bWire[0]);
    //util::printUintVec(B1);

    B1 = vector<uint64_t>({874537361747324275,15596160569201595389});

    if(permuteBitA == 0){
        ciphertext = util::vecXOR(A1, B1);
        gate0 = B1;
    }   else{
        ciphertext = util::vecXOR(A0, B1);
        gate0 = B1;
    }
    //ciphertext = util::vecXOR(ciphertext, globalDelta);
    return make_tuple(ciphertext, gate0, gate1);
}

tuple<vector<::uint64_t>, vector<uint64_t>, vector<uint64_t>>
baseGarble::xorGate(const vector<uint64_t> &globalDelta, int permuteBitA, int permuteBitB, vector<uint64_t> &A0,
                    vector<uint64_t> &A1, vector<uint64_t> &B0, vector<uint64_t> &B1, vector<uint64_t> &ciphertext,
                    vector<uint64_t> &gate0, vector<uint64_t> &gate1, int k) {
    ciphertext = util::vecXOR(A0, B0);

    if((permuteBitA == 0 & permuteBitB == 1) | (permuteBitA == 1 & permuteBitB == 0) ){
        ciphertext = util::vecXOR(ciphertext,globalDelta);
    }
    return make_tuple(ciphertext, gate0, gate1);
}

vector<uint64_t> baseGarble::XORHashpart(vector<uint64_t> &labelA, vector<uint64_t> &labelB, int k){
    return util::vecXOR(HashFunction(labelA, k), HashFunction(labelB, k));
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

vector<vector<uint64_t>> baseGarble::encode(vector<tuple<vector<uint64_t>, vector<uint64_t>>> e, vector<int> x) {
    vector<vector<uint64_t>> X;
    for (int i = 0; i < x.size(); ++i) {
        if (x[i] == 0) {
            X.emplace_back(get<0>(e[i]));
        } else {
            X.emplace_back(get<1>(e[i]));
        }
    }
    return X;
}

vector<vector<uint64_t>> baseGarble::eval(tuple<vector<tuple<vector<uint64_t>, vector<uint64_t>>>,
        vector<tuple<vector<uint64_t>, vector<uint64_t>>>,
        vector<tuple<vector<uint64_t>, vector<uint64_t>>>> F, vector<vector<uint64_t>> X, vector<string> f, int k = 128) {

    vector<vector<uint64_t>> Y;
    vector<tuple<vector<uint64_t>, vector<uint64_t>>> garbledCircuit = get<0>(F);
    vector<tuple<vector<uint64_t>, vector<uint64_t>>> inputLabels = get<1>(F);
    vector<tuple<vector<uint64_t>, vector<uint64_t>>> outputLabels = get<2>(F);

    string &wireAndGates = f[0]; //number of wires and gates
    auto gatesAndWiresSplit = util::split(wireAndGates, ' ');
    int numberOfGates = stoi(gatesAndWiresSplit[0]);
    int numberOfWires = stoi(gatesAndWiresSplit[1]);
    int numberOfInputBits;
    getBits(f[1], numberOfInputBits);
    string &outputs = f[2]; //number of outputs and how many bits each output is
    auto outputSplit = util::split(outputs, ' ');
    int numberOfOutputBits;
    getBits(f[2], numberOfOutputBits);


    vector<vector<uint64_t>> wireValues = vector< vector<uint64_t>>(numberOfWires);
    for (int i = 0; i < X.size(); ++i) {
        wireValues[i] = X[i];
    }

    if(garbledCircuit.size() != f.size()-3 & f.size()-3 != numberOfGates){
        cout << "garbledCircuit.size() != f.size()-3 != " << numberOfGates  << " != " << garbledCircuit.size() << " != " << f.size()-3 << endl;
        //exit(1);
    }

    for (int i = 0; i < garbledCircuit.size(); ++i) {
        //////////////////////// Getting out gate from string //////////////////////////
        string &line = f[i+3];
        auto gateInfo = extractGate(line);              // "2 1 0 1 2 XOR"
        auto inputWires = get<0>(gateInfo);         // [ ..., 0, 1]
        auto outputWires = get<1>(gateInfo);        // [..., 1, ..., 2]
        auto gateType = get<2>(gateInfo);               // "XOR"
        //////////////////////// Evaluation of gate //////////////////////////

        for (int j = 0; j < inputWires.size(); ++j) { //check if input for gate is calculated
            if(wireValues[inputWires[j]].empty()){
                cout << "wireValues[inputWires[" << inputWires[j] << "]].empty()" << endl;
                //exit(1);
            }
        }

        int input0;
        int input1;
        if(gateType == "INV"){
            input0 = inputWires[0];
            input1 = inputWires[0];
        } else {
            input0 = inputWires[0];
            input1 = inputWires[1];
        }

        auto A = wireValues[input0];
        auto B = wireValues[input1];

        vector<uint64_t> cipher;

        auto colorBitA = (A[0]) & 1;
        auto colorBitB = (B[0]) & 1;

        if (gateType == "XOR"){
            cipher = util::vecXOR(A, B);
        } else
        if(gateType == "INV"){ //todo!!!! NOT SECURE !!!!
            vector<uint64_t> gate0 = vector<uint64_t>({874537361747324275,15596160569201595389});
            //vector<uint64_t> gate0 = get<0>(garbledCircuit[i]);
            cipher = util::vecXOR(A, gate0);
        } else
            if(gateType == "AND")
            { //AND/INV CASE
            vector<uint64_t> gate0 = get<0>(garbledCircuit[i]);
            vector<uint64_t> gate1 = get<1>(garbledCircuit[i]);

            if (colorBitA == 0 && colorBitB == 0) {
                cipher = XORHashpart(A, B, 128);
            } else if (colorBitA == 0 && colorBitB == 1) {
                cipher = util::vecXOR(util::vecXOR(XORHashpart(B, A, 128), gate1), A);
            } else if (colorBitA == 1 && colorBitB == 0) {
                cipher = util::vecXOR(XORHashpart(B, A, 128), gate0);
            } else if (colorBitA == 1 && colorBitB == 1) {
                cipher = util::vecXOR(util::vecXOR(util::vecXOR(XORHashpart(B, A, 128), gate0), gate1), A);
            }
        }
        wireValues[outputWires[0]] = cipher;

        if (outputWires[0] >= numberOfWires - numberOfOutputBits) {
            Y.push_back(cipher);
        }
    }
    return Y;
}

vector<int> baseGarble::decode(vector<tuple<vector<uint64_t>, vector<uint64_t>>> d, vector<vector<uint64_t>> Y) {
    vector<int> y;
    for (int i = 0; i < Y.size(); ++i) {
        if (Y[i] == get<0>(d[i])) {
            y.push_back(0);
        } else if (Y[i] == get<1>(d[i])){
            y.push_back(1);
        } else {
            cout << "Could not decode as encrypted output was invalid" << endl;
             //y.push_back(2);
            //exit(1);
        }
    }
    return y;
}

vector<uint64_t> baseGarble::HashFunction(vector<uint64_t> x, int k) {
    auto xstring = otUtil::printBitsetofVectorofUints(x);
    return util::hash_variable(xstring, k);
}


