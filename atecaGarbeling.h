//
// Created by a on 21/02/2023.
//

#ifndef GARBLINGGATES_ATECAGARBELING_H
#define GARBLINGGATES_ATECAGARBELING_H


#include <vector>
#include <string>

using namespace std;
class atecaGarbeling {

public:
    class scheme {
        static string garble(int secParam, const vector<std::string>& circuit);
        static vector<::uint64_t> encode(string randomness, vector<::uint64_t> input);
    private:
        static vector<tuple<vector<::uint64_t>,vector<::uint64_t>>> generateLabels(vector<std::string> circuit, int externalLength);
        static vector<string> garbleCircuit(int externalParam, vector<std::string> circuit, vector<tuple<vector<::uint64_t>,vector<::uint64_t>>> inputLabels);

        //single gate garble
        static vector<vector<uint64_t>> gate(const tuple<vector<::uint64_t>, vector<::uint64_t>>&in0, const tuple<vector<::uint64_t>, vector<::uint64_t>>&in1, const string& typ, int gateNo, int externalParam);
        //projection method

    };

};

#endif //GARBLINGGATES_ATECAGARBELING_H
