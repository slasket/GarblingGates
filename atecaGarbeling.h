//
// Created by a on 21/02/2023.
//

#ifndef GARBLINGGATES_ATECAGARBELING_H
#define GARBLINGGATES_ATECAGARBELING_H


#include <vector>
#include <string>
#include <cryptoTools/Common/Defines.h>

using namespace std;
class atecaGarbeling {

public:
    class scheme {
        static string garble(int secParam, const vector<std::string>& circuit);
        static vector<::uint64_t> encode(string randomness, vector<::uint64_t> input);
    private:
        static vector<tuple<vector<oc::u64>,vector<oc::u64>>> generateLabels(vector<std::string> circuit, int externalLength);
        static vector<string> garbleCircuit(int externalParam, vector<std::string> circuit, vector<tuple<vector<oc::u64>,vector<oc::u64>>> inputLabels);

        //single gate garble
        static tuple<int,int,int> gate(tuple<vector<oc::u64>, vector<oc::u64>>in0,tuple<vector<oc::u64>, vector<oc::u64>>in1,string type, int gateNo, int externalParam);
        //projection method

    };

};

#endif //GARBLINGGATES_ATECAGARBELING_H
