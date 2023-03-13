//
// Created by a on 10/03/2023.
//

#ifndef GARBLINGGATES_ATECAFREEXOR_H
#define GARBLINGGATES_ATECAFREEXOR_H

//This file implements the grabling scheme described in https://eprint.iacr.org/2021/739
//In this project dubbed ateca garbling.

#include <vector>
#include <string>
#include "../util/util.h"
using namespace customTypeSpace;

class atecaFreeXOR {

public:
    //garbler public function
    static tuple<vector<vint>, vector<tuple<vint,vint>>, vector<vint>, int, tuple<vint,vint>,string>
    Gb(int l, const vector<std::string>& C, string hashtype);

    //Evaluator functions
    static vector<vint> En(vector<tuple<vint,vint>> encoding, vector<int> input);
    static vector<vint> Ev(const vector<vint>& F, const vector<vint>& X, vector<string>C, int l, tuple<vint,vint> invVar);
    static vint De(vector<vint> outputY, vector<vint> d);


private:
    static tuple<vector<tuple<vint,vint>>,vint> Init(vector<std::string> C, int l);
    static tuple<vector<vint>,vector<tuple<vint,vint>>, tuple<vint,vint>> GarbleCircuit(int l, vector<std::string> C, vector<tuple<vint,vint>> encoding, const tuple<vint,vint>& invVar, vint globalDelta);
    //single Gate Gb
    static vector<vint> Gate(const tuple<vint, vint>&in0, const tuple<vint, vint>&in1, const string& typ, int gateNo, int l, vint globalDelta);
    static vector<vint> DecodingInfo(const vector<tuple<vint,vint>>&D, int l);

    static tuple<vint, vint> genInvVar(int l, vint globalDelta);

};


#endif //GARBLINGGATES_ATECAFREEXOR_H
