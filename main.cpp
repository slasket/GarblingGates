

#include <boost/lambda/lambda.hpp>
#include <iostream>
#include "bloodcompatibility.h"
#include "util/otUtil/otUtil.h"
#include "util/OTUtil/OT.h"
#include "util/circuitParser.h"
#include "util/util.h"
#include "schemes/baseGarble.h"
#include "schemes/atecaGarble.h"
#include <bitset>
#include <cstdio>


#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>


using namespace std;

#include <openssl/sha.h>


void testBaseOT(int v, int k ,int l, int elgamalKeySize){
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




int main() {
    bitset<64> val ("0001000100010001");
    bitset<64> val2 ("000100000001");

    auto a = vector<::uint64_t>{val2.to_ullong(),0};
    auto b = vector<::uint64_t>{val.to_ullong(),0};

    auto res = atecaGarble::projection(a,b);
    util::printUintVec(res);


    cout << "and"<<endl;
    for (int i = 0; i < 2; ++i) {
        auto finput1 = vector<int>{0,1};
        auto C = circuitParser::parseCircuit("../tests/circuits/andTest.txt");
        int l = 64;
        auto feds = atecaGarble::GbLEAK(l, C);
        auto encodingInfo = get<1>(feds);
        auto encodedInput = atecaGarble::En(get<1>(feds), finput1);

        auto outwires = get<4>(feds);
        auto outwires0 = get<0>(outwires[0]);
        auto outwires1 = get<1>(outwires[0]);
        auto Y = atecaGarble::Ev(get<0>(feds), encodedInput, C, get<3>(feds));
        auto y = atecaGarble::De(Y, get<2>(feds));
        //cout<< "ow0: " << outwires0[0] << " ow1: " <<outwires1[0] <<endl;
        //cout<< "  Y: " << Y[0][0] <<endl;
        cout <<endl;
    }
    cout << "xor"<<endl;
    for (int i = 0; i < 2; ++i) {
        auto finput1 = vector<int>{1,1};
        auto C = circuitParser::parseCircuit("../tests/circuits/xorTest.txt");
        int l = 64;
        auto feds = atecaGarble::GbLEAK(l, C);
        auto encodingInfo = get<1>(feds);
        auto encodedInput = atecaGarble::En(get<1>(feds), finput1);

        auto outwires = get<4>(feds);
        auto outwires0 = get<0>(outwires[0]);
        auto outwires1 = get<1>(outwires[0]);
        auto Y = atecaGarble::Ev(get<0>(feds), encodedInput, C, get<3>(feds));
        auto y = atecaGarble::De(Y, get<2>(feds));
        //cout<< "ow0: " << outwires0[0] << " ow1: " <<outwires1[0] <<endl;
        //cout<< "  Y: " << Y[0][0] <<endl;
        cout <<endl;

    }


    //string input = "123+0uf892ujf984j9f8jds98afuq348+ju fs890_1";
    //vector<uint64_t> output = util::hash_variable(input,64);
    //cout << sha256("1234567890_1") << endl;
    //cout << sha256("1234567890_2") << endl;
    //cout << sha256("1234567890_3") << endl;
    //cout << sha256("1234567890_4") << endl;
    cout<<"xd"<<endl;
    //int aes =  mainAES();
    //auto res = circuitParser::parseCircuit("../tests/circuits/adder64.txt");
    //util::printStrVec(res);

    vector<string> smalltest = {"1 2", "1 1", "1 1", "1 1 0 1 INV"};
    int k = 128;
    for (int i = 0; i < 100; ++i) {
        auto smallCircuit = baseGarble::garble(128, smalltest);
        auto X00 = baseGarble::encode(get<1>(smallCircuit), {0});
        auto Y00 = baseGarble::eval(smallCircuit, X00, smalltest, 128);
        auto y00 = baseGarble::decode(get<2>(smallCircuit), Y00);
        cout << "y00.size: " << y00.size() << endl;
        cout << "y00[0]: " << y00[0] << endl;
    }



    //string input = "1234567890_1";
    //vector<uint64_t> output = hash_variable(input);
    //cout << "output size: " << output.size() * 64 << endl;
    //for (int i = 0; i < output.size(); ++i) {
    //    cout << output[i] << " ";
    //}
    //cout << endl;



    return 0;
}

//util::printCircuit("../tests/circuits/BloodComp.txt");
//testBaseOT(64,512,256,2048);
//bloodcompatibility bc;
//bc.testAllCombinations();
