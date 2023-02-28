

#include <boost/lambda/lambda.hpp>
#include <iostream>
#include "bloodcompatibility.h"
#include "util/otUtil/otUtil.h"
#include "util/OTUtil/OT.h"
#include "util/circuitParser.h"
#include "util/util.h"
#include "schemes/baseGarble.h"
#include <bitset>
#include <cstdio>


#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>


using namespace std;

#include <openssl/sha.h>
#include <openssl/aes.h>
#include <openssl/evp.h>


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


    //cout << sha256("1234567890_1") << endl;
    //cout << sha256("1234567890_2") << endl;
    //cout << sha256("1234567890_3") << endl;
    //cout << sha256("1234567890_4") << endl;
    cout<<"xd"<<endl;
    //int aes =  mainAES();
    //auto res = circuitParser::parseCircuit("../tests/circuits/adder64.txt");
    //util::printStrVec(res);

    vector<string> smalltest = {"2 4", "2 1 1", "1 1", "2 1 0 1 2 XOR", "2 1 0 1 3 XOR"};
    auto circuit = baseGarble::garble(128, smalltest);
    vector<int> inputLabels = {0, 0};
    auto X = baseGarble::encode(get<1>(circuit), inputLabels);
    auto Y = baseGarble::eval(circuit, X, smalltest);
    auto y = baseGarble::decode(get<2>(circuit), Y);
    cout << (y.size()) << endl;
    cout << y[0] << endl;


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
