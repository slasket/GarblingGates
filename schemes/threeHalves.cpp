//
// Created by svend on 001, 01-03-2023.
//

#include "threeHalves.h"
#include <set>
#include <utility>



tuple<int, tuple<halfDelta, vector<tuple<halfLabels, int>>>, vector<vint>> threeHalves::garble(int k, vector<string> f) {
    //get number of wires and gates
    auto &wireAndGates = f[0];
    auto gatesAndWiresSplit = util::split(wireAndGates, ' ');
    int numberOfWires = stoi(gatesAndWiresSplit[1]);

    //get number of input and output bits
    int numberOfInputBits;
    util::getBits(f[1], numberOfInputBits);
    int numberOfOutputBits;
    util::getBits(f[2], numberOfOutputBits);
    halfDelta delta = util::genDeltaHalves(k);
    vector<tuple<halfLabels, int>> labelAndPermuteBitPairs(numberOfWires);
    vector<vint> encryptedOutputLabels(numberOfOutputBits);
    for (int i = 0; i < numberOfInputBits; i++){
        auto label0 = util::genLabelHalves(k);
        auto permuteBit = (int)util::random_bitset<1>().to_ulong();
        labelAndPermuteBitPairs[i] = {label0, permuteBit};
    }
    for (int i = 3; i < f.size(); ++i) {
        //////////////////////// Getting out gate from string //////////////////////////
        auto &line = f[i];
        auto gateInfo = util::extractGate(line);              // "2 1 0 1 2 XOR"
        auto inputWires = get<0>(gateInfo);         // [ ..., 0, 1]
        auto outputWires = get<1>(gateInfo);        // [..., 1, ..., 2]
        auto gateType = get<2>(gateInfo);               // "XOR"

        tuple<halfLabels, int> A0AndPermuteBit;
        tuple<halfLabels, int> B0AndPermuteBit;
        if(gateType == "XOR") {
            A0AndPermuteBit = labelAndPermuteBitPairs[inputWires[0]];
            B0AndPermuteBit = labelAndPermuteBitPairs[inputWires[1]];

            auto A0 = get<0>(A0AndPermuteBit);
            auto B0 = get<0>(B0AndPermuteBit);
            auto leftHalf = util::vecXOR(get<0>(A0), get<0>(B0));
            auto rightHalf = util::vecXOR(get<1>(A0), get<1>(B0));
            auto APermuteBit = get<1>(A0AndPermuteBit);
            auto BPermuteBit = get<1>(B0AndPermuteBit);
            auto permuteBitXOR = APermuteBit ^ BPermuteBit;

            halfLabels outputLabel = {leftHalf, rightHalf};
            auto outputCipher = make_tuple(outputLabel, permuteBitXOR);

            labelAndPermuteBitPairs[i-3] = outputCipher;
        }
        else if( gateType == "AND"){
            A0AndPermuteBit = labelAndPermuteBitPairs[inputWires[0]];
            B0AndPermuteBit = labelAndPermuteBitPairs[inputWires[1]];
            auto [A0, permuteBitA] = A0AndPermuteBit;
            auto [B0, permuteBitB] = B0AndPermuteBit;
            auto rVec = sampleR(permuteBitA, permuteBitB);

            //Compute A1 and B1
            auto [A0Left, A0Right] = A0;
            auto [B0Left, B0Right] = B0;
            auto [deltaLeft, deltaRight] = delta;
            auto A1Left = util::vecXOR(A0Left, deltaLeft);
            auto A1Right = util::vecXOR(A0Right, deltaRight);
            auto B1Left = util::vecXOR(B0Left, deltaLeft);
            auto B1Right = util::vecXOR(B0Right, deltaRight);
            halfLabels A1 = make_tuple(A1Left, A1Right);
            halfLabels B1 = make_tuple(B1Left, B1Right);
            vector<halfLabels> Zij(4);
            int ctr = 0;
            for (int j = 0; j<=1; ++j) {
                for (int l = 0; l <=1; ++l) {
                    int index1 = 4*i+2*l;
                    int index2 = 4*1+2*l+1;

                    int r1 = util::checkIthBit(rVec, index1);
                    int r2 = util::checkIthBit(rVec, index2);
                    vint ALeft = A0Left;
                    vint ARight = A0Right;
                    vint BLeft = B0Left;
                    vint BRight = B0Right;
                    if(i == 1){
                        ALeft = A1Left;
                        ARight = A1Right;
                    }
                    if(j == 1){
                        BLeft = B1Left;
                        BRight = B1Right;
                    }
                    halfLabels rS1AjBl, rS2AjBl;
                    if(r1 == 1){
                        /*
                         * Calculate S1 * [Aj Bl]
                         */
                        auto left = util::vecXOR(util::vecXOR(BLeft, ARight), ALeft);
                        auto right = util::vecXOR(BLeft, ALeft);
                        rS1AjBl = {left, right};



                    }
                    if(r2 == 1){
                        /*
                         * Calculate S2 * [Ai Bj]
                         */
                        auto left = util::vecXOR(BRight, ALeft);
                        auto right = util::vecXOR(util::vecXOR(BRight, BLeft), ARight);
                        rS2AjBl = {left, right};
                    }

                    halfLabels Yij = util::halfLabelXOR(rS1AjBl, rS2AjBl);
                    bool includeDelta = (permuteBitA ^ j) & (permuteBitB ^ l);
                    if(includeDelta) {
                        Yij = util::halfLabelXOR(Yij, delta);
                    }

                    Zij[ctr] = Yij;
                }
                //VInvRpABDelta is calculated per discord picture
                vector<vint> VInvRpABDelta(5);
                VInvRpABDelta[0] = B0Left;
                VInvRpABDelta[1] = A0Right;
                VInvRpABDelta[2] = util::vecXOR(deltaRight, B0Left);
                VInvRpABDelta[3] = util::vecXOR(deltaLeft, A0Right);
                VInvRpABDelta[4] = {0};

                //VInvZ is calculated per discord picture
                vector<vint> VInvZ(5);
                VInvZ[0] = get<0>(Zij[0]);
                VInvZ[1] = get<1>(Zij[0]);
                auto temp = util::vecXOR(get<0>(Zij[2]), get<1>(Zij[2]));
                auto temp1 = util::vecXOR(get<0>(Zij[0]), get<1>(Zij[0]));
                VInvZ[2] = util::vecXOR(temp, temp1);
                temp = util::vecXOR(get<0>(Zij[1]), get<1>(Zij[1]));
                temp1 = util::vecXOR(get<0>(Zij[0]), get<1>(Zij[0]));
                VInvZ[3] = util::vecXOR(temp, temp1);
                VInvZ[4] = util::vecXOR(get<0>(Zij[3]), get<1>(Zij[3]));

                //HVecPrime is calculated per discord picture
                vint H0 = util::hash_variable(util::halfLabelsToFullLabelString(A0) + to_string(((3*k)-3)), (k/2)+1);
                vint H1 = util::hash_variable(util::halfLabelsToFullLabelString(A1) + to_string(((3*k)-3)), (k/2)+1);
                vint H2 = util::hash_variable(util::halfLabelsToFullLabelString(B0) + to_string(((3*k)-2)), (k/2)+1);
                vint H3 = util::hash_variable(util::halfLabelsToFullLabelString(B1) + to_string(((3*k)-2)), (k/2)+1);
                halfLabels A0xorB0 = {util::vecXOR(A0Left, B0Left), util::vecXOR(A0Right, B0Right)};
                vint H4 = util::hash_variable(util::halfLabelsToFullLabelString(A0xorB0) + to_string(((3*k)-1)), (k/2)+1);

                //A0 XOR B0 XOR Delta = A0 XOR B1
                halfLabels A0xorB1 = {util::vecXOR(A0Left, B1Left), util::vecXOR(A0Right, B1Right)};
                vint H5 = util::hash_variable(util::halfLabelsToFullLabelString(A0xorB1) + to_string(((3*k)-1)), (k/2)+1);
            }
        }
        else if(gateType == "INV") {

        }
        else {
            cout << "The fuck is this" << endl;
            exit(2);
        }
    }






    for (int i = numberOfWires - numberOfOutputBits; i < numberOfWires; ++i) {
        auto labelAndPermuteBit = labelAndPermuteBitPairs[i];
        auto halfLabels = get<0>(labelAndPermuteBit);
        auto permuteBit = get<1>(labelAndPermuteBit);
        auto label0 = get<0>(halfLabels);
        auto label1 = get<1>(halfLabels);
        label0.insert(label0.end(), label1.begin(), label1.end());
        int randomJ = (int)util::random_bitset<1>().to_ulong();
        int jXORPermuteBit = randomJ ^ permuteBit;
        if(jXORPermuteBit == 1){
            auto deltaLeft = get<0>(delta);
            auto deltaRight = get<1>(delta);
            deltaLeft.insert(deltaLeft.end(), deltaRight.begin(), deltaRight.end());
            label0 = util::vecXOR(label0, deltaLeft);
        }
        //Calculate tweak as 3*|f| + 2k
        int tweak = (numberOfWires * 3) + (2 * i);
        auto encryptedOutputLabel = hashPrime(label0, k, tweak);
        encryptedOutputLabels[i-(numberOfWires-1)] = encryptedOutputLabel;
    }

    auto e = make_tuple(delta, labelAndPermuteBitPairs);
    auto d =  encryptedOutputLabels;

    //Return F, e, d. //todo create F
    return make_tuple(0, e, d);
}

int threeHalves::encode(int e, int x) {
    return 0;
}

int threeHalves::eval(int F, int X) {
    return 0;
}

int threeHalves::decode(int d, int Y) {
    return 0;
}

vector<uint64_t> threeHalves::hashPrime(const vint& input, int k, int tweak) {
    string inputString = util::uintVec2Str(input);
    auto leftHalfHash = util::hash_variable( inputString + to_string(tweak), k);
    auto rightHalfHash = util::hash_variable( inputString + to_string(tweak+1), k);
    auto numberOfUIntBlocks = leftHalfHash.size();
    vint msbOfLeftHalf(numberOfUIntBlocks/2);
    vint msbOfRightHalf(numberOfUIntBlocks/2);
    for (int i = 0; i < numberOfUIntBlocks/2; ++i) {
        msbOfLeftHalf[i] = leftHalfHash[i];
        msbOfRightHalf[i] = rightHalfHash[i];
    }
    msbOfLeftHalf.insert(msbOfLeftHalf.end(), msbOfRightHalf.begin(), msbOfRightHalf.end());
    return msbOfLeftHalf;
}

vint threeHalves::sampleR(int permuteBitA, int permuteBitB) {
    int a = 1 - permuteBitA;
    int b = 1 - permuteBitB;
    /*
     * RaBar is seen as a single 8bit integer
     * To make [0] correspond to the 0'th bit in the integer the binary number is reversed
     * As such the following matrix is written as the binary number 01101100 which is 108 as an int.
     * [0 0
     *  1 1
     *  0 1
     *  1 0]
     */
    uint8_t RaBar = 108;
    uint8_t RbBar = 180;

    uint8_t R$ = 0;
    /*
     * R$ is any linear combination of
     * [1 0
     *  1 0
     *  1 0
     *  1 0] = 85
     *  and
     * [0 1
     *  0 1
     *  0 1
     *  0 1] = 170
     */
    auto twoRandomBits = util::random_bitset<2>();
    auto bit1 = twoRandomBits[0];
    auto bit2 = twoRandomBits[1];
    R$ = bit1 * 85 + bit2 * 170;
    uint64_t rVec = (a*RaBar) ^ (b*RbBar) ^ R$;
    return {rVec};
}
