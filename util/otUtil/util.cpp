//
// Created by a on 15/02/2023.
//

#include "util.h"
//#include "relic_rand.h"

vector <oc::u64> util::genBitsNonCrypto(int bits) {
    auto res = vector<oc::u64>((bits+64-1)/64);
    for (int blockNum = 0; blockNum <(bits+64-1)/64; ++blockNum) {
        res[blockNum] = random_bitset<64>().to_ullong();
        //for (int i = 0; i < 64; ++i) {bitset[i]=(prng.GenerateBit());}
        //res[blockNum]=bitset.to_ullong();
    }
    return res;
}

string util::printBitsetofVectorofUints(vector<uint64_t> uints){
    string res;
    for (int i = 0; i < uints.size(); ++i) {
        res += bitset<64>(uints[i]).to_string();
    }
    return res;
}


int util::findithBit(vector<uint64_t> ui, int i) {
    int size = ui.size();
    //ith bit
    int bit = i % 64;
    //find block
    int block = i / 64;
    string blockStr = bitset<64>(ui[block]).to_string();
    return blockStr[bit] - '0';
    //for (int j = 0; j < size; ++j) {
    //    string block = bitset<64>(ui[j]).to_string();
    //    uiString += block;
    //}
    ////find number of blocks
    //int block = (i / 64);
    //int bit = i % 64;
    //uint64_t ithblock = ui[block];
    //return bitset<64>(ithblock)[bit];
}