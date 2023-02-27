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
    public:
        //garbler public function
        static tuple<vector<vector<::uint64_t>>,vector<tuple<vector<::uint64_t>,vector<::uint64_t>>>,vector<vector<uint64_t>>,int>
                Gb(int secParam, const vector<std::string>& circuit);
        //Evaluator functions
        static vector<vector<::uint64_t>> encode(vector<tuple<vector<::uint64_t>,vector<::uint64_t>>> encoding, vector<int> input);
        static vector<vector<::uint64_t>> evaluate(const vector<vector<::uint64_t>>& gatesGarbled, const vector<vector<::uint64_t>>& inputGarbled, vector<string>circuit,int secparam);
        static vector<::uint64_t> De(vector<vector<::uint64_t>> outputWires, vector<vector<uint64_t>> d);
        //projection method
        static vector<uint64_t> projection(const vector<::uint64_t>& a,const vector<::uint64_t>& b);
    private:
        static vector<tuple<vector<::uint64_t>,vector<::uint64_t>>> Init(vector<std::string> circuit, int externalLength);
        static tuple<vector<vector<::uint64_t>>,vector<tuple<vector<::uint64_t>,vector<::uint64_t>>>> garbleCircuit(int externalParam, vector<std::string> circuit, vector<tuple<vector<::uint64_t>,vector<::uint64_t>>> inputLabels);
        //single gate Gb
        static vector<vector<uint64_t>> gate(const tuple<vector<::uint64_t>, vector<::uint64_t>>&in0, const tuple<vector<::uint64_t>, vector<::uint64_t>>&in1, const string& typ, int gateNo, int externalParam);
        static vector<vector<uint64_t>> decodingInfo(const vector<tuple<vector<::uint64_t>,vector<::uint64_t>>>&D,int l);
    };

};

#endif //GARBLINGGATES_ATECAGARBELING_H
