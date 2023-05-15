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
        auto amount = 100000;

        vint key = util::genBitsNonCrypto(256);
        vint iv = util::genBitsNonCrypto(256);
        ////
        vint keys = util::genBitsNonCrypto(128);
        vint ivs = util::genBitsNonCrypto(128);
        vector<vint> data(amount);
        vector<halfLabels> dat(amount);
        vector<vint> tweak(amount);
        vector<double> hashvariable_times(label_incs);
        vector<double> tccr_times(label_incs);
        vector<double> prf_times(label_incs);
        vector<double> threehalves(label_incs);
        cout<<"varying label length "<< endl;
        for (int i = 0; i < label_incs; ++i) {
            //cout << "i "<<i<<endl;
            auto labelsize = pow(2,(7+i));
            auto halfLabelSize = pow(2,(6+i));
            for (int j = 0; j < amount; ++j) {
                dat[j]= {util::genBitsNonCrypto(halfLabelSize),util::genBitsNonCrypto(halfLabelSize)};
                vint onedat;
                onedat.insert(onedat.end(), get<0>(dat[j]).begin(), get<0>(dat[j]).end());
                onedat.insert(onedat.end(), get<1>(dat[j]).begin(), get<1>(dat[j]).end());
                data[j]= onedat;
                tweak[j]= util::genBitsNonCrypto(64);
            }

            auto outlen =pow(2,(7+i)); //pow(2,(11+i));
            //cout <<"(" << outlen /pow(2,(i)) << "-bit label)"<<endl;
            cout <<"(" << pow(2,(7+i)) << "-bit label)"<<endl;
            auto t1 = high_resolution_clock::now();
            for (int l = 0; l < amount; ++l) {
                auto Xk = util::hash_variable(data[l],tweak[l], outlen);
            }
            auto t2 = high_resolution_clock::now();
            duration<double, std::milli> ms_double = t2 - t1;
            hashvariable_times[i] += ms_double.count();


            auto tccr = hashTCCR(128);
            t1 = high_resolution_clock::now();
            for (int l = 0; l < amount; ++l) {
                auto Xk = tccr.hash(data[l],data[l],tweak[l],outlen);
            }
            t2 = high_resolution_clock::now();
            ms_double = t2 - t1;
            tccr_times[i] += ms_double.count();


            auto prf = hashTCCR(128);
            t1 = high_resolution_clock::now();
            for (int l = 0; l < amount; ++l) {
                auto Xk = tccr.prfHash(data[l],data[l],tweak[l],outlen);
            }
            t2 = high_resolution_clock::now();
            ms_double = t2 - t1;
            prf_times[i] += ms_double.count();

            auto rtccr = hashRTCCR(key, iv,256);
            t1 = high_resolution_clock::now();
            for (int j = 0; j < amount; ++j) {
                //use index 6 now
                auto res = rtccr.hash(dat[j],tweak[j]);
                //cout<<"rt length "<< res.size()<<endl;
            }
            t2 = high_resolution_clock::now();
            ms_double = t2 - t1;
            threehalves[i] += ms_double.count();

        }

        for (int i = 0; i < label_incs; ++i) {
            cout<< "shake256 "<< hashvariable_times[i]/1000<< " s for "<<  pow(2,(7+i)) << " label length"<<endl;
        }
        for (int i = 0; i < label_incs; ++i) {
            cout<< "tccr     "<< tccr_times[i]/1000<< " s for "<<  pow(2,(7+i)) << " label length"<<endl;
        }
        for (int i = 0; i < label_incs; ++i) {
            cout<< "PRF      "<< prf_times[i]/1000<< " s for "<<  pow(2,(7+i)) << " label length"<<endl;
        }
        for (int i = 0; i < label_incs; ++i) {
            cout<< "rtccr    "<< threehalves[i]/1000<< " s for "<<  pow(2,(7+i)) << " label length"<<endl;
        }

    }

    static void hashOutputLengthTest(){
        using std::chrono::high_resolution_clock;
        using std::chrono::duration_cast;
        using std::chrono::duration;
        using std::chrono::milliseconds;
        auto k= 128;
        vector<double> shake(8);
        vector<double> prf(8);
        vector<double> ateFxor(8);
        vector<double> threeHash(1);
        auto internal = 1000;
        auto external = 100;
        vint key = util::genBitsNonCrypto(256);
        vint iv = util::genBitsNonCrypto(256);

        cout<<internal*external <<" hashings tests"<<endl;
        for (int i = 0; i < external; ++i) {
            vector<vint> data(internal);
            vector<halfLabels> dat(internal);
            vector<vint> tweak(internal);

            for (int j = 0; j < internal; ++j) {
                data[j]= util::genBitsNonCrypto(128);
                dat[i]= {util::genBitsNonCrypto(64),util::genBitsNonCrypto(64)};
                tweak[i]= util::genBitsNonCrypto(3*64);
                //dataAsString[j] = util::uintVec2Str(data[j]);
            }
            //call aes 128->256
            //shake
            for (int j = 0; j < 8; ++j) {
                auto outlen = k*pow(2,j);
                auto t1 = high_resolution_clock::now();
                for (int l = 0; l < internal; ++l) {
                    auto Xk = util::hash_variable(data[l],tweak[l], outlen);
                }
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
                    //use index 6 now
                    auto res = e.prfHash(data[j], key, tweak[j],outlen);
                    //cout<<"aes length "<< res.size()<<endl;
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


            auto rtccr = hashRTCCR(key, iv,256);
            auto t1 = high_resolution_clock::now();
            for (int j = 0; j < internal; ++j) {
                //use index 6 now
                auto res = rtccr.hash(dat[j],tweak[j]);
                //cout<<"rt length "<< res.size()<<endl;
            }
            auto t2 = high_resolution_clock::now();
            duration<double, std::milli> ms_double = t2 - t1;
            threeHash[0] += ms_double.count();




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


    static void repetitionTest(const vector<string>&f, circuit& f2, int k,util::hashtype hashfunc, int repetitions){
        vector<double> baseline(2);
        vector<double> three(2);
        vector<double> ate(2);
        vector<double> ate_f(2);
        vector<vector<::uint64_t>> dataSize({{0,0,0},{0,0,0},{0,0,0},{0,0,0}});
        int inputsize = circuitParser::inputsize(f);


        string hashtype;
        if (hashfunc){
            hashtype = "fast";
        }else{
            hashtype= "RO";
        }
        cout<<"garbling all 4 schemes for aes_128 " << repetitions <<" reps"<<endl;
        cout << "hashtype: "<< hashtype<<endl;
        for (int i = 0; i < repetitions; ++i) {
            //inputgen
            vector<int> x = util::genFunctionInput(inputsize);
            runGarble(f,f2, util::baseline, k, hashfunc, baseline, x, dataSize);
            runGarble(f,f2, util::threehalves, k, hashfunc, three, x, dataSize);
            runGarble(f,f2, util::ateca, k, hashfunc, ate, x, dataSize);
            runGarble(f,f2, util::atecaFXOR, k, hashfunc, ate_f, x, dataSize);
        }

        printResult(util::baseline, baseline, hashfunc, dataSize, repetitions, f2);
        printResult(util::threehalves, three, hashfunc, dataSize, repetitions, f2);
        printResult(util::ateca, ate, hashfunc, dataSize, repetitions, f2);
        printResult(util::atecaFXOR, ate_f, hashfunc, dataSize, repetitions, f2);

    }

    static void printResult(util::scheme scheme, const vector<double> &baseline, util::hashtype hashfunc,vector<vector<::uint64_t>>& dataSize, int repetitions, circuit& f2) {
        string title;
        //all and gates
        auto gates =  6400;
        int hashSize; int Fsize; int decodingsize;
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

        auto outWires = circuitParser::getOutBits(f2);
        cout<< "hashSize: " << hashSize/repetitions << " bits"<<endl;
        cout<< "FSize:    " << (Fsize/gates)/repetitions <<" x" << gates<< " bits"<<endl;
        cout<< "Decoding: " << (decodingsize/outWires)/repetitions <<" x" << outWires<< " bits"<<endl;
        cout<< "total:    " << ((hashSize+Fsize+decodingsize)/repetitions) *0.000125<< " KB"<<endl;
    }

    static void runGarble(const vector<string> &f, circuit& f2, util::scheme type, int k, util::hashtype &hashfunc, vector<double> &timings, vector<int> &x, vector<vector<::uint64_t>> &dataSize) {
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

                for (int i = 0; i < GarbF.size(); ++i) {
                    auto xd = GarbF[i];
                    auto firstVint = get<0>(xd);
                    if (firstVint.size()!=0){
                        dataSize[0][1] += 2*k;
                    }
                }

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
                for (int i = 0; i < three_F.size(); ++i) {
                    if (get<0>(three_F[i]).size()!=0){
                        dataSize[1][1] += 3*(k/2)+8;
                    }
                }
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
                for (int i = 0; i < ate_F.size(); ++i) {
                    for (int j = 0; j < ate_F[i].size(); ++j) {
                        if (ate_F[i][j]!=0){
                            approxkeysize +=64;
                        }
                    }
                }

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
                for (int i = 0; i < atef_F.size(); ++i) {
                    for (int j = 0; j < atef_F[i].size(); ++j) {
                        if (atef_F[i][j]!=0){
                            approxkeysize +=64;
                        }
                    }
                }

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
};


#endif //GARBLINGGATES_TIMING_H