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
#include "../util/hashTCCR.h"

using namespace customTypeSpace;

class atecaFreeXOR {

public:
    //garbler public function
    static tuple<vector<vint>, vector<tuple<vint, vint>>, vector<vint>, int, tuple<vint, vint>, hashTCCR>
    garble(vector<tuple<vector<int>, vector<int>, string>> &f, int k= 128, util::hashtype hashtype= util::RO);

    //Evaluator functions
    static vector<vint> encode(vector<tuple<vint,vint>> e, vector<int> x);
    static vector<vint>
    eval(const vector<vint> &F, const vector<vint> &X, vector<tuple<vector<int>, vector<int>, string>> &C, int k, tuple<vint, vint> invVar,
         hashTCCR &c);
    static vint decode(vector<vint> Y, vector<vint> d, hashTCCR &dc);


private:
    static tuple<vector<tuple<vint,vint>>,vint> Init(vector<tuple<vector<int>, vector<int>, string>> &C, int k);
    static tuple<vector<vint>, vector<tuple<vint, vint>>, tuple<vint, vint>, hashTCCR>
    GarbleCircuit(int k, vector<tuple<vector<int>, vector<int>, string>> &C, vector<tuple<vint, vint>> encoding,
                  const tuple<vint, vint> &invVar, const vint &globalDelta, util::hashtype hashtype);
    //single Gate garble
    static vector<vint>
    Gate(const tuple<vint, vint> &in0, const tuple<vint, vint> &in1, int gateNo, int k,
         const vint &globalDelta, hashTCCR &c);
    static vector<vint>
    DecodingInfo(const vector<tuple<vint, vint>> &D, int k, hashTCCR &c);

    static tuple<vint, vint> genInvVar(int k, vint globalDelta);

    static tuple<vint, vint> ateFXorSlicing(const vint& X_00, const vint& X_01, const vint& X_10, const vint& X_11);

    static int ateFXORSliceCheck(const vint &globalDelta, const vint& d0flags, const vint& d1flags, int hw, int j);

    static void
    invGate(const tuple<vint, vint> &invVar, const vector<tuple<vint, vint>> &wires, vector<vint> &garbledGate,
            int &out,
            vector<int> &inWires, vector<int> &outWires);


    static void xorGate(vector<tuple<vint, vint>> &wires, vector<vint> &garbledGate, int &out, vector<int> &inWires,
                        vector<int> &outWires);

    static void
    andGate(int k, const vint &globalDelta, const vector<tuple<vint, vint>> &wires, int gateNo, hashTCCR &c,
            vector<vint> &F,
            vector<vint> &garbledGate, int &out, vector<int> &inWires, vector<int> &outWires);
};


#endif //GARBLINGGATES_ATECAFREEXOR_H
