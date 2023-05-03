//
// Created by a on 21/02/2023.
//

#ifndef GARBLINGGATES_ATECAGARBLE_H
#define GARBLINGGATES_ATECAGARBLE_H


//This file implements the grabling scheme described in https://eprint.iacr.org/2021/739
//In this project dubbed ateca garbling.

#include <vector>
#include <string>
#include "../util/util.h"
#include "../util/hashTCCR.h"

using namespace customTypeSpace;


class atecaGarble {

public:
    //garbler public function
    static tuple<vector<vint>, vector<tuple<vint, vint>>, vector<vint>, int, tuple<vint, vint>, hashTCCR>
            garble(const vector<std::string> &f, int k= 128, util::hashtype hashtype= util::RO);
    //Evaluator functions
    static vector<vint> encode(vector<tuple<vint,vint>> e, vector<int> x);
    static vector<vint>
    eval(const vector<vint> &F, const vector<vint> &X, vector<string> C, int k, tuple<vint, vint> invVar,
         hashTCCR &dc);
    static vint decode(vector<vint> Y, vector<vint> d, hashTCCR &dc);

private:
    static vector<tuple<vint,vint>> Init(vector<std::string> C, int k);
    static tuple<vector<vint>, vector<tuple<vint, vint>>, tuple<vint, vint>, hashTCCR>
    GarbleCircuit(int k, vector<std::string> C, vector<tuple<vint, vint>> e, const tuple<vint, vint> &invVar,
                  util::hashtype hashtype);
    //single Gate garble
    static vector<vint>
    Gate(const tuple<vint, vint> &in0, const tuple<vint, vint> &in1, const string &typ, int gateNo, int k,
         hashTCCR &c);
    static vector<vint> DecodingInfo(const vector<tuple<vint, vint>> &D, int k, hashTCCR &c);

    static inline tuple<vint, vint> genInvVar(int k);
    static vint masksForSlices(const vint& X_00, const vint& X_01, const vint& X_10, const vint& X_11, const string& typ);
};

#endif //GARBLINGGATES_ATECAGARBLE_H
