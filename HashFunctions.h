//
// Created by simon on 21/02/2023.
//

#ifndef GARBLINGGATES_HASHFUNCTIONS_H
#define GARBLINGGATES_HASHFUNCTIONS_H
#include <vector>

using namespace std;

class HashFunctions {
public:
    //Security parameter is k and k = l. The hash functions need to take input of 2l bits + gate id
    //Output is l' = 8l = 8k
    vector<uint64_t> newApproachFastHash(int securityParameterK, int gateId);
    vector<uint64_t> newApproachSlowHash(int securityParameterK, int gateId);

    void halfGatesFastHash();
    void halfGatesSlowHash();

    void threeHalvesFastHash();
    void threeHalvesSlowHash();



};


#endif //GARBLINGGATES_HASHFUNCTIONS_H
