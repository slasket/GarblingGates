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
    static void time_AES_V_ourImpl(int k=256) {
        auto amount = 1000000;
        vint key = util::genBitsNonCrypto(256);
        vint iv = util::genBitsNonCrypto(256);
        ////
        vint keys = util::genBitsNonCrypto(128);
        vint ivs = util::genBitsNonCrypto(128);
        vector<vint> data(amount);
        vector<halfLabels> dat(amount);
        vector<vint> tweak(amount);

        for (int i = 0; i < amount; ++i) {
            data[i]= util::genBitsNonCrypto(128);
            dat[i]= {util::genBitsNonCrypto(64),util::genBitsNonCrypto(64)};
            tweak[i]= util::genBitsNonCrypto(3*64);
        }


        auto e123 = hashRTCCR::AES_enc_init(key,iv);
        boost::timer timer;
        for (int j = 0; j < amount; ++j) {
            auto res = hashRTCCR::AES_enc(data[j],e123);
        }
        cout<<"aes alone "<<timer.elapsed()<<endl;

        auto e = hashRTCCR(key,iv,k);

        timer.restart();
        for (int j = 0; j < amount; ++j) {
            //use index 6 now
            auto res = hashRTCCR::hash(dat[j],tweak[j],e.getKey(),e.getIv(),e.getE(),e.getAlpha(),e.getU1(),e.getU2());
        }
        cout<<"our impl "<<timer.elapsed()<<endl;
    }

    static void hashOutputLengthTest(string both){
        using std::chrono::high_resolution_clock;
        using std::chrono::duration_cast;
        using std::chrono::duration;
        using std::chrono::milliseconds;
        auto k= 128;
        vector<double> baseline(8);
        auto internal = 10000;
        auto external = 100;
        vint key = util::genBitsNonCrypto(256);
        vint iv = util::genBitsNonCrypto(256);

        cout<<internal*external <<" hashings tests"<<endl;
        for (int i = 0; i < external; ++i) {
            vector<vint> data(internal);
            vector<string> dataAsString(internal);

            for (int j = 0; j < internal; ++j) {
                data[j]= util::genBitsNonCrypto(128);
                dataAsString[j] = util::uintVec2Str(data[j]);
            }
            //call aes 128->256

            if (both=="both"){
                for (int j = 0; j < 6; ++j) {
                    auto outlen = k*pow(2,j);
                    auto t1 = high_resolution_clock::now();
                    for (int l = 0; l < internal; ++l) {
                        auto Xk = util::hash_variable(dataAsString[l], outlen);
                    }
                    auto t2 = high_resolution_clock::now();
                    duration<double, std::milli> ms_double = t2 - t1;
                    baseline[j] += ms_double.count();
                }
            }

            for (int j = 0; j < internal; ++j) {
                //use index 6 now
                auto t1 = high_resolution_clock::now();
                auto e = hashRTCCR::AES_vint_init(key, iv);
                auto res = hashRTCCR::AES_vint_encrypt(data[j],key,iv,e);
                auto t2 = high_resolution_clock::now();
                duration<double, std::milli> ms_double = t2 - t1;
                baseline[6] += ms_double.count();
            }


        }
        for (int i = 0; i < 6; ++i) {
            auto outlen = k*pow(2,i);
            cout<<"shake256 "<< outlen<< "-bit ("<<pow(2,i) << "x) output in "<< baseline[i]/1000<< " s" <<endl;
        }
        cout<<"aes256 "<< "256-bit output in "<< baseline[6]/1000<< " s" <<endl;

    }


    static void repetitionTest(const vector<string>&f, int k,util::hashtype hashfunc, int repetitions){
        vector<double> baseline(2);
        vector<double> three(2);
        vector<double> ate(2);
        vector<double> ate_f(2);
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
            runGarble(f, util::baseline, k, hashfunc, baseline, x);
            //runGarble(f, util::threehalves, k, hashfunc, three, x);
            //runGarble(f, util::ateca, k, hashfunc, ate, x);
            //runGarble(f, util::atecaFXOR, k, hashfunc, ate_f, x);
        }

        printResult(util::baseline, baseline, hashfunc);
        printResult(util::threehalves, three, hashfunc);
        printResult(util::ateca, ate, hashfunc);
        printResult(util::atecaFXOR, ate_f, hashfunc);

    }

    static void printResult(util::scheme scheme, const vector<double> &baseline, util::hashtype hashfunc) {
        string title;
        switch (scheme) {
            case util::baseline: {
                title = "###baseline###";
                break;
            }
            case util::threehalves: {
                title = "###threehalves###";
                break;
            }
            case util::ateca: {
                title = "###ateca###";
                break;
            }
            case util::atecaFXOR: {
                title = "###atecaFXOR###";
                break;
            }
        }
        cout << title << endl;
        vector<string> categories = {"garble", "eval"};
        for (int i = 0; i < 2; ++i) {
            cout << categories[i] << " " << baseline[i] << " s" << endl;
        }
    }

    static void runGarble(const vector<string> &f, util::scheme type, int k, util::hashtype &hashfunc, vector<double> &timings, vector<int> &x) {
        switch (type) {
            case util::scheme::baseline:{
                boost::timer timer;
                auto [F,e,d] = baseGarble::garble(f, k, hashfunc);//needs hash type
                auto hash = get<2>(F);
                timings[0] += timer.elapsed();
                auto base_X = baseGarble::encode(e, x);
                timer.restart();
                auto base_Y = baseGarble::eval(F, base_X, f, k);
                timings[1] += timer.elapsed();
                auto base_y = baseGarble::decode(d, base_Y, k, hash);
                break;
            }
            case util::threehalves:{
                boost::timer timer;
                auto [three_F,three_e,three_d, three_ic, three_hash] = threeHalves::garble(f, k, hashfunc);
                timings[0] += timer.elapsed();
                auto three_X = threeHalves::encode(three_e, x);

                timer.restart();
                auto three_Y = threeHalves::eval(three_F, three_X, f, k, three_ic, three_hash, hashfunc);
                timings[1] += timer.elapsed();
                auto three_y = threeHalves::decode(three_d, three_Y, f, k);

                break;
            }
            case util::ateca:{
                boost::timer timer;
                auto [ate_F, ate_e, ate_d, ate_k, ate_ic, ate_hash] = atecaGarble::garble(f, k, hashfunc);
                timings[0] += timer.elapsed();

                auto ate_X = atecaGarble::encode(ate_e, x);
                timer.restart();
                auto ate_Y = atecaGarble::eval(ate_F, ate_X, f, ate_k, ate_ic, ate_hash);
                timings[1] += timer.elapsed();
                auto ate_y = atecaGarble::decode(ate_Y, ate_d, ate_hash);
                break;
            }
            case util::atecaFXOR:{
                boost::timer timer;
                auto [atef_F, atef_e, atef_d, atef_k, atef_ic, atef_hash] = atecaFreeXOR::garble(f, k, hashfunc);
                timings[0] += timer.elapsed();

                auto atef_X = atecaFreeXOR::encode(atef_e, x);
                timer.restart();
                auto atef_Y = atecaFreeXOR::eval(atef_F, atef_X, f, atef_k, atef_ic, atef_hash);
                timings[1] += timer.elapsed();
                auto atef_y = atecaFreeXOR::decode(atef_Y, atef_d, atef_hash);
                break;
            }
        }
    }

    static void time_circuit_all(const vector<string>&f, const vector<int>& x, int k, util::hashtype hashfunc){
        if (hashfunc){
            cout<<"windows fast"<<endl;
        }else{
            cout<<"windows slow"<<endl;
        }
        //cout<< "Keccak_f test"<<endl;
        timing::time_circuit(f,x,k,util::baseline, hashfunc);
        timing::time_circuit(f,x,k,util::threehalves, hashfunc);
        timing::time_circuit(f,x,k,util::ateca, hashfunc);
        timing::time_circuit(f,x,k,util::atecaFXOR, hashfunc);

    }

    static void time_circuit(const vector<string>&f, const vector<int>& x, int k, util::scheme type, util::hashtype hashfunc){
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
                auto [F,e,d] = baseGarble::garble(f, k, hashfunc);//needs hash type

                auto t2 = high_resolution_clock::now();
                duration<double, std::milli> ms_double = t2 - t1;
                cout<< "garbling: " <<ms_double.count()<< "ms"<<endl;
                auto hash = get<2>(F);
                auto base_X = baseGarble::encode(e, x);
                t1 = high_resolution_clock::now();
                auto base_Y = baseGarble::eval(F, base_X, f, k);
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
                auto [three_F,three_e,three_d, three_ic, three_hash] = threeHalves::garble(f, k, hashfunc);
                auto t2 = high_resolution_clock::now();
                duration<double, std::milli> ms_double = t2 - t1;
                cout<< "garbling: " <<ms_double.count()<< "ms"<<endl;

                auto three_X = threeHalves::encode(three_e, x);

                t1 = high_resolution_clock::now();
                auto three_Y = threeHalves::eval(three_F, three_X, f, k, three_ic, three_hash, hashfunc);
                t2 = high_resolution_clock::now();
                ms_double = t2 - t1;
                cout<< "evaluation: " <<ms_double.count()<< "ms"<<endl;

                auto three_y = threeHalves::decode(three_d, three_Y, f, k);
                cout<< "three: " << three_y[0] <<endl;
                break;
            }
            case util::scheme::ateca:{
                title = "ateca w. ";
                cout<<title << hashtype<<endl;
                auto t1 = high_resolution_clock::now();
                auto [ate_F, ate_e, ate_d, ate_k, ate_ic, ate_hash] = atecaGarble::garble(f, k, hashfunc);
                auto t2 = high_resolution_clock::now();
                duration<double, std::milli> ms_double = t2 - t1;
                cout<< "garbling: " <<ms_double.count()<< "ms"<<endl;

                cout<<"bits in f: "<< util::averageFweight(ate_F)<<endl;

                auto ate_X = atecaGarble::encode(ate_e, x);

                t1 = high_resolution_clock::now();
                auto ate_Y = atecaGarble::eval(ate_F, ate_X, f, ate_k, ate_ic, ate_hash);
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
                auto [atef_F, atef_e, atef_d, atef_k, atef_ic, atef_hash] = atecaFreeXOR::garble(f, k, hashfunc);
                auto t2 = high_resolution_clock::now();
                duration<double, std::milli> ms_double = t2 - t1;
                cout<< "garbling: " <<ms_double.count()<< "ms"<<endl;
                cout<<"bits in f: "<< util::averageFweight(atef_F)<<endl;

                auto atef_X = atecaFreeXOR::encode(atef_e, x);

                t1 = high_resolution_clock::now();
                auto atef_Y = atecaFreeXOR::eval(atef_F, atef_X, f, atef_k, atef_ic, atef_hash);
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