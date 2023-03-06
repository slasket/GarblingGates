

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


void testsubAteca();

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




int main() {
    testsubAteca();


    return 0;

}

void testsubAteca() {
    //least significant bit first :^)
    auto finput = vector<int>{0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                              0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                              0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                              0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    auto C = circuitParser::parseCircuit("../tests/circuits/sub64.txt");
    cout<<"garbling"<<endl;
    auto feds = atecaGarble::Gb(64, C);
    cout<<"encoding"<<endl;
    auto encodedInput = atecaGarble::En(get<1>(feds), finput);
    cout<<"eval"<<endl;
    auto Y = atecaGarble::Ev(get<0>(feds), encodedInput, C, get<3>(feds),get<4>(feds));
    cout<<"decoding"<<endl;
    auto y = atecaGarble::De(Y, get<2>(feds));
    util::printUintVec(y);
}

//util::printCircuit("../tests/circuits/BloodComp.txt");
//testBaseOT(64,512,256,2048);
//bloodcompatibility bc;
//bc.testAllCombinations();
