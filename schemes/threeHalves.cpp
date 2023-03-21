//
// Created by svend on 001, 01-03-2023.
//

#include "threeHalves.h"
#include <set>
#include <utility>



tuple<Ftype, tuple<halfDelta, vector<tuple<halfLabels, int>>>, vector<halfLabels>, halfLabels> threeHalves::garble(int k, vector<string> f) {
    //get number of wires and gates
    auto &wireAndGates = f[0];
    auto gatesAndWiresSplit = util::split(wireAndGates, ' ');
    int numberOfWires = stoi(gatesAndWiresSplit[1]);

    //get number of input and output bits
    int numberOfInputBits;
    util::getBits(f[1], numberOfInputBits);
    int numberOfOutputBits;
    util::getBits(f[2], numberOfOutputBits);
    halfDelta delta = genDeltaHalves(k);
    vector<tuple<halfLabels, int>> labelAndPermuteBitPairs(numberOfWires);
    vector<tuple<halfLabels, int>> inputLabelAndPermuteBitPairs(numberOfInputBits); //testing
    vector<halfLabels> encryptedOutputLabels(numberOfOutputBits);
    for (int i = 0; i < numberOfInputBits; i++){
        auto label0 = genLabelHalves(k);
        auto permuteBit = (int)util::random_bitset<1>().to_ulong();
        labelAndPermuteBitPairs[i] = {label0, permuteBit};
        inputLabelAndPermuteBitPairs[i] = {label0, permuteBit};
    }

    //invConst
    halfLabels invConst = genLabelHalves(k);

    Ftype F(numberOfWires);
    for (int i = 3; i < f.size(); ++i) {
        //////////////////////// Getting out gate from string //////////////////////////
        auto &line = f[i];
        auto gateInfo = util::extractGate(line);              // "2 1 0 1 2 XOR"
        auto inputWires = get<0>(gateInfo);             // [ ..., 0, 1]
        auto outputWires = get<1>(gateInfo);                    // [..., 1, ..., 2]
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

            labelAndPermuteBitPairs[outputWires[0]] = outputCipher;
        }
        else if(gateType == "INV") {
            A0AndPermuteBit = labelAndPermuteBitPairs[inputWires[0]];

            halfLabels A0 = get<0>(A0AndPermuteBit);
            vint leftHalf = util::vecXOR(get<0>(A0), get<0>(invConst));
            vint rightHalf = util::vecXOR(get<1>(A0), get<1>(invConst));
            int APermuteBit = get<1>(A0AndPermuteBit);
            int icPermuteBit = get<0>(invConst)[0] & 1;
            int permuteBitXOR = APermuteBit ^ icPermuteBit;

            halfLabels outputLabel = {leftHalf, rightHalf};
            auto outputCipher = make_tuple(outputLabel, 1- permuteBitXOR);

            labelAndPermuteBitPairs[outputWires[0]] = outputCipher;
        }
        else if( gateType == "AND"){
            A0AndPermuteBit = labelAndPermuteBitPairs[inputWires[0]];
            B0AndPermuteBit = labelAndPermuteBitPairs[inputWires[1]];
            auto [A0, permuteBitA] = A0AndPermuteBit;
            auto [B0, permuteBitB] = B0AndPermuteBit;
            auto rVec = sampleR(permuteBitA, permuteBitB);

            //cout << "rVec: " << endl;
            //util::printUintVec(rVec);


            //Compute A1 and B1
            auto A1 = util::halfLabelXOR(A0, delta);
            auto B1 = util::halfLabelXOR(B0, delta);
            auto [A0Left, A0Right] = A0;
            auto [B0Left, B0Right] = B0;
            auto [A1Left, A1Right] = A1;
            auto [B1Left, B1Right] = B1;
            auto [deltaLeft, deltaRight] = delta;

            //Calculate Zij
            auto Zij = calcZij(A0, B0, A1, B1, rVec, permuteBitA, permuteBitB, delta);

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
            VInvZ[4] = util::vecXOR(get<0>(Zij[3]), get<0>(Zij[2]));

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
            vint HA0xorB0xorDelta = util::hash_variable(util::halfLabelsToFullLabelString(A0xorB0xorDelta) + to_string(((3 * k) - 1)), (k / 2) + 8);
            //make sure hash vectors extra bit is only one bit
            HA0[HA0.size()-1] = HA0[HA0.size()-1] & 1;
            HA1[HA1.size()-1] = HA1[HA1.size()-1] & 1;
            HB0[HB0.size()-1] = HB0[HB0.size()-1] & 1;
            HB1[HB1.size()-1] = HB1[HB1.size()-1] & 1;
            HA0xorB0[HA0xorB0.size()-1] = HA0xorB0[HA0xorB0.size()-1] & 1;
            HA0xorB0xorDelta[HA0xorB0xorDelta.size() - 1] = HA0xorB0xorDelta[HA0xorB0xorDelta.size() - 1] & 1;

            //cout << "GARBLE HASHES" << endl;
            //util::printUintVec(HA0);
            //util::printUintVec(HA1);
            //util::printUintVec(HB0);
            //util::printUintVec(HB1);
            //util::printUintVec(HA0xorB0);
            //util::printUintVec(HA0xorB0xorDelta);

            vector<vint> HVecPrime(5);
            HVecPrime[0] = util::vecXOR(HA0, HA0xorB0);
            HVecPrime[1] = util::vecXOR(HB0, HA0xorB0);
            HVecPrime[2] = util::vecXOR(HA1, HA0);
            HVecPrime[3] = util::vecXOR(HB1, HB0);
            HVecPrime[4] = util::vecXOR(HA0xorB0xorDelta, HA0xorB0);


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
            //int APermuteBit = get<1>(A0AndPermuteBit);
            //int BPermuteBit = get<1>(B0AndPermuteBit);
            //int permuteBitCipherCheck = APermuteBit ^ BPermuteBit;
            //if(permuteBitCipher != permuteBitCipherCheck) {
            //    cout << "GARBLE PERMUTE BIT MISMATCH" << endl;
            //    exit(2);
            //}
            halfLabels cipherLabel = {CG[0], CG[1]}; //C left and C right
            if(permuteBitCipher == 1) {
                cipherLabel = util::halfLabelXOR(cipherLabel, delta);
            }
            auto outputCipher = make_tuple(cipherLabel, permuteBitCipher);
            //auto cipherLabelStr = util::halfLabelsToFullLabelString(cipherLabel);
            //cout << "GARBLE CIPHER LABEL: " << endl;
            //cout << cipherLabelStr << endl;
            labelAndPermuteBitPairs[outputWires[0]] = outputCipher;
            F[outputWires[0]] = {CG[2], CG[3], CG[4], zVec};
            if(outputWires[0] >= numberOfWires - numberOfOutputBits) {
                //cout << "Outputwires input wire permute bits" << endl;
                //cout << "permuteBitA: " << permuteBitA << endl;
                //cout << "permuteBitB: " << permuteBitB << endl;
            }
        }
        else {
            cout << "The fuck is this" << endl;
            //exit(2);
        }
    }






    for (int i = numberOfWires - numberOfOutputBits; i < numberOfWires; ++i) {
        auto labelAndPermuteBit = labelAndPermuteBitPairs[i];
        auto [label0, label1] = get<0>(labelAndPermuteBit);
        auto permuteBit = get<1>(labelAndPermuteBit);
        label0.insert(label0.end(), label1.begin(), label1.end());
        vint encryptedOutputLabel0;
        vint encryptedOutputLabel1;

        //Calculate tweak as 3*|f| + 2k
        int tweak = (numberOfWires * 3) + (2 * i);
        encryptedOutputLabel0 = hashPrime(label0, k, tweak);
        auto deltaLeft = get<0>(delta);
        auto deltaRight = get<1>(delta);
        deltaLeft.insert(deltaLeft.end(), deltaRight.begin(), deltaRight.end());
        label0 = util::vecXOR(label0, deltaLeft);
        encryptedOutputLabel1 = hashPrime(label0, k, tweak);
        if(permuteBit == 1){
            encryptedOutputLabels[(i)-(numberOfWires-numberOfOutputBits)] = make_tuple(encryptedOutputLabel1, encryptedOutputLabel0);
        }
        else {
            encryptedOutputLabels[(i)-(numberOfWires-numberOfOutputBits)] = make_tuple(encryptedOutputLabel0, encryptedOutputLabel1);
        }
    }

    auto e = make_tuple(delta, inputLabelAndPermuteBitPairs);
    auto d =  encryptedOutputLabels;

    //Return F, e, d. //todo create F
    return make_tuple(F, e, d, invConst);
}

vector<halfLabels> threeHalves::encode(tuple<halfDelta, vector<tuple<halfLabels, int>>> e, vector<int> x) {
    vector<halfLabels> X;
    halfDelta delta = get<0>(e);
    vector<tuple<halfLabels, int>> labelAndPermuteBitPairs = get<1>(e);
    for (int i = 0; i < labelAndPermuteBitPairs.size(); ++i) {
        auto labelAndPermuteBit = labelAndPermuteBitPairs[i];
        auto halfLabels = get<0>(labelAndPermuteBit);
        auto permuteBit = get<1>(labelAndPermuteBit);
        permuteBit = permuteBit ^ x[i];
        auto [label0, label1] = halfLabels;
        //label0.insert(label0.end(), label1.begin(), label1.end());
        if(permuteBit == 1){
            auto deltaLeft = get<0>(delta);
            auto deltaRight = get<1>(delta);
            //deltaLeft.insert(deltaLeft.end(), deltaRight.begin(), deltaRight.end());
            label0 = util::vecXOR(label0, deltaLeft);
            label1 = util::vecXOR(label1, deltaRight);
        }
        halfLabels = {label0, label1};
        X.emplace_back(halfLabels);
    }
    return X;
}

vector<halfLabels> threeHalves::eval(Ftype F, vector<halfLabels> X, vector<string> f, int k, const halfLabels& invConst) {
    auto &wireAndGates = f[0];
    auto gatesAndWiresSplit = util::split(wireAndGates, ' ');
    int numberOfWires = stoi(gatesAndWiresSplit[1]);

    //get number of input and output bits
    int numberOfInputBits;
    util::getBits(f[1], numberOfInputBits);
    int numberOfOutputBits;
    util::getBits(f[2], numberOfOutputBits);

    vector<halfLabels> Y(numberOfOutputBits);
    vector<tuple<halfLabels, int>> labelAndPermuteBitPairs(numberOfWires);
    //fill with encrypted input X
    for (int i = 0; i < X.size(); ++i) {
        auto permuteBit = (get<0>(X[i])[0]) & 1;
        labelAndPermuteBitPairs[i] = {X[i], permuteBit};
    }
    for (int i = 3; i < f.size(); ++i) {
        //////////////////////// Getting out gate from string //////////////////////////
        auto &line = f[i];
        auto gateInfo = util::extractGate(line);              // "2 1 0 1 2 XOR"
        auto inputWires = get<0>(gateInfo);         // [ ..., 0, 1]
        auto outputWires = get<1>(gateInfo);        // [..., 1, ..., 2]
        auto gateType = get<2>(gateInfo);               // "XOR"

        auto [A, ApermuteBit] = labelAndPermuteBitPairs[inputWires[0]];
        auto [B, BpermuteBit] = labelAndPermuteBitPairs[inputWires[0]];
        if(inputWires.size()==2)  {
            auto [C, CpermuteBit] = labelAndPermuteBitPairs[inputWires[1]];
            B = C;
            BpermuteBit = CpermuteBit;
        }
        auto [Al, Ar] = A;
        auto [Bl, Br] = B;
        //if(ApermuteBit != (Al[0] & 1)){
        //    exit(2);
        //}
        //ApermuteBit = 1- ApermuteBit;
        //BpermuteBit = 1- BpermuteBit;

        //cout << "Eval permute bits" << endl;
        //cout << "ApermuteBit: " << ApermuteBit << endl;
        //cout << "BpermuteBit: " << BpermuteBit << endl;

        halfLabels Ek;
        uint64_t EpermuteBit;
        if (gateType == "XOR") {
            auto cl = util::vecXOR(Al, Bl);
            auto cr = util::vecXOR(Ar, Br);
            Ek = {cl, cr};
            EpermuteBit = (cl[0]) & 1;
        } else if (gateType == "INV"){
            auto [il, ir] = invConst;
            auto cl = util::vecXOR(Al, il);
            auto cr = util::vecXOR(Ar, ir);
            Ek = {cl, cr};
            EpermuteBit = (cl[0]) & 1;
        }
        else if (gateType == "AND") {
            auto gate = F[outputWires[0]];
            //big math
            vector<uint8_t> z = get<3>(gate);
            auto g0 = get<0>(gate);
            auto g1 = get<1>(gate);
            auto g2 = get<2>(gate);


            vector<vint> gVec = {{0}, {0}, g0, g1, g2};
            //append z[j] to g0, g1, g2
            for (int j = 0; j < 5; ++j) {
                gVec[j].emplace_back(z[j]); //z[0] and z[1] are for cl cr
            }

            // Vij (zk || [G])
            vector<vint> vzgVec;
            switch (ApermuteBit * 2 + BpermuteBit) {
                case 0:
                    vzgVec = {util::vecXOR({gVec[0]}),
                              util::vecXOR({gVec[1]})};
                    break;
                case 1:
                    vzgVec = {util::vecXOR({gVec[4], gVec[0]}),
                              util::vecXOR({gVec[4], gVec[3], gVec[1]})};
                    break;
                case 2:
                    vzgVec = {util::vecXOR({gVec[4], gVec[2], gVec[0]}),
                              util::vecXOR({gVec[4], gVec[1]})};
                    break;
                case 3:
                    vzgVec = {util::vecXOR({gVec[2], gVec[0]}),
                              util::vecXOR({gVec[3], gVec[1]})};
                    break;
            }

            //cout << "vzgVec last blocks: " << endl;
            //vector<vint> vzgVec0 = {util::vecXOR({gVec[0]}),
            //                        util::vecXOR({gVec[1]})};
            //vector<vint> vzgVec1 = {util::vecXOR({gVec[4], gVec[0]}),
            //                        util::vecXOR({gVec[4], gVec[3], gVec[1]})};
            //vector<vint> vzgVec2 =  {util::vecXOR({gVec[4], gVec[2], gVec[0]}),
            //                         util::vecXOR({gVec[4], gVec[1]})};
            //vector<vint> vzgVec3 ={util::vecXOR({gVec[2], gVec[0]}),
            //                       util::vecXOR({gVec[3], gVec[1]})};
            //util::printUintVec( vzgVec0[0]);
            //util::printUintVec( vzgVec0[vzgVec1.size()-1]);
            //util::printUintVec( vzgVec1[0]);
            //util::printUintVec( vzgVec1[vzgVec1.size()-1]);
            //util::printUintVec( vzgVec2[0]);
            //util::printUintVec( vzgVec2[vzgVec2.size()-1]);
            //util::printUintVec( vzgVec3[0]);
            //util::printUintVec( vzgVec3[vzgVec3.size()-1]);

            //Hashes
            vector<vint> hVec;

            vint HA = util::hash_variable(util::halfLabelsToFullLabelString(A) + to_string(((3 * k) - 3)), (k / 2) + 8);
            vint HB = util::hash_variable(util::halfLabelsToFullLabelString(B) + to_string(((3 * k) - 2)), (k / 2) + 8);
            halfLabels AxorB = {util::vecXOR(Al, Bl), util::vecXOR(Ar, Br)};
            vint HAxorB = util::hash_variable(util::halfLabelsToFullLabelString(AxorB) + to_string(((3*k)-1)), (k/2)+8);

            //make sure hash vectors extra bit is only one bit
            HA[HA.size()-1] = HA[HA.size()-1] & 1;
            HB[HB.size()-1] = HB[HB.size()-1] & 1;
            HAxorB[HAxorB.size()-1] = HAxorB[HAxorB.size()-1] & 1;

            //cout << "EVAL HASHES:" << endl;
            //util::printUintVec(HA);
            //util::printUintVec(HB);
            //util::printUintVec(HAxorB);

            hVec = {util::vecXOR(HA, HAxorB),       // { HA,  0, HAB },
                    util::vecXOR(HB, HAxorB)};      // {  0, HB, HAB }

            // rVec || Xij
            vector<vint> rxVec = {util::vecXOR(vzgVec[0], hVec[0]),     // { rl, Xl },
                                  util::vecXOR(vzgVec[1], hVec[1])};    // { rr, Xr }
            //vector<vint> rxVec0 = {util::vecXOR(vzgVec0[2], hVec[0]),     // { rl, Xl },
            //                       util::vecXOR(vzgVec0[3], hVec[1])};    // { rr, Xr }
            //vector<vint> rxVec1 = {util::vecXOR(vzgVec1[2], hVec[0]),     // { rl, Xl },
            //                      util::vecXOR(vzgVec1[3], hVec[1])};    // { rr, Xr }
            //vector<vint> rxVec2 = {util::vecXOR(vzgVec2[2], hVec[0]),     // { rl, Xl },
            //                      util::vecXOR(vzgVec2[3], hVec[1])};    // { rr, Xr }
            //vector<vint> rxVec3 = {util::vecXOR(vzgVec3[2], hVec[0]),     // { rl, Xl },
            //                      util::vecXOR(vzgVec3[3], hVec[1])};    // { rr, Xr }

            // rVec
            vector<uint64_t> rVec = {rxVec[0][rxVec[0].size()-1],
                                     rxVec[1][rxVec[1].size()-1]};
            //vector<uint64_t> rVec0 = {rxVec0[0][rxVec0[0].size()-1],
            //                          rxVec0[1][rxVec0[1].size()-1]};
            //vector<uint64_t> rVec1 = {rxVec1[0][rxVec1[0].size()-1],
            //                         rxVec1[1][rxVec1[1].size()-1]};
            //vector<uint64_t> rVec2 = {rxVec2[0][rxVec2[0].size()-1],
            //                         rxVec2[1][rxVec2[1].size()-1]};
            //vector<uint64_t> rVec3 = {rxVec3[0][rxVec3[0].size()-1],
            //                         rxVec3[1][rxVec3[1].size()-1]};

            //cout << "EVAL RVECs:" << endl;
            //util::printUintVec(rVec);
            //util::printUintVec(rVec0);
            //util::printUintVec(rVec1);
            //util::printUintVec(rVec2);
            //util::printUintVec(rVec3);

            //Xij
            halfLabels Xij = {{rxVec[0].begin(), rxVec[0].end()-1},
                                      {rxVec[1].begin(), rxVec[1].end()-1}};
            //if(get<0>(Xij).empty()) exit(2);

            //RijpAB
            auto RijAB = decodeR(rVec, A, B, ApermuteBit, BpermuteBit);
            auto RijAB1 = decodeR({0,0}, A, B, ApermuteBit, BpermuteBit);
            auto RijAB2 = decodeR({0,1}, A, B, ApermuteBit, BpermuteBit);
            auto RijAB3 = decodeR({1,0}, A, B, ApermuteBit, BpermuteBit);
            auto RijAB4 = decodeR({1,1}, A, B, ApermuteBit, BpermuteBit);


            //Ek
            Ek = util::halfLabelXOR(Xij, RijAB);
            //auto Ek1 = util::halfLabelXOR(Xij, RijAB1);
            //auto Ek2 = util::halfLabelXOR(Xij, RijAB2);
            //auto Ek3 = util::halfLabelXOR(Xij, RijAB3);
            //auto Ek4 = util::halfLabelXOR(Xij, RijAB4);
//
            //auto Ek1str = util::halfLabelsToFullLabelString(Ek1);
            //auto Ek2str = util::halfLabelsToFullLabelString(Ek2);
            //auto Ek3str = util::halfLabelsToFullLabelString(Ek3);
            //auto Ek4str = util::halfLabelsToFullLabelString(Ek4);
            //cout << "EVAL Ek:" << endl;
            //cout << Ek1str << endl;
            //cout << Ek2str << endl;
            //cout << Ek3str << endl;
            //cout << Ek4str << endl;

            //Calculate tweak as 3*|f| + 2k
            //int wireIndex = (numberOfWires-outputWires[0]-1);
            //int tweak = 13;

            //auto [Ekl, Ekr] = Ek1;
            //Ekl.insert(Ekl.end(), Ekr.begin(), Ekr.end());
            //auto Ekv = Ekl;
            ////H'(Ek, tweak)
            //auto EkHash1 = hashPrime(Ekv, k, tweak);
            //cout << "EVAL EkHash1: " << util::uintVec2Str(EkHash1) << endl;
            //auto [Ekl2, Ekr2] = Ek2;
            //Ekl2.insert(Ekl2.end(), Ekr2.begin(), Ekr2.end());
            //Ekv = Ekl2;
            ////H'(Ek, tweak)
            //auto EkHash2 = hashPrime(Ekv, k, tweak);
            //cout << "EVAL EkHash2: " << util::uintVec2Str(EkHash2) << endl;
            //auto [Ekl3, Ekr3] = Ek3;
            //Ekl3.insert(Ekl3.end(), Ekr3.begin(), Ekr3.end());
            //Ekv = Ekl3;
            ////H'(Ek, tweak)
            //auto EkHash3 = hashPrime(Ekv, k, tweak);
            //cout << "EVAL EkHash3: " << util::uintVec2Str(EkHash3) << endl;
            //auto [Ekl4, Ekr4] = Ek4;
            //Ekl4.insert(Ekl4.end(), Ekr4.begin(), Ekr4.end());
            //Ekv = Ekl4;
            ////H'(Ek, tweak)
            //auto EkHash4 = hashPrime(Ekv, k, tweak);
            //cout << "EVAL EkHash4: " << util::uintVec2Str(EkHash4) << endl;


            EpermuteBit = get<0>(Ek)[0] & 1;
            //if(!decode(d, {Ek}, f, k).empty() || decode(d, {Ek}, f, k)[0] == 0){
            //    cout << "EVAL Ek DECODED TO 0" << endl;
            //}
        }
        labelAndPermuteBitPairs[outputWires[0]] = {Ek, EpermuteBit};

        if(outputWires[0] >= numberOfWires - numberOfOutputBits){
            Y[outputWires[0]-(numberOfWires-numberOfOutputBits)] = (Ek);
        }
    }
    return Y;
}

vector<int> threeHalves::decode(vector<halfLabels> d, vector<halfLabels> Y, vector<string> f, int k) {
    auto &wireAndGates = f[0];
    auto gatesAndWiresSplit = util::split(wireAndGates, ' ');
    int numberOfWires = stoi(gatesAndWiresSplit[1]);
    vector<int> y;
    for (int i = 0; i < Y.size(); ++i) {
        auto Ekpair = Y[i];
        auto [Ekl, Ekr] = Ekpair;
        Ekl.insert(Ekl.end(), Ekr.begin(), Ekr.end());
        auto Ek = Ekl;

        //Calculate tweak as 3*|f| + 2k
        int wireIndex = (numberOfWires-(Y.size()-i));
        int tweak = (numberOfWires * 3) + (2 * wireIndex);

        //H'(Ek, tweak)
        auto EkHash = hashPrime(Ek, k, tweak);

        auto encryptedOutputLabel0 = get<0>(d[i]);
        auto encryptedOutputLabel1 = get<1>(d[i]);
        if(EkHash == encryptedOutputLabel0) {
            y.emplace_back(0);
            //cout << "IT WAS CORRECT FOR FALSE!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl;
            //cout << "EkHash: " << util::uintVec2Str(EkHash) << endl;
            //cout << "encryptedOutputLabel0: " << util::uintVec2Str(encryptedOutputLabel0) << endl;
            //cout << "encryptedOutputLabel1: " << util::uintVec2Str(encryptedOutputLabel1) << endl;
        }
        else if(EkHash == encryptedOutputLabel1) {
            y.emplace_back(1);
            //cout << "IT WAS CORRECT FOR TRUE!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl;
            //cout << "EkHash: " << util::uintVec2Str(EkHash) << endl;
            //cout << "encryptedOutputLabel0: " << util::uintVec2Str(encryptedOutputLabel0) << endl;
            //cout << "encryptedOutputLabel1: " << util::uintVec2Str(encryptedOutputLabel1) << endl;
        }
        else {
            y.emplace_back(2);
            //cout << "Input did not decode to anything correct [threeHalves::decode]" << endl;
            //cout << "EkHash: " << util::uintVec2Str(EkHash) << endl;
            //cout << "encryptedOutputLabel0: " << util::uintVec2Str(encryptedOutputLabel0) << endl;
            //cout << "encryptedOutputLabel1: " << util::uintVec2Str(encryptedOutputLabel1) << endl;
            //exit(2);
        }
    }
    return y;
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
    int a = 1- permuteBitA;
    int b = 1- permuteBitB;
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

vector<halfLabels>
threeHalves::calcZij(halfLabels &A0, halfLabels &B0, halfLabels &A1, halfLabels &B1, vint &rVec, int permuteBitA,
                     int permuteBitB, halfLabels &delta) {
    vector<halfLabels> Zij(4);
    int ctr = 0;
    auto [A0Left, A0Right] = A0;
    auto [B0Left, B0Right] = B0;
    auto [A1Left, A1Right] = A1;
    auto [B1Left, B1Right] = B1;

    for (int i = 0; i <= 1; ++i) {
        for (int j = 0; j <= 1; ++j) {
            int index1 = 4 * i + 2 * j;
            int index2 = 4 * i + 2 * j + 1;

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
                auto right = util::vecXOR(BRight, ALeft);
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
            bool includeDelta = (permuteBitA ^ i) & (permuteBitB ^ j);
            if(includeDelta) {
                Yij = util::halfLabelXOR(Yij, delta);
            }


            Zij[ctr] = Yij;
            ctr++;
        }
    }
    return Zij;
}

halfLabels threeHalves::decodeR(vector<uint64_t> rVec, halfLabels A, halfLabels B, int Aperm, int Bperm) {
    auto c1 = rVec[0];
    auto c2 = rVec[1];
    auto [Al, Ar] = std::move(A);
    auto [Bl, Br] = std::move(B);

    //cout << "c1: " << c1 << endl;
    //cout << "c2: " << c2 << endl;

    halfLabels Rij;
    halfLabels Rp;
    halfLabels Rijp;

    switch (c1*2 + c2) {
        case 0:
            Rij = {{0},
                   {0}};
            break;
        case 1:
            Rij = {util::vecXOR(Al, Br),    // s2
                   util::vecXOR({Ar, Bl, Br})};
            break;
        case 2:
            Rij = {util::vecXOR({Al, Ar, Bl}),  // s1
                   util::vecXOR({Al, Br})};
            break;
        case 3:
            Rij = {util::vecXOR({Ar, Bl, Br}),  // s1 s2
                   util::vecXOR({Al, Ar, Bl})};
            break;
    }

    vint zeroes(Al.size());

    switch (Aperm*2 + Bperm) {
        case 0:
            Rp = { Bl,                                      //{{0, 0}, {1, 0},  // rp
                   Ar };                                       // {0, 1}, {0, 0}};
            break;
        case 1:
            Rp =  { Bl,    //{{0, 0}, {1, 0},  // s2 rp
                    zeroes};   // {0, 0}, {0, 0}};
            break;
        case 2:
            Rp =  {zeroes,    //{{0, 0}, {0, 0},  // s1 rp
                   Ar};          // {0, 1}, {0, 0}};
            break;
        case 3:
            Rp =  {zeroes,     //{{0, 0}, {0, 0},  // s1 s2 rp
                   zeroes};           // {0, 0}, {0, 0}};
            break;
    }

    Rijp = util::halfLabelXOR(Rij, Rp);

    return Rijp;
}
vector<int> threeHalves::computeT(int permuteBitA, int permuteBitB, const string& gateType) {
    if(gateType == "XOR"){
        return {permuteBitA ^ permuteBitB, permuteBitA ^ (1-permuteBitB), (1-permuteBitA) ^ permuteBitB, (1-permuteBitA) ^ (1-permuteBitB)};
    }
    else if(gateType == "AND") {
        return {permuteBitA & permuteBitB, permuteBitA & (1-permuteBitB), (1-permuteBitA) & permuteBitB, (1-permuteBitA) & (1-permuteBitB)};
    }
    else {
        cout << "Unknown gatetype in computeT" << endl;
    }
}
