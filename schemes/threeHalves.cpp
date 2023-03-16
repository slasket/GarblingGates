//
// Created by svend on 001, 01-03-2023.
//

#include "threeHalves.h"
#include <set>
#include <utility>



tuple<Ftype, tuple<halfDelta, vector<tuple<halfLabels, int>>>, vector<vint>> threeHalves::garble(int k, vector<string> f) {
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
    Ftype F;
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

            halfLabels A0 = get<0>(A0AndPermuteBit);
            halfLabels B0 = get<0>(B0AndPermuteBit);
            vint leftHalf = util::vecXOR(get<0>(A0), get<0>(B0));
            vint rightHalf = util::vecXOR(get<1>(A0), get<1>(B0));
            int APermuteBit = get<1>(A0AndPermuteBit);
            int BPermuteBit = get<1>(B0AndPermuteBit);
            int permuteBitXOR = APermuteBit ^ BPermuteBit;

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
            auto A1 = util::halfLabelXOR(A0, delta);
            auto B1 = util::halfLabelXOR(B0, delta);
            auto [A0Left, A0Right] = A0;
            auto [B0Left, B0Right] = B0;
            auto [A1Left, A1Right] = A1;
            auto [B1Left, B1Right] = B1;
            auto [deltaLeft, deltaRight] = delta;

            //Calculate Zij
            auto Zij = calcZij(A0, B0, A1, B1, rVec, permuteBitA, permuteBitB, delta, i);

            //Prepend rVec to Z
            for (int l = 0; l < 4; ++l) {
                auto [left, right] = Zij[l];
                left = util::prependBitToVint(left, util::checkIthBit(rVec, l*2));
                right = util::prependBitToVint(right, util::checkIthBit(rVec, l*2+1));
                Zij[l] = {left, right};
            }
            //VInvZ is calculated per discord picture
            vector<vint> VInvZ(5);
            VInvZ[0] = get<0>(Zij[0]);
            VInvZ[1] = get<1>(Zij[0]);
            VInvZ[2] = util::vecXOR({get<0>(Zij[2]), get<1>(Zij[2]), get<0>(Zij[0]), get<1>(Zij[0])});
            VInvZ[3] = util::vecXOR({get<0>(Zij[1]), get<1>(Zij[1]),get<0>(Zij[0]), get<1>(Zij[0]) });
            VInvZ[4] = util::vecXOR(get<0>(Zij[3]), get<1>(Zij[3]));

            //VInvRpABDelta is calculated per discord picture
            vector<vint> VInvRpABDelta(5);
            VInvRpABDelta[0] = B0Left;
            VInvRpABDelta[1] = A0Right;
            VInvRpABDelta[2] = util::vecXOR(deltaRight, B0Left);
            VInvRpABDelta[3] = util::vecXOR(deltaLeft, A0Right);
            VInvRpABDelta[4] = {0};
            //Prepend 0's to all entries to make lenghts match (According to Lance)
            for (int l = 0; l < 5; ++l) {
                util::prependBitToVint(VInvRpABDelta[l], 0);
            }

            //HVecPrime is calculated per discord picture
            vint HA0 = util::hash_variable(util::halfLabelsToFullLabelString(A0) + to_string(((3*k)-3)), (k/2)+8);
            vint HA1 = util::hash_variable(util::halfLabelsToFullLabelString(A1) + to_string(((3*k)-3)), (k/2)+8);
            vint HB0 = util::hash_variable(util::halfLabelsToFullLabelString(B0) + to_string(((3*k)-2)), (k/2)+8);
            vint HB1 = util::hash_variable(util::halfLabelsToFullLabelString(B1) + to_string(((3*k)-2)), (k/2)+8);
            halfLabels A0xorB0 = {util::vecXOR(A0Left, B0Left), util::vecXOR(A0Right, B0Right)};
            vint HA0xorB0 = util::hash_variable(util::halfLabelsToFullLabelString(A0xorB0) + to_string(((3*k)-1)), (k/2)+8);

            //A0 XOR B0 XOR Delta = A0 XOR B1
            halfLabels A0xorB0xorDelta = {util::vecXOR(A0Left, B1Left), util::vecXOR(A0Right, B1Right)};
            vint HAxorB0xorDelta = util::hash_variable(util::halfLabelsToFullLabelString(A0xorB0xorDelta) + to_string(((3*k)-1)), (k/2)+8);
            //make sure hash vectors extra bit is only one bit
            HA0[HA0.size()-1] = HA0[HA0.size()-1] & 1;
            HA1[HA1.size()-1] = HA1[HA1.size()-1] & 1;
            HB0[HB0.size()-1] = HB0[HB0.size()-1] & 1;
            HB1[HB1.size()-1] = HB1[HB1.size()-1] & 1;
            HA0xorB0[HA0xorB0.size()-1] = HA0xorB0[HA0xorB0.size()-1] & 1;
            HAxorB0xorDelta[HAxorB0xorDelta.size()-1] = HAxorB0xorDelta[HAxorB0xorDelta.size()-1] & 1;
            vector<vint> HVecPrime(5);
            HVecPrime[0] = util::vecXOR(HA0, HA0xorB0);
            HVecPrime[1] = util::vecXOR(HB0, HA0xorB0);
            HVecPrime[2] = util::vecXOR(HA1, HA0);
            HVecPrime[3] = util::vecXOR(HB1, HB0);
            HVecPrime[4] = util::vecXOR(HAxorB0xorDelta, HA0xorB0);


            vector<vint> CG(5);
            vector<uint8_t> zVec(5);
            //XOR stuff together
            for (int l = 0; l < 5; ++l) {
                auto zConcatCG = util::vecXOR({VInvZ[l], VInvRpABDelta[l], HVecPrime[l]});
                zVec[l] = (zConcatCG[zConcatCG.size()-1]);
                zConcatCG.pop_back();
                CG[l] = zConcatCG;
            }
            uint64_t permuteBitCipher = CG[0][0] & 1;
            halfLabels cipherLabel = {CG[0], CG[1]}; //C left and C right
            if(permuteBitCipher == 1) {
                util::halfLabelXOR(cipherLabel, delta);
            }
            auto outputCipher = make_tuple(cipherLabel, permuteBitCipher);
            labelAndPermuteBitPairs[i-3] = outputCipher;
            F.emplace_back(CG[2], CG[3], CG[4], zVec);
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
    return make_tuple(F, e, d);
}

int threeHalves::encode(int e, int x) {
    return 0;
}

int threeHalves::eval(Ftype F, int X) {
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

vector<halfLabels> threeHalves::calcZij(halfLabels &A0, halfLabels &B0, halfLabels &A1, halfLabels &B1, vint &rVec, int permuteBitA, int permuteBitB, halfLabels &delta, int i){
    vector<halfLabels> Zij(4);
    int ctr = 0;
    auto [A0Left, A0Right] = A0;
    auto [B0Left, B0Right] = B0;
    auto [A1Left, A1Right] = A1;
    auto [B1Left, B1Right] = B1;

    for (int j = 0; j<=1; ++j) {
        for (int l = 0; l <=1; ++l) {
            int index1 = 4*i+2*l;
            int index2 = 4*i+2*l+1;

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
            halfLabels rS1AjBl = {{0}, {0}};
            halfLabels rS2AjBl = {{0}, {0}};
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
            ctr++;
        }
    }
    return Zij;
}
