//
// Created by svend on 001, 01-03-2023.
//

#include "threeHalves.h"
#include <set>
#include <utility>


tuple<Ftype, tuple<halfDelta, vector<tuple<halfLabels, int>>>, vector<halfLabels>, halfLabels, hashRTCCR>
threeHalves::garble(circuit f, int k, util::hashtype h) {
    //get number of wires and gates
    int numberOfWires = circuitParser::getWires(f);

    //get number of input and output bits
    int numberOfInputBits = circuitParser::getInputSize(f);
    int numberOfOutputBits = circuitParser::getOutBits(f);
    halfDelta delta = genDeltaHalves(k);
    vector<tuple<halfLabels, int>> labelAndPermuteBitPairs(numberOfWires);
    vector<tuple<halfLabels, int>> inputLabelAndPermuteBitPairs(numberOfInputBits); //testing
    vector<halfLabels> encryptedOutputLabels(numberOfOutputBits);
    initInputLabels(k, numberOfInputBits, labelAndPermuteBitPairs, inputLabelAndPermuteBitPairs);

    vint key = util::genBitsNonCrypto(256);
    vint iv = util::genBitsNonCrypto(256);
    hashRTCCR hashRTCCR(key, iv, 256);

    //invConst
    halfLabels invConst = genLabelHalves(k);

    Ftype F(numberOfWires);
    for (int i = 2; i < f.size(); ++i) {
        //////////////////////// Getting out gate from string //////////////////////////
        //auto &line = f[i];
        //auto gateInfo = util::extractGate(line);              // "2 1 0 1 2 XOR"
        //auto inputWires = get<0>(gateInfo);             // [ ..., 0, 1]
        //auto outputWires = get<1>(gateInfo);                    // [..., 1, ..., 2]
        //auto gateType = get<2>(gateInfo);               // "XOR"

        auto [inputWires,outputWires,gateType] = f[i];

        tuple<halfLabels, int> A0AndPermuteBit;
        tuple<halfLabels, int> B0AndPermuteBit;
        if(gateType == "XOR") {
            gateXOR(inputWires, outputWires, labelAndPermuteBitPairs, A0AndPermuteBit, B0AndPermuteBit);

        }
        else if(gateType == "INV") {
            gateINV(invConst, outputWires, inputWires, labelAndPermuteBitPairs, A0AndPermuteBit);
        }
        else if( gateType == "AND") {
            gateAND(k, h, hashRTCCR, inputWires, outputWires, delta, labelAndPermuteBitPairs, F, A0AndPermuteBit,
                    B0AndPermuteBit);
        }
        else {
            cout << "Got a non-supported gate" << endl;
            exit(2);
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

    //Return F, e, d.
    return make_tuple(F, e, d, invConst, hashRTCCR);
}

void threeHalves::gateAND(int k, const util::hashtype &h, hashRTCCR &hashRTCCR, const vector<int> &inputWires,
                          const vector<int> &outputWires, halfDelta &delta,
                          vector<tuple<halfLabels, int>> &labelAndPermuteBitPairs, Ftype &F,
                          tuple<halfLabels, int> &A0AndPermuteBit, tuple<halfLabels, int> &B0AndPermuteBit) {
    A0AndPermuteBit = labelAndPermuteBitPairs[inputWires[0]];
    B0AndPermuteBit = labelAndPermuteBitPairs[inputWires[1]];
    auto [A0, permuteBitA] = A0AndPermuteBit;
    auto [B0, permuteBitB] = B0AndPermuteBit;

    //Compute A1 and B1
    auto A1 = util::halfLabelXOR(A0, delta);
    auto B1 = util::halfLabelXOR(B0, delta);
    auto [A0Left, A0Right] = A0;
    auto [B0Left, B0Right] = B0;
    auto [A1Left, A1Right] = A1;
    auto [B1Left, B1Right] = B1;
    auto [deltaLeft, deltaRight] = delta;

    //SampleR
    auto rVec = sampleR(permuteBitA, permuteBitB);

    //Calculate Zij
    auto Zij = calcZij(A0, B0, A1, B1, rVec, permuteBitA, permuteBitB, delta);

    //Prepend rVec to Z
    Zij = calcRZ(rVec, Zij);
    //VInvZ is calculated per discord picture
    vector<vint> VInvZ = calcVZ(Zij);

    //VInvRpABDelta is calculated per discord picture
    vector<vint> VInvRpABDelta = calcVRP(A0Left, A0Right, B0Left, deltaLeft, deltaRight);
    //Prepend 0's to all entries to make lenghts match (According to Lance)
    for (int l = 0; l < 5; ++l) {
        util::prependBitToVint(VInvRpABDelta[l], 0);
    }

    //HVecPrime is calculated per discord picture
    vint hashA0;
    vint hashA1;
    vint hashB0;
    vint hashB1;
    vint hashA0xorB0;
    vint hashA0xorB0xorDelta;
    vector<vint> hashes({hashA0, hashA1, hashB0, hashB1, hashA0xorB0, hashA0xorB0xorDelta});
    halfLabels A0xorB0 = {util::vecXOR(A0Left, B0Left), util::vecXOR(A0Right, B0Right)};
    //A0 XOR B0 XOR Delta = A0 XOR B1
    halfLabels A0xorB0xorDelta = {util::vecXOR(A0Left, B1Left), util::vecXOR(A0Right, B1Right)};
    vector<halfLabels> inputs({A0, A1, B0, B1, A0xorB0, A0xorB0xorDelta});
    if (h == util::RO) {
        hashes = hashRO(k, hashes, inputs);
    } else if(h==util::fast){
        hashes = hashFast(k, hashRTCCR, A0, A1, B0, B1, A0Left, hashes, A0xorB0, A0xorB0xorDelta, inputs);

    }
    vint HA0 = hashes[0];
    vint HA1 = hashes[1];
    vint HB0 = hashes[2];
    vint HB1 = hashes[3];
    vint HA0xorB0 = hashes[4];
    vint HA0xorB0xorDelta = hashes[5];

    //make sure hash vectors extra bit is only one bit
    sliceHashes(HA0, HA1, HB0, HB1, HA0xorB0, HA0xorB0xorDelta);


    vector<vint> HVecPrime = calcHprime(HA0, HA1, HB0, HB1, HA0xorB0, HA0xorB0xorDelta);


    vector<vint> CG(5);
    vector<uint8_t> zVec(5);
    //XOR stuff together
    calcZCG(VInvZ, VInvRpABDelta, HVecPrime, CG, zVec);

    uint64_t permuteBitCipher = CG[0][0] & 1;
    halfLabels cipherLabel = {CG[0], CG[1]}; //C left and C right
    if(permuteBitCipher == 1) {
        cipherLabel = util::halfLabelXOR(cipherLabel, delta);
    }
    auto outputCipher = make_tuple(cipherLabel, permuteBitCipher);
    labelAndPermuteBitPairs[outputWires[0]] = outputCipher;
    F[outputWires[0]] = {CG[2], CG[3], CG[4], zVec};
}

vector<halfLabels> &threeHalves::calcRZ(vint &rVec, vector<halfLabels> &Zij) {
    for (int l = 0; l < 4; ++l) {
        auto [left, right] = Zij[l];
        left = util::prependBitToVint(left, util::checkIthBit(rVec, l * 2));
        right = util::prependBitToVint(right, util::checkIthBit(rVec, l * 2 + 1));
        Zij[l] = {left, right};
    }
    return Zij;
}

void
threeHalves::calcZCG(const vector<vint> &VInvZ, const vector<vint> &VInvRpABDelta, const vector<vint> &HVecPrime,
                     vector<vint> &CG, vector<uint8_t> &zVec) {
    for (int l = 0; l < 5; ++l) {
        auto zConcatCG = util::vecXOR({VInvZ[l], VInvRpABDelta[l], HVecPrime[l]});
        zVec[l] = (zConcatCG[zConcatCG.size()-1]);
        zConcatCG.pop_back();
        CG[l] = zConcatCG;
    }
}

vector<vint>
threeHalves::calcHprime(vint &HA0, vint &HA1, vint &HB0, vint &HB1, const vint &HA0xorB0, vint &HA0xorB0xorDelta) {
    vector<vint> HVecPrime(5);
    HVecPrime[0] = util::vecXOR(HA0, HA0xorB0);
    HVecPrime[1] = util::vecXOR(HB0, HA0xorB0);
    HVecPrime[2] = util::vecXOR(HA1, HA0);
    HVecPrime[3] = util::vecXOR(HB1, HB0);
    HVecPrime[4] = util::vecXOR(HA0xorB0xorDelta, HA0xorB0);
    return HVecPrime;
}

void
threeHalves::sliceHashes(vint &HA0, vint &HA1, vint &HB0, vint &HB1, vint &HA0xorB0, vint &HA0xorB0xorDelta) {
    HA0[HA0.size() - 1] = HA0[HA0.size() - 1] & 1;
    HA1[HA1.size() - 1] = HA1[HA1.size() - 1] & 1;
    HB0[HB0.size() - 1] = HB0[HB0.size() - 1] & 1;
    HB1[HB1.size() - 1] = HB1[HB1.size() - 1] & 1;
    HA0xorB0[HA0xorB0.size() - 1] = HA0xorB0[HA0xorB0.size() - 1] & 1;
    HA0xorB0xorDelta[HA0xorB0xorDelta.size() - 1] = HA0xorB0xorDelta[HA0xorB0xorDelta.size() - 1] & 1;
}

vector<vint> &
threeHalves::hashFast(int k, hashRTCCR &hashRTCCR, halfLabels &A0, halfLabels &A1, halfLabels &B0, halfLabels &B1,
                      const vint &A0Left, vector<vint> &hashes, halfLabels &A0xorB0, halfLabels &A0xorB0xorDelta,
                      vector<halfLabels> &inputs) {
    for (halfLabels lbl: inputs) {
        get<0>(lbl).emplace_back(0);
        get<1>(lbl).emplace_back(0);
    }
    uint64_t k64 = k;
    vint tweak0 {((3 * k64) - 3)};
    vint tweak1 {((3 * k64) - 2)};
    vint tweak2 {((3 * k64) - 1)};

    hashes[0] = hashRTCCR.hash(A0             , tweak0);
    hashes[1] = hashRTCCR.hash(A1             , tweak0);
    hashes[2] = hashRTCCR.hash(B0             , tweak1);
    hashes[3] = hashRTCCR.hash(B1             , tweak1);
    hashes[4] = hashRTCCR.hash(A0xorB0        , tweak2);
    hashes[5] = hashRTCCR.hash(A0xorB0xorDelta, tweak2);

    int size = hashes[0].size()-(A0Left.size()+1);
    for (int j = 0; j < size; ++j) {
        for (int hIx = 0; hIx < hashes.size(); ++hIx) {
            hashes[hIx].pop_back();
        }
    }
    return hashes;
}

vector<vint> &threeHalves::hashRO(int k, vector<vint> &hashes, const vector<halfLabels> &inputs) {
    vector<vint> in_for_RO(inputs.size());
    uint64_t tweak ((3 * k) - 3);
    for (int j = 0; j < inputs.size(); ++j) {
        //inputgen
        in_for_RO[j] = util::vecConcat(get<0>(inputs[j]),get<1>(inputs[j]));
        if (j==2||j==4){
            tweak += 1;
        }
        hashes[j] = util::hash_variable(in_for_RO[j], {tweak}, ((k / 2) + 8));
    }
    return hashes;
}

vector<vint>
threeHalves::calcVRP(const vint &A0Left, const vint &A0Right, const vint &B0Left, const vint &deltaLeft,
                     const vint &deltaRight) {
    vector<vint> VInvRpABDelta(5);
    VInvRpABDelta[0] = B0Left;
    VInvRpABDelta[1] = A0Right;
    VInvRpABDelta[2] = util::vecXOR(deltaRight, B0Left);
    VInvRpABDelta[3] = util::vecXOR(deltaLeft, A0Right);
    vint zeroes(A0Left.size());
    VInvRpABDelta[4] = zeroes;
    return VInvRpABDelta;
}

vector<vint> threeHalves::calcVZ(const vector<halfLabels> &Zij) {
    vector<vint> VInvZ(5);
    VInvZ[0] = get<0>(Zij[0]);
    VInvZ[1] = get<1>(Zij[0]);
    VInvZ[2] = util::vecXOR({get<0>(Zij[2]), get<1>(Zij[2]), get<0>(Zij[0]), get<1>(Zij[0])});
    VInvZ[3] = util::vecXOR({get<0>(Zij[1]), get<1>(Zij[1]), get<0>(Zij[0]), get<1>(Zij[0])});
    VInvZ[4] = util::vecXOR(get<0>(Zij[3]), get<0>(Zij[2]));
    return VInvZ;
}

void threeHalves::gateINV(halfLabels &invConst, const vector<int> &outputWires, const vector<int> &inputWires,
                          vector<tuple<halfLabels, int>> &labelAndPermuteBitPairs,
                          tuple<halfLabels, int> &A0AndPermuteBit) {
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

void threeHalves::gateXOR(const vector<int> &inputWires, const vector<int> &outputWires,
                          vector<tuple<halfLabels, int>> &labelAndPermuteBitPairs,
                          tuple<halfLabels, int> &A0AndPermuteBit, tuple<halfLabels, int> &B0AndPermuteBit) {
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

void
threeHalves::initInputLabels(int k, int numberOfInputBits, vector<tuple<halfLabels, int>> &labelAndPermuteBitPairs,
                             vector<tuple<halfLabels, int>> &inputLabelAndPermuteBitPairs) {
    for (int i = 0; i < numberOfInputBits; i++){
        auto label0 = genLabelHalves(k);
        auto permuteBit = (int)util::random_bitset<1>().to_ulong();
        labelAndPermuteBitPairs[i] = {label0, permuteBit};
        inputLabelAndPermuteBitPairs[i] = {label0, permuteBit};
    }
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
        if(permuteBit == 1){
            auto deltaLeft = get<0>(delta);
            auto deltaRight = get<1>(delta);
            label0 = util::vecXOR(label0, deltaLeft);
            label1 = util::vecXOR(label1, deltaRight);
        }
        halfLabels = {label0, label1};
        X.emplace_back(halfLabels);
    }
    return X;
}

vector<halfLabels> threeHalves::eval(Ftype F, vector<halfLabels> X, circuit f, int k, const halfLabels& invConst, hashRTCCR &hash, util::hashtype h) {
    //auto &wireAndGates = f[0];
    //auto gatesAndWiresSplit = util::split(wireAndGates, ' ');
    int numberOfWires = circuitParser::getWires(f); //stoi(gatesAndWiresSplit[1]);

    //get number of input and output bits
    int numberOfInputBits = circuitParser::getInputSize(f); //util::getBits(f[1]);
    int numberOfOutputBits = circuitParser::getOutBits(f); //util::getBits(f[2]);

    vector<halfLabels> Y(numberOfOutputBits);
    vector<tuple<halfLabels, int>> labelAndPermuteBitPairs(numberOfWires);
    //fill with encrypted input e
    for (int i = 0; i < X.size(); ++i) {
        auto permuteBit = (get<0>(X[i])[0]) & 1;
        labelAndPermuteBitPairs[i] = {X[i], permuteBit};
    }

    for (int i = 2; i < f.size(); ++i) {
        //////////////////////// Getting out gate from string //////////////////////////
        //auto &line = f[i];
        //auto gateInfo = util::extractGate(line);              // "2 1 0 1 2 XOR"
        //auto inputWires = get<0>(gateInfo);         // [ ..., 0, 1]
        //auto outputWires = get<1>(gateInfo);        // [..., 1, ..., 2]
        //auto gateType = get<2>(gateInfo);               // "XOR"

        auto [inputWires,outputWires,gateType] = f[i];

        auto [A, ApermuteBit] = labelAndPermuteBitPairs[inputWires[0]];
        auto [B, BpermuteBit] = labelAndPermuteBitPairs[inputWires[0]];
        if(inputWires.size()==2)  {
            auto [C, CpermuteBit] = labelAndPermuteBitPairs[inputWires[1]];
            B = C;
            BpermuteBit = CpermuteBit;
        }
        auto [Al, Ar] = A;
        auto [Bl, Br] = B;

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

            vint zeroes(Al.size());
            vector<vint> gVec = {zeroes, zeroes, g0, g1, g2};
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

            //Hashes
            vector<vint> hVec;

            vector<vint> hashes(3);
            halfLabels AxorB = {util::vecXOR(Al, Bl), util::vecXOR(Ar, Br)};
            vector<halfLabels> inputs({A, B, AxorB});
            if(h==util::RO) {
                vector<vint> in_for_RO(inputs.size());
                ::uint64_t tweak = (3 * k) - 3;
                for (int j = 0; j < inputs.size(); ++j) {
                    //inputgen
                    in_for_RO[j] = util::vecConcat(get<0>(inputs[j]),get<1>(inputs[j]));

                    hashes[j]= util::hash_variable(in_for_RO[j],{tweak+j},(k/2)+8);
                }
            } else if(h==util::fast){
                for (halfLabels lbl:inputs) {
                    get<0>(lbl).emplace_back(0);
                    get<1>(lbl).emplace_back(0);
                }
                uint64_t k64 = k;
                vint tweak0 {((3 * k64) - 3)};
                vint tweak1 {((3 * k64) - 2)};
                vint tweak2 {((3 * k64) - 1)};

                hashes[0] = hash.hash(inputs[0], tweak0);
                hashes[1] = hash.hash(inputs[1], tweak1);
                hashes[2] = hash.hash(inputs[2], tweak2);

                int size = hashes[0].size()-(Al.size()+1);
                for (int j = 0; j < size; ++j) {
                    for (int hIx = 0; hIx < hashes.size(); ++hIx) {
                        hashes[hIx].pop_back();
                    }
                }
            }
            vint HA = hashes[0];
            vint HB = hashes[1];
            vint HAxorB = hashes[2];

            //make sure hash vectors extra bit is only one bit
            HA[HA.size()-1] = HA[HA.size()-1] & 1;
            HB[HB.size()-1] = HB[HB.size()-1] & 1;
            HAxorB[HAxorB.size()-1] = HAxorB[HAxorB.size()-1] & 1;

            hVec = {util::vecXOR(HA, HAxorB),       // { HA,  0, HAB },
                    util::vecXOR(HB, HAxorB)};      // {  0, HB, HAB }

            // rVec || Xij
            vector<vint> rxVec = {util::vecXOR(vzgVec[0], hVec[0]),     // { rl, Xl },
                                  util::vecXOR(vzgVec[1], hVec[1])};    // { rr, Xr }

            // rVec
            vector<uint64_t> rVec = {rxVec[0][rxVec[0].size()-1],
                                     rxVec[1][rxVec[1].size()-1]};

            //Xij
            halfLabels Xij = {{rxVec[0].begin(), rxVec[0].end()-1},
                                      {rxVec[1].begin(), rxVec[1].end()-1}};

            //RijpAB
            auto RijAB = decodeR(rVec, A, B, ApermuteBit, BpermuteBit);

            //Ek
            Ek = util::halfLabelXOR(Xij, RijAB);

            EpermuteBit = get<0>(Ek)[0] & 1;
        }
        labelAndPermuteBitPairs[outputWires[0]] = {Ek, EpermuteBit};

        if(outputWires[0] >= numberOfWires - numberOfOutputBits){
            Y[outputWires[0]-(numberOfWires-numberOfOutputBits)] = (Ek);
        }
    }
    return Y;
}

vint threeHalves::decode(vector<halfLabels> d, vector<halfLabels> Y, circuit f, int k) {
    auto outbits = Y.size();
    auto unit64sNeeded = outbits/64 + ((outbits%64!=0) ? 1 : 0);
    auto outputSets =  vector<bitset<64>>(unit64sNeeded);

    //auto &wireAndGates = f[0];
    //auto gatesAndWiresSplit = util::split(wireAndGates, ' ');
    int numberOfWires = circuitParser::getWires(f);//stoi(gatesAndWiresSplit[1]);
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
            outputSets = util::insertBitVecBitset(outputSets,0,i);
        }
        else if(EkHash == encryptedOutputLabel1) {
            outputSets = util::insertBitVecBitset(outputSets,1,i);
        }
        else {
            exit(2);
        }
    }
    auto y = vint(unit64sNeeded);
    for (int i = 0; i < unit64sNeeded; ++i) {
        y[i] = outputSets[i].to_ullong();
    }
    return y;
}

vector<int> threeHalves::decodeBits(vector<halfLabels> d, vector<halfLabels> Y, circuit f, int k) {
    //auto &wireAndGates = f[0];
    //auto gatesAndWiresSplit = util::split(wireAndGates, ' ');
    int numberOfWires = circuitParser::getWires(f); //stoi(gatesAndWiresSplit[1]);
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
        }
        else if(EkHash == encryptedOutputLabel1) {
            y.emplace_back(1);
        }
        else {
            y.emplace_back(2);
            cout << "Input did not decodeBits to anything correct [threeHalves::decodeBits]" << endl;
            exit(2);
        }
    }
    return y;
}

vector<uint64_t> threeHalves::hashPrime(const vint& input, int k, int tweak) {
    //string inputString = util::uintVec2Str(input);

    auto leftHalfHash = util::hash_variable( input, {(::uint64_t)tweak}, k);
    auto rightHalfHash = util::hash_variable( input,{(::uint64_t)(tweak+1)}, k);
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
            halfLabels rS1AjBl = zeroes(ALeft.size());
            halfLabels rS2AjBl = zeroes(ALeft.size());
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

halfLabels threeHalves::zeroes(uint64_t size) {
    vint zeroes(size);
    return {zeroes, zeroes};
}
