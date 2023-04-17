

#include <boost/lambda/lambda.hpp>
#include <iostream>
#include "bloodcompatibility.h"
#include "util/otUtil/otUtil.h"
#include "util/OTUtil/OT.h"
#include "util/circuitParser.h"
#include "util/util.h"
#include "schemes/baseGarble.h"
#include "schemes/atecaGarble.h"
#include "schemes/atecaFreeXOR.h"
#include <bitset>
#include <string>
#include <emmintrin.h>
#include <immintrin.h>
#include "util/hashRTCCR.h"
#include <chrono>
#include <utility>
#include <openssl/sha.h>
#include "schemes/threeHalves.h"
#include "util/hashTCCR.h"

using namespace std;


void testsubAteca();
void testFreexorAteca();
void testBaseOT(int v, int k , int l, int elgamalKeySize);

void sliceTest();

void threehalves_Test();
void timetest(const vector<string>&c, const vector<int>& x, int k, util::scheme type, util::hashtype typ);

int main() {
    //this is a comment
    vector<string> c = circuitParser::parseCircuit("../tests/circuits/adder64.txt");
    //auto x = vector<int>{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    vector<int> x = util::genFunctionInput(64);
    //vector<string> c = circuitParser::parseCircuit("../tests/circuits/aes_128.txt");
    //vector<int> x = util::genFunctionInput(256);
    timetest(c,x,128,util::baseline, util::RO);
    timetest(c,x,128,util::threehalves, util::RO);
    timetest(c,x,128,util::ateca, util::RO);
    timetest(c,x,128,util::atecaFXOR, util::RO);
    return 0;

}


void timetest(const vector<string>&c, const vector<int>& x, int k, util::scheme type, util::hashtype typ){
    using std::chrono::high_resolution_clock;
    using std::chrono::duration_cast;
    using std::chrono::duration;
    using std::chrono::milliseconds;

    switch (type) {
        case util::scheme::baseline:{
            cout<< "baseline"<<endl;
            auto t1 = high_resolution_clock::now();
            auto base_C = baseGarble::garble(c,k);//needs hash type
            auto t2 = high_resolution_clock::now();
            duration<double, std::milli> ms_double = t2 - t1;
            cout<< "garbling: " <<ms_double.count()<< "ms"<<endl;

            auto base_X = baseGarble::encode(get<1>(base_C), x);

            t1 = high_resolution_clock::now();
            auto base_Y = baseGarble::eval(base_C, base_X, c);
            t2 = high_resolution_clock::now();
            ms_double = t2 - t1;
            cout<< "evaluation: " <<ms_double.count()<< "ms"<<endl;

            auto base_y = baseGarble::decode(get<2>(base_C), base_Y);
            cout<< "base: " << base_y[0] <<endl;
            break;
        }
        case util::scheme::threehalves:{
            cout<< "three halves"<<endl;
            auto t1 = high_resolution_clock::now();
            auto [three_F,three_e,three_d, three_ic, three_hash] = threeHalves::garble(c, k, typ);
            auto t2 = high_resolution_clock::now();
            duration<double, std::milli> ms_double = t2 - t1;
            cout<< "garbling: " <<ms_double.count()<< "ms"<<endl;

            auto three_X = threeHalves::encode(three_e, x);

            t1 = high_resolution_clock::now();
            auto three_Y = threeHalves::eval(three_F, three_X, c, k, three_ic, three_hash,typ);
            t2 = high_resolution_clock::now();
            ms_double = t2 - t1;
            cout<< "evaluation: " <<ms_double.count()<< "ms"<<endl;

            auto three_y = threeHalves::decode(three_d, three_Y, c, k);
            cout<< "three: " << three_y[0] <<endl;
            break;
        }
        case util::scheme::ateca:{
            cout<< "ateca"<<endl;
            auto t1 = high_resolution_clock::now();
            auto [ate_F, ate_e, ate_d, ate_k, ate_ic,ate_hashtyp, ate_hash, ate_evalhash] = atecaGarble::garble(c, k, typ);
            auto t2 = high_resolution_clock::now();
            duration<double, std::milli> ms_double = t2 - t1;
            cout<< "garbling: " <<ms_double.count()<< "ms"<<endl;

            auto ate_X = atecaGarble::encode(ate_e, x);

            t1 = high_resolution_clock::now();
            auto ate_Y = atecaGarble::eval(ate_F, ate_X, c, ate_k, ate_ic, 0, hashTCCR());
            t2 = high_resolution_clock::now();
            ms_double = t2 - t1;
            cout<< "evaluation: " <<ms_double.count()<< "ms"<<endl;

            auto ate_y = atecaGarble::decode(ate_Y, ate_d, hashTCCR());
            cout<< "ateca: " << ate_y[0] <<endl;
            break;
        }
        case util::scheme::atecaFXOR:{
            cout<< "ateca-Freexor"<<endl;
            auto t1 = high_resolution_clock::now();
            auto [atef_F, atef_e, atef_d, atef_k, atef_ic, atef_hash] = atecaFreeXOR::garble(c, k, typ);
            auto t2 = high_resolution_clock::now();
            duration<double, std::milli> ms_double = t2 - t1;
            cout<< "garbling: " <<ms_double.count()<< "ms"<<endl;

            auto atef_X = atecaFreeXOR::encode(atef_e, x);

            t1 = high_resolution_clock::now();
            auto atef_Y = atecaFreeXOR::eval(atef_F, atef_X, c, atef_k, atef_ic);
            t2 = high_resolution_clock::now();
            ms_double = t2 - t1;
            cout<< "evaluation: " <<ms_double.count()<< "ms"<<endl;

            auto atef_y = atecaFreeXOR::decode(atef_Y, atef_d);

            cout<< "atecaFxor: " << atef_y[0] <<endl;
            break;
        }
        default:
            break;
    }
};

void threehalves_Test() {
    hashRTCCR::testHashRTCCR();
    hashRTCCR::testDecrypt();


    int lInput =6;
    int rInput = 1;
    auto xb = vector<int>{1,1,0,0,0,1,1};
    auto Cb = circuitParser::parseCircuit("../tests/circuits/BloodComp.txt");
    int lb = 64;
    auto [Fb, eb, db, icb, hashb] = threeHalves::garble(Cb, 128);
    auto encLabelsb = threeHalves::encode(eb, xb);
    auto Yb = threeHalves::eval(Fb, encLabelsb, Cb, 128, icb, hashb);
    auto yb = threeHalves::decode(db, Yb, Cb, 128);
    uint64_t bloodCompAns = bloodcompatibility::bloodCompLookup(lInput,rInput);

    cout << (yb[0]==bloodCompAns) << endl;

    vector<string> smalltest = {"1 3", "2 1 1", "1 1", "2 1 0 1 2 AND"};
    int k = 256;
    int h = 1;
    auto [F,e,d,ic, hash] = threeHalves::garble(smalltest, k, h);
    auto x = vector<int>{1, 1};
    auto encLabels = threeHalves::encode(e, x);
    auto Y = threeHalves::eval(F, encLabels, smalltest, k, ic, hash, h);
    auto y = threeHalves::decode(d, Y, smalltest, k);
    (y.size() == 1);
    if(y.size() == 1) {
        cout << y[0] << endl;
    }
    k = 128;
    auto [F1,e1,d1,ic1, hash1] = threeHalves::garble(smalltest, k, h);
    encLabels = threeHalves::encode(e1, x);
    Y = threeHalves::eval(F1, encLabels, smalltest, k, ic1, hash1, h);
    y = threeHalves::decode(d1, Y, smalltest, k);
    (y.size() == 1);
    if(y.size() == 1) {
        cout << y[0] << endl;
    }


    auto a = util::genBitsNonCrypto(128);
    auto b = util::genBitsNonCrypto(128);
    halfLabels A = {a, b};
    auto HA = hashRTCCR::hash(A, {0,0,0,1}, hash.getKey(), hash.getIv(), hash.getE(), hash.getAlpha(), hash.getU1(), hash.getU2());
    util::printUintVec(a);
    util::printUintVec(b);
    util::printUintVec(hash.getAlpha());
    util::printUintVec(hash.getU1());
    util::printUintVec(hash.getU2());
    cout << "hash1" << endl;
    util::printUintVec(HA);
    HA = hashRTCCR::hash(A, {0,0,0,1}, hash.getKey(), hash.getIv(), hash.getE(), hash.getAlpha(), hash.getU1(), hash.getU2());
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

    vint X_00 = util::hash_variable(util::uintVec2Str(l00), internalParam);
    vint X_01 = util::hash_variable(util::uintVec2Str(l01), internalParam);
    vint X_10 = util::hash_variable(util::uintVec2Str(l10), internalParam);
    vint X_11 = util::hash_variable(util::uintVec2Str(l11), internalParam);
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
            delta=util::setIthBitTo1L2R(delta,j);
            deltaHW++;
        }
        j++;
    }while(deltaHW!=8);
    util::printUintVec(delta);

    vint L0 = atecaGarble::projection(X_00, delta);
    vint Lx01 = atecaGarble::projection(X_01, delta);
    vint Lx10 = atecaGarble::projection(X_10, delta);

    vint L1 = atecaGarble::projection(X_11, delta);
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
    auto C = circuitParser::parseCircuit("../tests/circuits/BloodComp.txt");

    cout<<"garbling"<<endl;
    auto [F,encodingInfo,decoding,secL,invVar,hashtype] = atecaFreeXOR::garble(C, 64, util::RO);
    cout<<"encoding"<<endl;
    auto encodedInput = atecaFreeXOR::encode(encodingInfo, finput);
    cout<<"eval"<<endl;
    auto Y = atecaFreeXOR::eval(F, encodedInput, C, secL, invVar);
    cout<<"decoding"<<endl;
    auto y = atecaFreeXOR::decode(Y, decoding);
    util::printUintVec(y);
    cout<< "bloodAns "<< bloodCompAns<<endl;
}

void testsubAteca() {
    //least significant bit first :^)
    auto finput = vector<int>{0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                              0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                              0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                              0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    auto C = circuitParser::parseCircuit("../tests/circuits/sub64.txt");
    cout<<"garbling"<<endl;
    auto feds = atecaGarble::garble(C, 64, util::RO);
    cout<<"encoding"<<endl;
    auto encodedInput = atecaGarble::encode(get<1>(feds), finput);
    cout<<"eval"<<endl;
    auto Y = atecaGarble::eval(get<0>(feds), encodedInput, C, get<3>(feds), get<4>(feds), 0, hashTCCR());
    cout<<"decoding"<<endl;
    auto y = atecaGarble::decode(Y, get<2>(feds), hashTCCR());
    util::printUintVec(y);
}

//util::printCircuit("../tests/circuits/BloodComp.txt");
//testBaseOT(64,512,256,2048);
//bloodcompatibility bc;
//bc.testAllCombinations();


void testBaseOT(int v, int k , int l, int elgamalKeySize){
    cout<< "#Testing baseOTs from "<< v << " to " << k<< " Amount of OTs"<<endl;

    for (int j = v; j <= k; j=j*2) {
        vector<tuple<vector<::uint64_t>,vector<::uint64_t>>> senderPairs(j);
        vector<::uint64_t> selectionBits(j);
        senderPairs = OT::genKAmountOfSelectionStrings(j, 128);

        selectionBits = otUtil::genBitsNonCrypto(j);

        cout<< "Initial OT with "<< j<< " OTs " << endl;

        //initial OT phase
        //cout<< "Starting initial OT phase" << endl;
        auto result = OT::BaseOTTest(elgamalKeySize, j, senderPairs, selectionBits);
        cout<< "done with "<< j<< " OTs " << endl;

        string choicebits;
        int correctcounter = 0;
        int incorrectcounter = 0;
        int zeroes = 0;
        int ones = 0;
        for (int i = 0; i < result->size(); ++i) {
            int choicebit = otUtil::findithBit(selectionBits, i);
            choicebits += to_string(choicebit);
            if(choicebit == 0){
                if(otUtil::printBitsetofVectorofUints(result[i]) == otUtil::printBitsetofVectorofUints(get<0>(senderPairs[i])) ){
                    //cout<<"res: "<<result[i]<<endl;
                    //cout<<"par: "<<otUtil::str2bitstr(get<0>(senderPairs[i]))<<endl;
                    correctcounter++;
                    zeroes++;
                } else {
                    //cout<<"res: "<<result[i]<<endl;
                    //cout<<"par: "<<otUtil::str2bitstr(get<0>(senderPairs[i]))<<endl;
                    incorrectcounter++;
                }
            }else{
                if(otUtil::printBitsetofVectorofUints(result[i]) == otUtil::printBitsetofVectorofUints(get<1>(senderPairs[i])) ){
                    //cout<<"res: "<<result[i]<<endl;
                    //cout<<"par: "<<otUtil::str2bitstr(get<1>(senderPairs[i]))<<endl;
                    correctcounter++;
                    ones++;
                } else {
                    //cout<<"res: "<<result[i]<<endl;
                    //cout<<"par: "<<otUtil::str2bitstr(get<1>(senderPairs[i]))<<endl;
                    incorrectcounter++;
                }
            }
        }
        //reverse findIntchoicebits
        //string findIntchoicebitsReversed;
        //for (int i = 0; i < findIntchoicebits.length(); ++i) {
        //    findIntchoicebitsReversed += findIntchoicebits[findIntchoicebits.length()-1-i];
        //}
        //cout << "findIn Rev: " << findIntchoicebitsReversed << endl;
        //count where choicebits and findIntchoicebits differ
        cout << "correct: " << correctcounter << endl;
        cout << "incorrect: " << incorrectcounter << endl;
        cout<<"zeroes: "<<zeroes<<endl;
        cout<<"ones: "<<ones<<endl;
    }
}