/*
//
// Created by a on 15/02/2023.
//

#ifndef GARBLINGGATES_OT_H
#define GARBLINGGATES_OT_H


#include <cstdint>
//#include <cryptoTools/Common/Defines.h>
#include <cryptopp/elgamal.h>

using namespace CryptoPP;
using namespace std;
class OT{

public:
    class Alice{
        int bitVal;
        ElGamal::PrivateKey privateKey;
        public:
        explicit Alice(int decisionBit){
            bitVal = decisionBit;
        };

        vector<uint64_t> receiveCipherArr(string *cpArr);

        tuple<Integer, Integer, Integer> *genPKArray(int keySize, Integer mod, Integer g);

    };
    class Bob{
        vector<uint64_t > str0;
        vector<uint64_t > str1;
        public:
        explicit Bob(const vector<uint64_t>& string0, const vector<uint64_t>& string1){
            str0 = string0;
            str1 = string1;
        };

        //receive public key arr and encrypt l-bit strings
        string* receivePKArray(tuple<Integer, Integer,Integer> pkArray[]);
    };

    ::uint64_t a;

    static vector<uint64_t> OT1out2(int keySize, const Integer& mod, const Integer& g, int choicebit, const vector<uint64_t>& string0, const vector<uint64_t>& string1);

    static int findUIntBit(int idx, const vector<uint64_t>& uint);

    static vector<uint64_t>* BaseOTTest(const int elgamalkeysize, int amountOfOTs, vector<tuple<vector<uint64_t>, vector<uint64_t>>> recPairs, vector<uint64_t> choiceBits);

    static vector<tuple<vector<::uint64_t>,vector<::uint64_t>>>
    genKAmountOfSelectionStrings(int keysize, int bitAmount);
};


#endif //GARBLINGGATES_OT_H
*/
