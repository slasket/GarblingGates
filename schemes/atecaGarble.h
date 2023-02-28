//
// Created by a on 21/02/2023.
//

#ifndef GARBLINGGATES_ATECAGARBLE_H
#define GARBLINGGATES_ATECAGARBLE_H


//This file implements the grabling scheme described in https://eprint.iacr.org/2021/739
//In this project dubbed ateca garbling.

#include <vector>
#include <string>

using namespace std;
class atecaGarble {

public:
    //garbler public function
    static tuple<vector<vector<::uint64_t>>,vector<tuple<vector<::uint64_t>,vector<::uint64_t>>>,vector<vector<uint64_t>>,int>
            Gb(int l, const vector<std::string>& C);

    static tuple<vector<vector<::uint64_t>>,vector<tuple<vector<::uint64_t>,vector<::uint64_t>>>,vector<vector<uint64_t>>,int,vector<tuple<vector<::uint64_t>,vector<::uint64_t>>>>
    GbLEAK(int l, const vector<std::string>& C);
    //Evaluator functions
    static vector<vector<::uint64_t>> En(vector<tuple<vector<::uint64_t>,vector<::uint64_t>>> encoding, vector<int> input);
    static vector<vector<::uint64_t>> Ev(const vector<vector<::uint64_t>>& F, const vector<vector<::uint64_t>>& X, vector<string>C, int l);
    static vector<::uint64_t> De(vector<vector<::uint64_t>> outputY, vector<vector<uint64_t>> d);
    //projection method
    static vector<uint64_t> projection(const vector<::uint64_t>& a,const vector<::uint64_t>& b);
private:
    static vector<tuple<vector<::uint64_t>,vector<::uint64_t>>> Init(vector<std::string> C, int l);
    static tuple<vector<vector<::uint64_t>>,vector<tuple<vector<::uint64_t>,vector<::uint64_t>>>> GarbleCircuit(int l, vector<std::string> C, vector<tuple<vector<::uint64_t>,vector<::uint64_t>>> encoding);
    //single Gate Gb
    static vector<vector<uint64_t>> Gate(const tuple<vector<::uint64_t>, vector<::uint64_t>>&in0, const tuple<vector<::uint64_t>, vector<::uint64_t>>&in1, const string& typ, int gateNo, int l);
    static vector<vector<uint64_t>> DecodingInfo(const vector<tuple<vector<::uint64_t>,vector<::uint64_t>>>&D, int l);
};

#endif //GARBLINGGATES_ATECAGARBLE_H
