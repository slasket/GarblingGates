

#include <boost/lambda/lambda.hpp>
#include <iostream>
#include "bloodcompatibility.h"
#include "util/circuitParser.h"
#include "util/util.h"
#include "schemes/baseGarble.h"
#include "schemes/atecaGarble.h"
#include "schemes/atecaFreeXOR.h"
#include <bitset>
#include "util/hashRTCCR.h"
#include <chrono>
#include <openssl/sha.h>
#include "schemes/threeHalves.h"
#include "util/hashTCCR.h"
#include "util/timing.h"
#include <boost/timer.hpp>

using namespace std;


void testsubAteca();
void testFreexorAteca();
void testBaseOT(int v, int k , int l, int elgamalKeySize);

void sliceTest();

void threehalves_Test();

int main() {
    //vector<string> f = circuitParser::parseCircuit("../tests/circuits/adder64.txt");
    //auto x = vector<int>{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    //vector<int> x = util::genFunctionInput(128);


    //auto k=256;
    //timing::hashOutputLengthTest(128);
    //timing::testLabelLength();

    auto k=256;
    vector<string> f = circuitParser::parseCircuit("../tests/circuits/Keccak_f.txt");
    circuit b = circuitParser::parse("../tests/circuits/Keccak_f.txt");
    vector<int> x = util::genFunctionInput(circuitParser::inputsize(f));
    util::hashtype type = util::fast;
    //timing::time_circuit_all(f,b,x,k,type);
    //cout<<endl;
    f = circuitParser::parseCircuit("../tests/circuits/aes_128.txt");
    auto f2 = circuitParser::parse("../tests/circuits/aes_128.txt");
    //auto f2 = circuitParser::parse("../tests/circuits/aes_128.txt");
    //timing::testSchemesVariableLabelSize(type, 1000);
    timing::repetitionTest(f2,k,type,100);


    return 0;
}



void threehalves_Test() {
    //hashRTCCR::testHashRTCCR();
    hashRTCCR::testDecrypt();

    int lInput =6;
    int rInput = 1;
    auto xb = vector<int>{1,1,0,0,0,1,1};
    auto Cb = circuitParser::parse("../tests/circuits/BloodComp.txt");

    auto [Fb, eb, db, icb, hashb] = threeHalves::garble(Cb, 128, util::fast);
    auto encLabelsb = threeHalves::encode(eb, xb);
    auto Yb = threeHalves::eval(Fb, encLabelsb, Cb, 128, icb, hashb, util::fast);
    auto yb = threeHalves::decode(db, Yb, Cb, 128);
    uint64_t bloodCompAns = bloodcompatibility::bloodCompLookup(lInput,rInput);

    cout << (yb[0]==bloodCompAns) << endl;

    auto smalltest = circuitParser::parse("../tests/circuits/andTest.txt");
    int k = 256;
    int h = 1;
    auto [F,e,d,ic, hash] = threeHalves::garble(smalltest, k, util::fast);
    auto x = vector<int>{1, 1};
    auto encLabels = threeHalves::encode(e, x);
    auto Y = threeHalves::eval(F, encLabels, smalltest, k, ic, hash, util::fast);
    auto y = threeHalves::decode(d, Y, smalltest, k);
    if(y.size() == 1) {
        cout << y[0] << endl;
    }
    k = 128;
    auto [F1,e1,d1,ic1, hash1] = threeHalves::garble(smalltest, k, util::fast);
    encLabels = threeHalves::encode(e1, x);
    Y = threeHalves::eval(F1, encLabels, smalltest, k, ic1, hash1, util::fast);
    y = threeHalves::decode(d1, Y, smalltest, k);
    if(y.size() == 1) {
        cout << y[0] << endl;
    }


    auto a = util::genBitsNonCrypto(128);
    auto b = util::genBitsNonCrypto(128);
    halfLabels A = {a, b};
    vint tweak{1};
    auto HA = hash.hash(A, tweak);
    util::printUintVec(a);
    util::printUintVec(b);
    util::printUintVec(hash.getAlpha());
    util::printUintVec(hash.getU1());
    util::printUintVec(hash.getU2());
    cout << "hash1" << endl;
    util::printUintVec(HA);
    HA = hash.hash(A, tweak);
    cout << "hash2" << endl;
    util::printUintVec(HA);
}


void sliceTest() {
    vint globalDelta ={12249790986447749120};
    vint l00 = {86, 62, 2};
    vint l01 = {86, 148, 2};
    vint l10 = {252, 62, 2};
    vint l11 = {252, 148, 2};
    int internalParam= 8 * 16;

    vint X_00 = util::hash_variable(l00,{}, internalParam);
    vint X_01 = util::hash_variable(l01,{}, internalParam);
    vint X_10 = util::hash_variable(l10,{}, internalParam);
    vint X_11 = util::hash_variable(l11,{}, internalParam);
    util::printUintVec(X_00);
    util::printUintVec(X_01);
    util::printUintVec(X_10);
    util::printUintVec(X_11);
    auto delta = vint((internalParam+63)/64);

    int j =0; int deltaHW =0;
    do {
        string slice = util::sliceVecL2RAtecaFreeXorSpecial(globalDelta, X_00, X_01, X_10, X_11, deltaHW, j);
        ///slices of importance "00000", "10001", "11110", "01111"
        if (slice=="00000"||slice=="10001"||slice=="11110"||slice=="01111"){
            util::setIthBitTo1L2R(&delta,j);
            deltaHW++;
        }
        j++;
    }while(deltaHW!=8);
    util::printUintVec(delta);

    vint L0 = util::projection(X_00, delta);
    vint Lx01 = util::projection(X_01, delta);
    vint Lx10 = util::projection(X_10, delta);

    vint L1 = util::projection(X_11, delta);
    util::printUintVec(L0);
    util::printUintVec(Lx01);
    util::printUintVec(Lx10);
    util::printUintVec(L1);

    cout<<"this is delta"<<endl;
    util::printUintVec(util::vecXOR(L0,L1));
}




void testFreexorAteca() {
    //least significant bit first :^)
    //auto finput = vector<int>{0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    //                          0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    //                          0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    //                          0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    //auto C = circuitParser::parseCircuit("../tests/circuits/sub64.txt");
    int lInput =6; int rInput = 1;
    ::uint64_t bloodCompAns = bloodcompatibility::bloodCompLookup(lInput,rInput);
    auto finput = vector<int>{1,1,0,0,0,1,1};
    auto C = circuitParser::parse("../tests/circuits/BloodComp.txt");

    cout<<"garbling"<<endl;
    auto [F,encodingInfo,decoding,secL,invVar,hashtype] = atecaFreeXOR::garble(C, 64, util::RO);
    cout<<"encoding"<<endl;
    auto encodedInput = atecaFreeXOR::encode(encodingInfo, finput);
    cout<<"eval"<<endl;
    auto Y = atecaFreeXOR::eval(F, encodedInput, C, secL, invVar, hashtype);
    cout<<"decoding"<<endl;
    auto y = atecaFreeXOR::decode(Y, decoding, hashtype);
    util::printUintVec(y);
    cout<< "bloodAns "<< bloodCompAns<<endl;
}

void testsubAteca() {
    //least significant bit first :^)
    auto finput = vector<int>{0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                              0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                              0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                              0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    auto C = circuitParser::parse("../tests/circuits/sub64.txt");
    cout<<"garbling"<<endl;
    auto feds = atecaGarble::garble(C, 64, util::RO);
    cout<<"encoding"<<endl;
    auto encodedInput = atecaGarble::encode(get<1>(feds), finput);
    cout<<"eval"<<endl;
    auto Y = atecaGarble::eval(get<0>(feds), encodedInput, C, get<3>(feds), get<4>(feds), get<5>(feds));
    cout<<"decoding"<<endl;
    auto y = atecaGarble::decode(Y, get<2>(feds), get<5>(feds));
    util::printUintVec(y);
}

//util::printCircuit("../tests/circuits/BloodComp.txt");
//testBaseOT(64,512,256,2048);
//bloodcompatibility bc;
//bc.testAllCombinations();