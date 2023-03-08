//
// Created by a on 17/02/2023.
//
#ifndef GARBLINGGATES_UTIL_H
#define GARBLINGGATES_UTIL_H
#include <string>
#include <sstream>
#include <vector>
#include <iterator>
#include <iostream>
#include "circuitParser.h"
#include <bitset>
#include <random>
#include "otUtil/otUtil.h"
#include <openssl/sha.h>
#include <openssl/aes.h>
#include <openssl/evp.h>


//Namespace for custom types
namespace customTypeSpace {
    typedef vector<uint64_t> vint;
    typedef tuple<vint, vint> labelPair;
    typedef tuple<vint, vint> halfDelta;
    typedef tuple<vint, vint> halfLabels;
}
using namespace customTypeSpace;

class util {

public:

    //template for splitting strings taken from:
    // https://stackoverflow.com/questions/236129/how-do-i-iterate-over-the-words-of-a-string
    template <typename Out>
    static void split(const std::string &s, char delim, Out result) {
        std::istringstream iss(s);
        std::string item;
        while (std::getline(iss, item, delim)) {
            *result++ = item;
        }
    }
    static std::vector<std::string> split(const std::string &s, char delim) {
        std::vector<std::string> elems;
        split(s, delim, std::back_inserter(elems));
        return elems;
    }

    static void printUintVec(const std::vector<uint64_t>& vec){
        for (auto const& i: vec) {
        std::cout<< i << ' ';
        }
        std::cout<< std::endl;
    }
    static void printStrVec(const std::vector<std::string>& vec){
        for (auto const& i: vec) {
            std::cout<< i << std::endl;
        }
    }
    static string uintVec2Str(vector<::uint64_t>vec){
        stringstream res;
        copy(vec.begin(),vec.end(),ostream_iterator<::uint64_t>(res));
        return res.str();
    }

    static void printCircuit(const std::string& path){
        auto res = circuitParser::parseCircuit(path);
        util::printStrVec(res);
    }

    static tuple<vector<uint64_t>,
            vector<tuple<vector<uint64_t>, vector<uint64_t>>>>
            generateRandomLabels(int k, vector<uint64_t>& globalDelta,
                                     vector<tuple<vector<uint64_t>, vector<uint64_t>>>& wiresLabels) {
                //generate new global delta if non is given
                if(globalDelta[0]==0){
                    globalDelta = genDelta(k);
                }
                //generate input labels
                for(auto & wiresLabel : wiresLabels){
                    labelPointAndPermute(k, wiresLabel, globalDelta);
                }
                return make_tuple(globalDelta, wiresLabels);
    }

    static vector<uint64_t> genDelta(int k) {
        vector<uint64_t> globalDelta = otUtil::genBitsNonCrypto(k);
        globalDelta[0] = globalDelta[0] | 1;
        return globalDelta;
    }

    static halfDelta genDeltaHalves(int k) {
        vector<uint64_t> leftDeltaHalf = otUtil::genBitsNonCrypto(k/2);
        leftDeltaHalf[0] = leftDeltaHalf[0] | 1;
        vector<uint64_t> rightDeltaHalf = otUtil::genBitsNonCrypto(k/2);

        return {leftDeltaHalf, rightDeltaHalf};
    }

    static halfDelta genLabelHalves(int k) {
        vector<uint64_t> leftLabelHalf = otUtil::genBitsNonCrypto(k/2);
        leftLabelHalf[0] = leftLabelHalf[0] & (UINT64_MAX << 1);
        vector<uint64_t> rightLabelHalf = otUtil::genBitsNonCrypto(k/2);

        return {leftLabelHalf, rightLabelHalf};
    }

    static void labelPointAndPermute(int k,
                                     tuple<vector<uint64_t>, vector<uint64_t>> &wiresLabel,
                                        const vector<uint64_t>& globalDelta,
                                     int permuteBit = 2) {//generate random permute bit
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, 1);
        if (permuteBit >= 2) permuteBit = dis(gen);
        vector<uint64_t> a0 = otUtil::genBitsNonCrypto(k);
        //add permute bit
        a0[0] = a0[0] & -2; //1111 ... 1111 1110 THE COLOR BIT IS ON THE LEAST SIGNIFICANT BIT ON BLOCK ZERO
        a0[0] = a0[0] | permuteBit; //can be hardcoded as it is one block
        auto a1 = vector<uint64_t>(a0.size());
        for (int i = 0; i < a0.size(); ++i) {
            a1[i] = a0[i] ^ globalDelta[i];
        }
        wiresLabel = make_pair(a0, a1);
    }

    static vector <::uint64_t> genBitsNonCrypto(int bits) {
        auto res = vector<::uint64_t>((bits+64-1)/64);
        for (int blockNum = 0; blockNum <(bits+64-1)/64; ++blockNum) {
            res[blockNum] = random_bitset<64>().to_ullong();
            //for (int i = 0; i < 64; ++i) {bitset[i]=(prng.GenerateBit());}
            //res[blockNum]=bitset.to_ullong();
        }
        return res;
    }
    //ith bit of vector might have fucked up indexing, nice (:
    //checks right to left
    static int checkIthBit(vector<uint64_t> ui, int i) {
        //ith bit
        int bit = i%64;
        //find block
        int block = i / 64;
        return checkBit(ui[block],bit);
    }
    //find ith bit the slow way
    //static int findIthBit(vector<::uint64_t> ui, int i){
    //    //ith bit
    //    int bit = i%64;
    //    //find block
    //    int block = i / 64;
    //    string blockStr = bitset<64>(ui[block]).to_string();
    //    return blockStr[bit] - '0';
    //}

    static int checkBit(::uint64_t num, int i){  //# From https://stackoverflow.com/questions/18111488/convert-integer-to-binary-in-python-and-compare-the-bits
        return (num >> i) & 1;
    }

    static int ithBitL2R(vector<uint64_t> v, int i){
        int block = i / 64;
        return  checkBitL2R(v[block],i);
    }
    //this has reverse index, will check from the left most bit
    //checks left to right
    static int checkBitL2R(::uint64_t num, int i){  //# From https://stackoverflow.com/questions/18111488/convert-integer-to-binary-in-python-and-compare-the-bits
        return (num >> (63-(i%64))) & 1;
    }


    static vector<::uint64_t> setIthBitTo1L2R(vector<::uint64_t> vec, int pos){
        int block = pos / 64;
        auto oneshifted = ((uint64_t)1) << (63-(pos%64));
        vec[block] |= oneshifted;
        return vec;
    }

    static vector<::uint64_t> vecXOR(vector<::uint64_t>left, const vector<::uint64_t>& right){
        for (int i = 0; i < left.size(); ++i) {
            left[i] = left[i] ^ right[i];
        }
        return left;
    }
    static vector<::uint64_t> vecAND(vector<::uint64_t>left, const vector<::uint64_t>& right){
        for (int i = 0; i < left.size(); ++i) {
            left[i] = left[i] & right[i];
        }
        return left;
    }
    static vector<::uint64_t> vecOR(vector<::uint64_t>left, const vector<::uint64_t>& right){
        for (int i = 0; i < left.size(); ++i) {
            left[i] = left[i] | right[i];
        }
        return left;
    }

    static inline vector<::uint64_t> vecAndStatic(vector<::uint64_t> v, ::uint64_t s){
        for (int i = 0; i < v.size(); ++i) {
            v[i] = v[i] & s;
        }
        return v;
    }
    //computes the nor operation
    static uint64_t norOP(uint64_t a, uint64_t b){
        return ~(~(~(a) & ~(b)));
    }

    static inline vector<::uint64_t> vecNorStatic(vector<::uint64_t> v, ::uint64_t s){
        for (int i = 0; i <v.size(); ++i) {
            v[i] = norOP(v[i],s);
        }
        return v;
    }

    static inline vector<::uint64_t> vecInvert(vector<::uint64_t> v){
        for (int i = 0; i <v.size(); ++i) {
            v[i] = ~v[i];
        }
        return v;
    }


    //taken from https://www.appsloveworld.com/cplus/100/112/c-efficient-way-to-generate-random-bitset-with-configurable-mean-1s-to-0s-r
    template< size_t size>
    static typename std::bitset<size> random_bitset( double p = 0.5) {

        typename std::bitset<size> bits;
        std::random_device rd;
        std::mt19937 gen( rd());
        std::bernoulli_distribution d( p);

        for( int n = 0; n < size; ++n) {
            bits[n] = d(gen);
        }

        return bits;
    }

    //taken from https://helloacm.com/c-coding-exercise-number-of-1-bits-revisited/
    static inline int hammingWeight(uint64_t x) {
        x -= (x >> 1) & 0x5555555555555555;             //put count of each 2 bits into those 2 bits
        x = (x & 0x3333333333333333) + ((x >> 2) & 0x3333333333333333); //put count of each 4 bits into those 4 bits
        x = (x + (x >> 4)) & 0x0f0f0f0f0f0f0f0f;        //put count of each 8 bits into those 8 bits
        return (x * 0x0101010101010101) >> 56;  //returns left 8 bits of x + (x<<8) + (x<<16) + (x<<24) + ...
    }

    static inline int vecHW(vector<uint64_t> x) {
        int hw = 0;
        for (int i = 0; i < x.size(); ++i) {
            hw += hammingWeight(x[i]);
        }
        return hw;
    }

    //perform variable output length hash
    static vector<uint64_t> hash_variable(const std::string& input, int output_length_bits = 128)
    {
        size_t output_length_bytes = output_length_bits / 8;
        EVP_MD_CTX* ctx = EVP_MD_CTX_create();
        EVP_DigestInit_ex(ctx, EVP_shake256(), NULL);
        EVP_DigestUpdate(ctx, input.c_str(), input.size());
        std::string output(output_length_bytes, '\0');
        EVP_DigestFinalXOF(ctx, reinterpret_cast<unsigned char *>(&output[0]), output_length_bytes);
        EVP_MD_CTX_destroy(ctx);
        //convert output to vector<uint64_t>
        vector<uint64_t> output_vector;
        for (int i = 0; i < output_length_bytes; i+=8) {
            uint64_t temp = 0;
            for (int j = 0; j < 8; ++j) {
                temp += (uint64_t)output[i+j] << (j*8);
            }
            output_vector.push_back(temp);
        }
        return output_vector;
    }

    static ::uint64_t leftShiftFill1(::uint64_t x){
        return ~((~x)<<1);
    }
    static vector<bitset<64>> insertBitVecBitset(vector<bitset<64>> vec, int bit, int i){

        int block = i/64;
        //for left to right insertion
        //int pos = 63-(i%64);
        //vec[block][pos] = bit;
        //for left to right uint but right to left index
        vec[block][i%64] = bit;
        return vec;
    }

    static string
    sliceVecL2R(vector<uint64_t> X_00, vector<uint64_t> X_01, vector<uint64_t> X_10, vector<uint64_t> X_11,
                int j) {
        int x00j = ithBitL2R(X_00,j);
        int x01j = ithBitL2R(X_01,j);
        int x10j = ithBitL2R(X_10,j);
        int x11j = ithBitL2R(X_11,j);
        return to_string(x00j).append(to_string(x01j)).append(to_string(x10j)).append(to_string(x11j));
    }
    static void getBits(string &f, int &numberOfInputBits) {
        numberOfInputBits= 0;
        auto split = util::split(f, ' ');
        int numberOfInputWires = stoi(split[0]);
        for (int i = 0; i < numberOfInputWires; ++i) {
            int numberOfBits = stoi(split[i + 1]);
            numberOfInputBits += numberOfBits;
        }
    }

    static tuple<vector<int>, vector<int>, string> extractGate(const string &line) {
        vector<int> inputWires;
        vector<int> outputWires;
        //split line into space separated values
        vector<string> lineSplit = util::split(line, ' ');
        int numInputWires = stoi(lineSplit[0]);
        int numOutputWires = stoi(lineSplit[1]);
        //handle input wires
        for (int j = 2; j < numInputWires + 2; ++j) { //index names start at 2
            //get next input wire label/index
            inputWires.push_back(stoi(lineSplit[j]));
        }
        //handle output wires
        for (int j = numInputWires + 2; j < numInputWires + numOutputWires + 2; ++j) {
            //get next output wire label/index
            outputWires.push_back(stoi(lineSplit[j]));
        }
        //handle gate type
        string gateType = lineSplit[numInputWires + numOutputWires + 2];
        return make_tuple(inputWires, outputWires, gateType);
    }
};


#endif //GARBLINGGATES_UTIL_H
