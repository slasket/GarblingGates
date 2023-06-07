//
// Created by a on 4/25/23.
//

#ifndef GARBLINGGATES_TIMING_H
#define GARBLINGGATES_TIMING_H

#include <boost/lambda/lambda.hpp>
#include <iostream>
#include <bitset>
#include <chrono>
#include <openssl/sha.h>
#include "util.h"
#include "../schemes/baseGarble.h"
#include "../schemes/threeHalves.h"
#include "../schemes/atecaGarble.h"
#include "../schemes/atecaFreeXOR.h"
#include "circuitParser.h"
#include <boost/timer.hpp>

using namespace std;

class timing{
public:
    static void testLabelLength(int label_incs=7) {
        auto amount = 1000000;

        vint key = util::gen64bitsNonCrypto(128);
        vint iv = util::gen64bitsNonCrypto(128);
        ////
        vint keys = util::gen64bitsNonCrypto(128);
        vint ivs = util::gen64bitsNonCrypto(128);
        vector<vint> data(amount);
        vector<halfLabels> dat(amount);
        vector<vint> tweak(amount);
        vector<double> hashvariable_times(label_incs);
        vector<double> hashvariable_times16(label_incs);
        vector<double> tccr_times(label_incs);
        vector<double> prf_times(label_incs);
        vector<double> threehalves(label_incs);
        cout<<"varying label length "<< endl;
        for (int i = 0; i < label_incs; ++i) {
            //cout << "i "<<i<<endl;
            auto labelsize = pow(2,(7+i));
            auto halfLabelSize = pow(2,(6+i));
            for (int j = 0; j < amount; ++j) {
                dat[j]= {util::gen64bitsNonCrypto(halfLabelSize),util::gen64bitsNonCrypto(halfLabelSize)};
                vint onedat;
                onedat.insert(onedat.end(), get<0>(dat[j]).begin(), get<0>(dat[j]).end());
                onedat.insert(onedat.end(), get<1>(dat[j]).begin(), get<1>(dat[j]).end());
                data[j]= onedat;
                tweak[j]= util::gen64bitsNonCrypto(64);
            }

            auto outlen8X =labelsize*8; //pow(2,(11+i));
            auto outlen16X =labelsize*16; //pow(2,(11+i));
            //cout <<"(" << outlen /pow(2,(i)) << "-bit label)"<<endl;
            cout <<"(" << pow(2,(7+i)) << "-bit label)"<<endl;
            auto t1 = high_resolution_clock::now();
            for (int l = 0; l < amount; ++l) {
                auto Xk = util::hash_variable(data[l],tweak[l], outlen8X);
            }
            auto t2 = high_resolution_clock::now();
            duration<double, std::milli> ms_double = t2 - t1;
            hashvariable_times[i] += ms_double.count();

            t1 = high_resolution_clock::now();
            for (int l = 0; l < amount; ++l) {
                auto Xk = util::hash_variable(data[l],tweak[l], outlen16X);
            }
            t2 = high_resolution_clock::now();
            ms_double = t2 - t1;
            hashvariable_times16[i] += ms_double.count();


            auto tccr = hashTCCR(labelsize);
            t1 = high_resolution_clock::now();
            for (int l = 0; l < amount; ++l) {
                auto Xk = tccr.hash(data[l],data[l],tweak[l],outlen16X);
            }
            t2 = high_resolution_clock::now();
            ms_double = t2 - t1;
            tccr_times[i] += ms_double.count();


            auto prf = hashTCCR(labelsize);
            t1 = high_resolution_clock::now();
            for (int l = 0; l < amount; ++l) {
                auto Xk = tccr.prfHash(data[l],data[l],tweak[l],outlen8X);
            }
            t2 = high_resolution_clock::now();
            ms_double = t2 - t1;
            prf_times[i] += ms_double.count();

            //auto rtccr = hashRTCCR(key, iv,labelsize);
            //t1 = high_resolution_clock::now();
            //for (int j = 0; j < amount; ++j) {
            //    auto res = rtccr.hash(dat[j],tweak[j]);
            //}
            //t2 = high_resolution_clock::now();
            //ms_double = t2 - t1;
            //threehalves[i] += ms_double.count();

        }

        for (int i = 0; i < label_incs; ++i) {
            cout<< "shake256-8x  "<< hashvariable_times[i]/1000<< " s for "<<  pow(2,(7+i)) << " label length"<<endl;
        }
        for (int i = 0; i < label_incs; ++i) {
            cout<< "shake256-16x "<< hashvariable_times16[i]/1000<< " s for "<<  pow(2,(7+i)) << " label length"<<endl;
        }
        for (int i = 0; i < label_incs; ++i) {
            cout<< "tccr(16x)    "<< tccr_times[i]/1000<< " s for "<<  pow(2,(7+i)) << " label length"<<endl;
        }
        for (int i = 0; i < label_incs; ++i) {
            cout<< "PRF(8x)      "<< prf_times[i]/1000<< " s for "<<  pow(2,(7+i)) << " label length"<<endl;
        }
        for (int i = 0; i < label_incs; ++i) {
            cout<< "rtccr        "<< threehalves[i]/1000<< " s for "<<  pow(2,(7+i)) << " label length"<<endl;
        }

    }

    static void hashOutputLengthTest(int a){
        using std::chrono::high_resolution_clock;
        using std::chrono::duration_cast;
        using std::chrono::duration;
        using std::chrono::milliseconds;
        auto k= a;
        vector<double> shake(10);
        vector<double> prf(10);
        vector<double> ateFxor(10);
        vector<double> threeHash(1);
        auto internal = 1000;
        auto external = 1000;
        auto extexternal = 1;
        vint key = util::gen64bitsNonCrypto(128);
        vint iv = util::gen64bitsNonCrypto(128);

        cout<<internal*external*extexternal <<" hashings tests"<<endl;
        vector<vint> data(internal);
        vector<halfLabels> dat(internal);
        vector<vint> tweak(internal);
        for (int o = 0; o < extexternal; ++o) {
        for (int i = 0; i < external; ++i) {
            for (int j = 0; j < internal; ++j) {
                data[j]= util::gen64bitsNonCrypto(k);
                dat[i]= {util::gen64bitsNonCrypto(k/2),util::gen64bitsNonCrypto(k/2)};
                tweak[i]= util::gen64bitsNonCrypto(k);
            }
            //call aes 128->256
            //shake
            for (int j = 0; j < 8; ++j) {
                auto outlen = k*pow(2,j);
                auto t1 = high_resolution_clock::now();
                for (int l = 0; l < internal; ++l) {
                    auto Xk = util::hash_variable(data[l],tweak[l], outlen);}
                auto t2 = high_resolution_clock::now();
                duration<double, std::milli> ms_double = t2 - t1;
                shake[j] += ms_double.count();
            }

            //prf
            auto e = hashTCCR(k);
            for (int j = 0; j < 8; ++j) {
                auto outlen = k*pow(2,j);
                auto t1 = high_resolution_clock::now();
                for (int j = 0; j < internal; ++j) {
                    auto res = e.prfHash(data[j], key, tweak[j],outlen);
                }
                auto t2 = high_resolution_clock::now();
                duration<double, std::milli> ms_double = t2 - t1;
                prf[j] += ms_double.count();

            }

            auto tccr = hashTCCR(k);
            for (int j = 0; j < 8; ++j) {
                auto outlen = k*pow(2,(j));
                auto t1 = high_resolution_clock::now();
                for (int l = 0; l < internal; ++l) {
                    auto Xk = tccr.hash(data[l],data[l],tweak[l],outlen);
                }
                auto t2 = high_resolution_clock::now();
                duration<double, std::milli> ms_double = t2 - t1;
                ateFxor[j] += ms_double.count();
            }


            auto rtccr = hashRTCCR(key, iv,128);
            auto t1 = high_resolution_clock::now();
            for (int j = 0; j < internal; ++j) {
                auto res = rtccr.hash(dat[j],tweak[j]);
            }
            auto t2 = high_resolution_clock::now();
            duration<double, std::milli> ms_double = t2 - t1;
            threeHash[0] += ms_double.count();
        }
        }
        for (int i = 0; i < 8; ++i) {
            auto outlen = k*pow(2,i);
            cout << "shake256 " << shake[i] / 1000 << " s " << outlen << "-bit (" << pow(2,i) << "x) output"<<  endl;
        }
        for (int i = 0; i < 8; ++i) {
            auto outlen = k*pow(2,(i));
            cout << "TCCR     " << ateFxor[i] / 1000 << " s "<< outlen << "-bit (" << pow(2,(i)) << "x) output"<< endl;
        }
        for (int i = 0; i < 8; ++i) {
            auto outlen = k*pow(2,(i));
            cout << "PRF      " << prf[i] / 1000 << " s "<< outlen << "-bit (" << pow(2,(i)) << "x) output"<< endl;
        }

            cout << "RTCCR    " << threeHash[0] / 1000 << " s " << "128-bit output" << endl;


    }


    static void repetitionTest(circuit& f2, int k,util::hashtype hashfunc, int repetitions){
        vector<double> baseline(2);
        vector<double> three(2);
        vector<double> ate(2);
        vector<double> ate_f(2);
        vector<vector<::uint64_t>> dataSize({{0,0,0},{0,0,0},{0,0,0},{0,0,0}});
        int inputsize = circuitParser::getInputSize(f2);


        string hashtype;
        if (hashfunc){
            hashtype = "fast";
        }else{
            hashtype= "RO";
        }
        cout<<"4 schemes for aes_128 " << repetitions <<"0 reps hashtype: "<< hashtype<< " labelsize "<< k<<endl;
        for (int i = 0; i < repetitions; ++i) {
            //inputgen
            vector<int> x = util::genFunctionInput(inputsize);
            runGarble(f2, util::baseline, k, hashfunc, baseline, x, dataSize);
            runGarble(f2, util::threehalves, k, hashfunc, three, x, dataSize);
            runGarble(f2, util::ateca, k, hashfunc, ate, x, dataSize);
            runGarble(f2, util::atecaFXOR, k, hashfunc, ate_f, x, dataSize);
        }

        printResult(util::baseline, baseline, hashfunc, dataSize, repetitions, f2);
        printResult(util::threehalves, three, hashfunc, dataSize, repetitions, f2);
        printResult(util::ateca, ate, hashfunc, dataSize, repetitions, f2);
        printResult(util::atecaFXOR, ate_f, hashfunc, dataSize, repetitions, f2);

    }

    static void printResult(util::scheme scheme, const vector<double> &baseline, util::hashtype hashfunc,vector<vector<::uint64_t>>& dataSize, int repetitions, circuit& f2) {
        string title;
        //all and gates
        auto gates = 19626;
        ::uint64_t hashSize; uint64_t Fsize; uint64_t decodingsize;
        switch (scheme) {
            case util::baseline: {
                title = "###baseline###";
                hashSize = dataSize[0][0];
                Fsize = dataSize[0][1];
                decodingsize = dataSize[0][2];
                break;
            }
            case util::threehalves: {
                title = "###threehalves###";
                hashSize = dataSize[1][0];
                Fsize = dataSize[1][1];
                decodingsize = dataSize[1][2];
                break;
            }
            case util::ateca: {
                title = "###ateca###";
                hashSize = dataSize[2][0];
                Fsize = dataSize[2][1];
                decodingsize = dataSize[2][2];
                gates = circuitParser::getGates(f2);
                break;
            }
            case util::atecaFXOR: {
                title = "###atecaFXOR###";
                hashSize = dataSize[3][0];
                Fsize = dataSize[3][1];
                decodingsize = dataSize[3][2];
                break;
            }
        }
        cout << title << endl;
        vector<string> categories = {"garble", "eval"};
        for (int i = 0; i < 2; ++i) {
            cout << categories[i] << " " << baseline[i] << " s" << endl;

        }

        //auto outWires = circuitParser::getOutBits(f2);
        //cout<< "hashSize: " << hashSize/repetitions << " bits"<<endl;
        //cout<< "FSize:    " << (Fsize/gates)/repetitions <<" x" << gates<< " bits"<<endl;
        //cout<< "Decoding: " << (decodingsize/outWires)/repetitions <<" x" << outWires<< " bits"<<endl;
        //cout<< "total:    " << ((hashSize+Fsize+decodingsize)/repetitions) *0.000125<< " KB"<<endl;
    }

    static void runGarble(circuit& f2, util::scheme type, int k, util::hashtype &hashfunc, vector<double> &timings, vector<int> &x, vector<vector<::uint64_t>> &dataSize) {
        switch (type) {
            case util::scheme::baseline:{
                boost::timer timer;
                auto [F,e,d] = baseGarble::garble(f2, k, hashfunc);//needs hash type
                auto hash = get<2>(F);
                timings[0] += timer.elapsed();
                auto base_X = baseGarble::encode(e, x);
                timer.restart();
                auto base_Y = baseGarble::eval(F, base_X, f2, k);
                timings[1] += timer.elapsed();
                auto base_y = baseGarble::decode(d, base_Y, k, hash);

                auto [Inv,GarbF,h] = F;
                auto size = h.getU1().size();
                size += h.getU2().size();
                size += h.getAlpha().size();
                size += h.getIv().size();
                //size += h.getKey().size();
                //size += sizeof(h.getE());
                //the E struct is 512 bits in size
                dataSize[0][0] += size*64;

                //for (int i = 0; i < GarbF.size(); ++i) {
                //    auto xd = GarbF[i];
                //    auto firstVint = get<0>(xd);
                //    if (firstVint.size()!=0){
                //        dataSize[0][1] += 2*k;
                //    }
                //}

                auto dtuples = d.size();
                dataSize[0][2] += dtuples*2*k;

                break;
            }
            case util::threehalves:{
                boost::timer timer;
                auto [three_F,three_e,three_d, three_ic, three_hash] = threeHalves::garble(f2, k, hashfunc);
                timings[0] += timer.elapsed();
                auto three_X = threeHalves::encode(three_e, x);

                timer.restart();
                auto three_Y = threeHalves::eval(three_F, three_X, f2, k, three_ic, three_hash, hashfunc);
                timings[1] += timer.elapsed();
                auto three_y = threeHalves::decode(three_d, three_Y, f2, k);

                auto h = three_hash;
                auto size = h.getU1().size();
                size += h.getU2().size();
                size += h.getAlpha().size();
                size += h.getIv().size();
                //size += h.getKey().size();
                //size += sizeof(h.getE());
                //the E struct is 512 bits in size
                dataSize[1][0] += size*64;

                auto tuples = three_F.size();
                //for (int i = 0; i < three_F.size(); ++i) {
                //    if (get<0>(three_F[i]).size()!=0){
                //        dataSize[1][1] += 3*(k/2)+8;
                //    }
                //}
                //the unit8 should be  sin single value lol.

                auto dtuples = three_d.size();
                dataSize[1][2] += dtuples*2*k;

                break;
            }
            case util::ateca:{
                boost::timer timer;
                auto [ate_F, ate_e, ate_d, ate_k, ate_ic, ate_hash] = atecaGarble::garble(f2, k, hashfunc);
                timings[0] += timer.elapsed();

                auto ate_X = atecaGarble::encode(ate_e, x);
                timer.restart();
                auto ate_Y = atecaGarble::eval(ate_F, ate_X, f2, ate_k, ate_ic, ate_hash);
                timings[1] += timer.elapsed();
                auto ate_y = atecaGarble::decode(ate_Y, ate_d, ate_hash);



                auto h = ate_hash;
                //auto size = h.getU1().size();
                //size += h.getU2().size();
                auto size = h.getIv().size();
                //size += h.getKey().size();
                //size += sizeof(h.getE());
                //the E struct is 512 bits in size
                dataSize[2][0] += size*64;

                auto approxkeysize =0;
                //for (int i = 0; i < ate_F.size(); ++i) {
                //    for (int j = 0; j < ate_F[i].size(); ++j) {
                //        if (ate_F[i][j]!=0){
                //            approxkeysize +=64;
                //        }
                //    }
                //}

                dataSize[2][1] += approxkeysize;

                auto decodingVals = ate_d.size();
                dataSize[2][2] += decodingVals*k;

                break;
            }
            case util::atecaFXOR:{
                boost::timer timer;
                auto [atef_F, atef_e, atef_d, atef_k, atef_ic, atef_hash] = atecaFreeXOR::garble(f2, k, hashfunc);
                timings[0] += timer.elapsed();

                auto atef_X = atecaFreeXOR::encode(atef_e, x);
                timer.restart();
                auto atef_Y = atecaFreeXOR::eval(atef_F, atef_X, f2, atef_k, atef_ic, atef_hash);
                timings[1] += timer.elapsed();
                auto atef_y = atecaFreeXOR::decode(atef_Y, atef_d, atef_hash);

                auto h = atef_hash;
                auto size = h.getU1().size();
                size += h.getU2().size();
                size += h.getIv().size();
                //size += h.getKey().size();
                size += sizeof(h.getE());
                //the E struct is 512 bits in size
                dataSize[3][0] += size*64;

                auto approxkeysize =0;
                //for (int i = 0; i < atef_F.size(); ++i) {
                //    for (int j = 0; j < atef_F[i].size(); ++j) {
                //        if (atef_F[i][j]!=0){
                //            approxkeysize +=64;
                //        }
                //    }
                //}

                dataSize[3][1] += approxkeysize;

                auto decodingVals = atef_d.size();
                dataSize[3][2] += decodingVals*k;

                break;
            }
        }
    }

    static void time_circuit_all(const vector<string>&f, circuit& f2, const vector<int>& x, int k, util::hashtype hashfunc){
        if (hashfunc){
            cout<<"windows fast"<<endl;
        }else{
            cout<<"windows slow"<<endl;
        }
        //cout<< "Keccak_f test"<<endl;
        timing::time_circuit(f,f2,x,k,util::baseline, hashfunc);
        timing::time_circuit(f,f2,x,k,util::threehalves, hashfunc);
        timing::time_circuit(f,f2,x,k,util::ateca, hashfunc);
        timing::time_circuit(f,f2,x,k,util::atecaFXOR, hashfunc);

    }

    static void time_circuit(const vector<string>&f, circuit& f2, const vector<int>& x, int k, util::scheme type, util::hashtype hashfunc){
        using std::chrono::high_resolution_clock;
        using std::chrono::duration_cast;
        using std::chrono::duration;
        using std::chrono::milliseconds;
        string title;
        string hashtype;
        if (hashfunc){
            hashtype = "fast";
        }else{
            hashtype= "RO";
        }

        switch (type) {
            case util::scheme::baseline:{
                title = "baseline w. ";
                cout<<title << hashtype<<endl;
                auto t1 = high_resolution_clock::now();
                auto [F,e,d] = baseGarble::garble(f2, k, hashfunc);//needs hash type

                auto t2 = high_resolution_clock::now();
                duration<double, std::milli> ms_double = t2 - t1;
                cout<< "garbling: " <<ms_double.count()<< "ms"<<endl;
                auto hash = get<2>(F);
                auto base_X = baseGarble::encode(e, x);
                t1 = high_resolution_clock::now();
                auto base_Y = baseGarble::eval(F, base_X, f2, k);
                t2 = high_resolution_clock::now();
                ms_double = t2 - t1;
                cout<< "evaluation: " <<ms_double.count()<< "ms"<<endl;
                auto base_y = baseGarble::decode(d, base_Y, k, hash);
                cout<< "base: " << base_y[0] <<endl;
                break;
            }
            case util::scheme::threehalves:{
                title = "three halves w. ";
                cout<<title << hashtype<<endl;
                auto t1 = high_resolution_clock::now();
                auto [three_F,three_e,three_d, three_ic, three_hash] = threeHalves::garble(f2, k, hashfunc);
                auto t2 = high_resolution_clock::now();
                duration<double, std::milli> ms_double = t2 - t1;
                cout<< "garbling: " <<ms_double.count()<< "ms"<<endl;

                auto three_X = threeHalves::encode(three_e, x);

                t1 = high_resolution_clock::now();
                auto three_Y = threeHalves::eval(three_F, three_X, f2, k, three_ic, three_hash, hashfunc);
                t2 = high_resolution_clock::now();
                ms_double = t2 - t1;
                cout<< "evaluation: " <<ms_double.count()<< "ms"<<endl;

                auto three_y = threeHalves::decode(three_d, three_Y, f2, k);
                cout<< "three: " << three_y[0] <<endl;
                break;
            }
            case util::scheme::ateca:{
                title = "ateca w. ";
                cout<<title << hashtype<<endl;
                auto t1 = high_resolution_clock::now();
                auto [ate_F, ate_e, ate_d, ate_k, ate_ic, ate_hash] = atecaGarble::garble(f2, k, hashfunc);
                auto t2 = high_resolution_clock::now();
                duration<double, std::milli> ms_double = t2 - t1;
                cout<< "garbling: " <<ms_double.count()<< "ms"<<endl;

                cout<<"bits in f: "<< util::averageFweight(ate_F)<<endl;

                auto ate_X = atecaGarble::encode(ate_e, x);

                t1 = high_resolution_clock::now();
                auto ate_Y = atecaGarble::eval(ate_F, ate_X, f2, ate_k, ate_ic, ate_hash);
                t2 = high_resolution_clock::now();
                ms_double = t2 - t1;
                cout<< "evaluation: " <<ms_double.count()<< "ms"<<endl;

                auto ate_y = atecaGarble::decode(ate_Y, ate_d, ate_hash);
                cout<< "ateca: " << ate_y[0] <<endl;
                break;
            }
            case util::scheme::atecaFXOR:{
                title = "ateca-Freexor w. ";
                cout<<title << hashtype<<endl;
                auto t1 = high_resolution_clock::now();
                auto [atef_F, atef_e, atef_d, atef_k, atef_ic, atef_hash] = atecaFreeXOR::garble(f2, k, hashfunc);
                auto t2 = high_resolution_clock::now();
                duration<double, std::milli> ms_double = t2 - t1;
                cout<< "garbling: " <<ms_double.count()<< "ms"<<endl;
                cout<<"bits in f: "<< util::averageFweight(atef_F)<<endl;

                auto atef_X = atecaFreeXOR::encode(atef_e, x);

                t1 = high_resolution_clock::now();
                auto atef_Y = atecaFreeXOR::eval(atef_F, atef_X, f2, atef_k, atef_ic, atef_hash);
                t2 = high_resolution_clock::now();
                ms_double = t2 - t1;
                cout<< "evaluation: " <<ms_double.count()<< "ms"<<endl;

                auto atef_y = atecaFreeXOR::decode(atef_Y, atef_d, atef_hash);

                cout<< "atecaFxor: " << atef_y[0] <<endl;
                break;
            }
            default:
                break;
        }
    }

    static void testSchemesVariableLabelSize(int repetitions) {
        auto c = circuitParser::parse("../tests/circuits/aes_128.txt");
        int inputsize = circuitParser::getInputSize(c);
        auto k = 128;
        auto type = util::RO;
        cout<< "varing label size for aes_128 using RO ";
        for (int i = 0; i < 11; ++i) {
            vector<double> baseline(2);
            vector<double> three(2);
            vector<double> ate(2);
            vector<double> ate_f(2);
            vector<vector<::uint64_t>> dataSize({{0,0,0},{0,0,0},{0,0,0},{0,0,0}});
            cout << "#inc;"<<pow(2,i)<<"x128" <<endl;
            int labelsize = pow(2,(7+i));
            for (int j = 0; j < repetitions; ++j) {
                vector<int> x = util::genFunctionInput(inputsize);
                runGarble(c, util::baseline, labelsize, type, baseline, x, dataSize);
                runGarble(c, util::threehalves, labelsize, type, three, x, dataSize);
                runGarble(c, util::ateca, labelsize, type, ate, x, dataSize);
                runGarble(c, util::atecaFXOR, labelsize, type, ate_f, x, dataSize);
            }

            printGarblinTime(util::baseline, baseline, type, dataSize, repetitions, c);
            printGarblinTime(util::threehalves, three, type, dataSize, repetitions, c);
            printGarblinTime(util::ateca, ate, type, dataSize, repetitions, c);
            printGarblinTime(util::atecaFXOR, ate_f, type, dataSize, repetitions, c);

        }
    }

    static void printGarblinTime(util::scheme scheme, const vector<double> &baseline, util::hashtype hashfunc,vector<vector<::uint64_t>>& dataSize, int repetitions, circuit& f2) {
        string title;
        //all and gates
        auto gates = 19626;
        ::uint64_t hashSize; uint64_t Fsize; uint64_t decodingsize;
        switch (scheme) {
            case util::baseline: {
                title = "base;";
                hashSize = dataSize[0][0];
                Fsize = dataSize[0][1];
                decodingsize = dataSize[0][2];
                break;
            }
            case util::threehalves: {
                title = "three;";
                hashSize = dataSize[1][0];
                Fsize = dataSize[1][1];
                decodingsize = dataSize[1][2];
                break;
            }
            case util::ateca: {
                title = "ateca;";
                hashSize = dataSize[2][0];
                Fsize = dataSize[2][1];
                decodingsize = dataSize[2][2];
                gates = circuitParser::getGates(f2);
                break;
            }
            case util::atecaFXOR: {
                title = "atecaFxor;";
                hashSize = dataSize[3][0];
                Fsize = dataSize[3][1];
                decodingsize = dataSize[3][2];
                break;
            }
        }
        cout << title << baseline[0]<< endl;
    }

    static void datagentime() {
        auto xd = 10000000;
        vector<double> time(2);

        auto t1 = high_resolution_clock::now();
        for (int l = 0; l < xd; ++l) {
            util::genBitsNonCrypto(128);
        }
        auto t2 = high_resolution_clock::now();
        duration<double, std::milli> ms_double = t2 - t1;
        time[0] += ms_double.count();

        t1 = high_resolution_clock::now();
        for (int l = 0; l < xd; ++l) {
            util::gen64bitsNonCrypto(128);
        }
        t2 = high_resolution_clock::now();
        ms_double = t2 - t1;
        time[1] += ms_double.count();

        cout<<"old: "<< time[0] <<" new: "<< time[1];
    }
};


#endif //GARBLINGGATES_TIMING_H