#include <boost/lambda/lambda.hpp>
#include <iostream>
#include <cryptoTools/Common/Defines.h>
#include "bloodcompatibility.h"
#include "util/OTUtil/util.h"
#include "util/OTUtil/OT.h"
#include "toyExample/toyGarblingScheme.h"
#include <bitset>

using namespace std;

void testBaseOT(int v, int k ,int l, int elgamalKeySize){
    cout<< "#Testing baseOTs from "<< v << " to " << k<< " Amount of OTs"<<endl;

    for (int j = v; j <= k; j=j*2) {
        vector<tuple<vector<oc::u64>,vector<oc::u64>>> senderPairs(j);
        vector<oc::u64> selectionBits(j);
        senderPairs = OT::genKAmountOfSelectionStrings(j, 128);

        selectionBits = util::genBitsNonCrypto(j);

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
            int choicebit = util::findithBit(selectionBits,i);
            choicebits += to_string(choicebit);
            if(choicebit == 0){
                if(util::printBitsetofVectorofUints(result[i]) == util::printBitsetofVectorofUints(get<0>(senderPairs[i])) ){
                    //cout<<"res: "<<result[i]<<endl;
                    //cout<<"par: "<<util::str2bitstr(get<0>(senderPairs[i]))<<endl;
                    correctcounter++;
                    zeroes++;
                } else {
                    //cout<<"res: "<<result[i]<<endl;
                    //cout<<"par: "<<util::str2bitstr(get<0>(senderPairs[i]))<<endl;
                    incorrectcounter++;
                }
            }else{
                if(util::printBitsetofVectorofUints(result[i]) == util::printBitsetofVectorofUints(get<1>(senderPairs[i])) ){
                    //cout<<"res: "<<result[i]<<endl;
                    //cout<<"par: "<<util::str2bitstr(get<1>(senderPairs[i]))<<endl;
                    correctcounter++;
                    ones++;
                } else {
                    //cout<<"res: "<<result[i]<<endl;
                    //cout<<"par: "<<util::str2bitstr(get<1>(senderPairs[i]))<<endl;
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

//testBaseOT(64,512,256,2048);

    bloodcompatibility bc;
    bc.testAllCombinations();

    int k = 1;
    const tuple<int, int, int> &F = toyGarblingScheme::garble(k, 1);
    int e = get<1>(F);
    int d = get<2>(F);
    int X = toyGarblingScheme::encode(e, 1);
    int Y = toyGarblingScheme::eval(get<0>(F), X);
    int y = toyGarblingScheme::decode(d, Y);




    return 0;
}
