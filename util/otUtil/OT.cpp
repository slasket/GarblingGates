//
// Created by a on 15/02/2023.
//

#include "OT.h"
#include "util.h"
#include "elgamal.h"

tuple<Integer, Integer,Integer>* OT::Alice::genPKArray(int keySize, Integer mod, Integer g) {
    const tuple<Integer, Integer> &keyValues = elgamal::KeyGen(keySize, mod, g);

    //save private key to alice
    privateKey.Initialize(mod, g, get<1>(keyValues));

    //ElGamal::PublicKey publicKey;
    //publicKey.AssignFrom(privateKey);

    Integer h = get<0>(keyValues);

    tuple<Integer, Integer,Integer> ogenVals = elgamal::OGen(mod, g, keySize);

    auto* pkArr = new tuple<Integer, Integer,Integer>[2];
    if (bitVal == 0){
        pkArr[0] = {mod, g, h};
        pkArr[1] = {get<0>(ogenVals),get<1>(ogenVals) ,get<2>(ogenVals)};

    } else{
        pkArr[0] = {get<0>(ogenVals),get<1>(ogenVals) ,get<2>(ogenVals)};
        pkArr[1] = {mod, g,h};
    }
    return pkArr;
}

vector<uint64_t> OT::Alice::receiveCipherArr(std::string* cpArr) {
    Integer mod = privateKey.GetGroupParameters().GetModulus();
    Integer g = privateKey.GetGroupParameters().GetGenerator();
    Integer x = privateKey.GetPrivateExponent();

    auto msg = elgamal::Decrypt(cpArr[bitVal], mod, g, x);

    uint64_t highBits = stoull(msg.substr(0, 64), nullptr, 2);
    uint64_t lowBits = stoull(msg.substr(64, 64), nullptr, 2);
    vector<uint64_t> res = {highBits, lowBits};
    return res;

}
vector<tuple<vector<oc::u64>,vector<oc::u64>>> OT::genKAmountOfSelectionStrings(int k,int bitAmount){
    auto receiverPairs = vector<tuple<vector<oc::u64>,vector<oc::u64>>>(k);
    for (int i = 0; i < k; ++i) {
        auto leftVal = util::genBitsNonCrypto(bitAmount);
        auto rightVal = util::genBitsNonCrypto(bitAmount);
        receiverPairs[i] = {leftVal,rightVal};
    }
    return receiverPairs;
}

string* OT::Bob::receivePKArray(tuple<Integer, Integer,Integer> *pkArray) {

    string* cipherArr= new string[2];

    auto highBitsStr0 = bitset<64>(str0[0]).to_string();
    auto lowBitsStr0 = bitset<64>(str0[1]).to_string();
    auto highBitsStr1 = bitset<64>(str1[0]).to_string();
    auto lowBitsStr1 = bitset<64>(str1[1]).to_string();
    string stringToEncrypt0 = highBitsStr0 + lowBitsStr0;
    string stringToEncrypt1 = highBitsStr1 + lowBitsStr1;
    cipherArr[0] = elgamal::Encrypt(stringToEncrypt0, get<0>(pkArray[0]), get<1>(pkArray[0]), get<2>(pkArray[0]));
    cipherArr[1] = elgamal::Encrypt(stringToEncrypt1, get<0>(pkArray[1]), get<1>(pkArray[1]), get<2>(pkArray[1]));

    return cipherArr;
}

vector<uint64_t> OT::OT1out2(int keySize, const Integer& mod, const Integer& g, int choicebit, const vector<uint64_t>& string0, const vector<uint64_t>& string1) {
    Alice alice(choicebit);
    Bob bob(string0, string1);

    auto pkarr = alice.genPKArray(keySize, mod, g);
    string* cipherArr = bob.receivePKArray(pkarr);

    return alice.receiveCipherArr(cipherArr);
}


vector<uint64_t>* OT::BaseOTTest(int const elgamalkeysize, int amountOfOTs, vector<tuple<vector<uint64_t>, vector<uint64_t>>> recPairs, vector<uint64_t> choiceBits){
    vector<uint64_t> OTChoiceBits = std::move(choiceBits);
    //R chooses k pairs of k-bit seeds
    auto receiverPairs = std::move(recPairs);
    //Init group parameters
    //auto groupParaKey = elgamal::InitializeGroupParameters(elgamalkeysize);
    //Integer mod = groupParaKey.GetGroupParameters().GetModulus();
    //Integer g = groupParaKey.GetGroupParameters().GetGenerator();
    Integer mod("27116049191505256263784855523797723975553564921607276577493997526291631279569122369847027536544391667432461448615575698337249152963126647280933395355596621192802732199252815662005480533690702737050115137493783942222614822345494204540888976090325814282122923034331559601288394898636347801746441337326774229495633839205081506187176586728440053775397129715038076224311504937586111691274934414582880412809681674266766531332848541880387464807922606949659251991951744452694321879285702283721136336698590539390027917128792587102374187765081984383149973451168741005071390186909346140945857286033586249386098657878796426911023");
    Integer g = 2;

    //kXOTk functionality
    auto* kresults = new vector<uint64_t> [amountOfOTs];
    for (int i = 0; i < amountOfOTs; ++i) {
        int senderChoiceBit = util::findithBit(OTChoiceBits,i);
        vector<uint64_t> receivedKey = OT1out2(elgamalkeysize, mod, g, senderChoiceBit, get<0>(receiverPairs[i]), get<1>(receiverPairs[i]));
        kresults[i] = receivedKey;
    }
    return kresults;

}

int OT::findUIntBit(int idx, const vector<uint64_t>& uint) {
    auto theInt= idx / 64;
    auto smallerIdx = idx % 64;

    oc::u64 theInteger = uint[theInt];
    return bitset<64>(theInteger)[smallerIdx];
}
