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
#include <bitset>
#include <random>
#include <openssl/sha.h>
#include <openssl/aes.h>
#include <openssl/evp.h>
#include <immintrin.h>
#include <tuple>
#include <algorithm>
#include <cstring>
#include <cstddef>
#include "string.h"

//Namespace for custom types
namespace customTypeSpace {
    typedef std::vector<uint64_t> vint;
    typedef std::tuple<vint, vint> labelPair;
    typedef std::tuple<vint, vint> halfDelta;
    typedef std::tuple<vint, vint> halfLabels;
}
using namespace customTypeSpace;
using namespace std;

class util {

public:
    enum scheme{
        baseline =0,
        threehalves =1,
        ateca =2,
        atecaFXOR=3
    };
    enum hashtype{
        RO =0,
        fast =1
    };
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
    static string printBitsetofVectorofUints(vector<uint64_t> uints){
        string res;
        for (int i = uints.size()-1; i >= 0; --i) {
            res += bitset<64>(uints[i]).to_string();
        }
        return res;
    }
    static std::string uintVec2Str(std::vector<::uint64_t>vec){
        std::stringstream res;
        copy(vec.begin(),vec.end(),std::ostream_iterator<::uint64_t>(res));
        return res.str();
    }

    static vector<tuple<vector<uint64_t>, vector<uint64_t>>>
            generateRandomLabels(int k, vector<uint64_t>& globalDelta, int size) {
                auto wiresLabels = vector<tuple<vector<uint64_t>, vector<uint64_t>>>(size);

                //generate new global delta if non is given
                //if(globalDelta[0]==0){
                //    globalDelta = genDelta(k);
                //}
                //generate input labels
                for(auto & wiresLabel : wiresLabels){
                    labelPointAndPermute(k, wiresLabel, globalDelta);
                }
                return wiresLabels;
    }

    static vector<uint64_t> genDelta(int k) {
        vector<uint64_t> globalDelta = genBitsNonCrypto(k);
        globalDelta[0] = globalDelta[0] | 1;
        return globalDelta;
    }

    static void labelPointAndPermute(int k,
                                     tuple<vector<uint64_t>, vector<uint64_t>> &wiresLabel,
                                        const vector<uint64_t>& globalDelta,
                                     int permuteBit = 2) {//generate random permute bit
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, 1);
        if (permuteBit >= 2) permuteBit = dis(gen);
        vector<uint64_t> a0 = genBitsNonCrypto(k);
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
    static vector <::uint64_t> gen64bitsNonCrypto(int bits) {
        auto res = vector<::uint64_t>((bits+64-1)/64);
        std::random_device rd;
        std::mt19937_64 gen( rd());
        for (int blockNum = 0; blockNum <(bits+64-1)/64; ++blockNum) {
            res[blockNum] = gen();
        }
        return res;
    }
    //ith bit of vector might have fucked up indexing, nice (:
    //checks right to left
    static int checkIthBit(vector<uint64_t> ui, int i) {
        //ith bit
        int bit = mod64(i);
        //find block
        int block = i / 64;
        return checkBit(ui[block],bit);
    }

    static int checkBit(::uint64_t num, int i){  //# From https://stackoverflow.com/questions/18111488/convert-integer-to-binary-in-python-and-compare-the-bits
        return (num >> i) & 1;
    }

    static int ithBitL2R(const vector<uint64_t>& v, int i){
        int block = i / 64;
        return checkBitL2R(v[block],(63-(mod64(i))));
    }

    //this has reverse index, will check from the left most bit
    //checks left to right
    static int checkBitL2R(::uint64_t num, int i){  //# From https://stackoverflow.com/questions/18111488/convert-integer-to-binary-in-python-and-compare-the-bits
        return (num >> i) & 1;
    }

    static void setIthBitTo1L2R(vector<uint64_t>* vec, int pos){
        int block = pos / 64;
        int index = (63-(mod64(pos)));
        //::int64_t val = (*vec)[block];
        auto oneshifted = ((uint64_t)1) << index;
        (*vec)[block] |= oneshifted;
    }
    static uint64_t setBit1L2R(uint64_t a, int pos){
        int index = (63-(pos%64));
        auto oneshifted = ((uint64_t)1) << (63-(mod64(pos)));
        //_bittestandset64(&sval, index);
        a |= oneshifted;
        return a;
    }

    static vector<::uint64_t> vecXOR(vector<::uint64_t>left, const vector<::uint64_t>& right){
        auto size = max(left.size(),right.size());
        left.reserve(size);
        for (int i = 0; i < size; ++i) {
            if(i<left.size() && i<right.size()){
                left[i] = left[i] ^ right[i];
            }
            else if(right.size()>left.size()){
                left.emplace_back(right[i]);
            }
        }
        return left;
    }

    static vint vecXOR(const vector<vint>& vints){
        vint res = vints[0];
        for (int i = 1; i < vints.size(); ++i) {
            res = vecXOR(res,vints[i]);
        }
        return res;
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

    static vector<::uint64_t> vecAndStatic(vector<::uint64_t> v, ::uint64_t s){
        for (int i = 0; i < v.size(); ++i) {
            v[i] = v[i] & s;
        }
        return v;
    }
    //computes the nor operation
    static uint64_t norOP(uint64_t a, uint64_t b){
        return ~(~(~(a) & ~(b)));
    }

    static vector<::uint64_t> vecNorStatic(vector<::uint64_t> v, ::uint64_t s){
        for (int i = 0; i <v.size(); ++i) {
            v[i] = norOP(v[i],s);
        }
        return v;
    }

    static vector<::uint64_t> vecInvert(vector<::uint64_t> v){
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
    static int hammingWeight(uint64_t x) {
        x -= (x >> 1) & 0x5555555555555555;             //put count of each 2 bits into those 2 bits
        x = (x & 0x3333333333333333) + ((x >> 2) & 0x3333333333333333); //put count of each 4 bits into those 4 bits
        x = (x + (x >> 4)) & 0x0f0f0f0f0f0f0f0f;        //put count of each 8 bits into those 8 bits
        return (x * 0x0101010101010101) >> 56;  //returns left 8 bits of x + (x<<8) + (x<<16) + (x<<24) + ...
    }
    //static int fastHW(::uint64_t x){
    //    return _mm_popcnt_u64(x);
    //}

    static int vecHW(const vector<uint64_t>& x) {
        int hw = 0;
        for (int i = 0; i < x.size(); ++i) {
            hw += hammingWeight(x[i]);
        }
        return hw;
    }

    //perform variable output length hash
    static vector<uint64_t> hash_variable(vint input, const vint& tweak , int output_length_bits = 128)
    {
        //append tweak
        //vector<::uint64_t> input = in;
        input = vecXOR(input,tweak);
        //input.insert(input.end(), tweak.begin(),tweak.end());
        //input padding
        if (input.size()<4){
            int diff = 4-input.size();
            for (int i = 0; i < diff; ++i) {
                input.emplace_back(0);
            }
        }

        //int byteInputSize = sizeof(::uint64_t)*input.size();
        size_t size_in_bytes = input.size() * sizeof(uint64_t);
        //auto *plaintext = static_cast<unsigned char *>(malloc(size_in_bytes));
        //memcpy(plaintext, input.data(), size_in_bytes);
        //vector<unsigned char> arr(size_in_bytes);
        //memcpy(arr.data(), reinterpret_cast<const unsigned char*>(input.data()), size_in_bytes);
        EVP_MD_CTX* ctx = EVP_MD_CTX_create();
        EVP_DigestInit_ex(ctx, EVP_shake256(), NULL);
        //EVP_DigestUpdate(ctx, reinterpret_cast<const unsigned char*>(input.data()), size_in_bytes);
        EVP_DigestUpdate(ctx, reinterpret_cast<const unsigned char*>(input.data()), size_in_bytes);
        size_t output_length_bytes = (output_length_bits+7) / sizeof(uint64_t);

        auto *ciphertext = static_cast<unsigned char *>(malloc(output_length_bytes));
        EVP_DigestFinalXOF(ctx, ciphertext, output_length_bytes);

        //convert the input back into uint64_t's
        vint res((output_length_bytes+7)/sizeof(::uint64_t));
        memcpy(res.data(), ciphertext, output_length_bytes);
        free(ciphertext);
        //free(plaintext);
        EVP_MD_CTX_destroy(ctx);
        return res;
    }

    //source: https://stackoverflow.com/questions/75652101/aes-with-openssl-in-c
    static vector<::uint64_t> anotherHash(const string& plaintext){

        const std::string key="Error generating random IV";
        unsigned char iv[EVP_MAX_IV_LENGTH]; // zero initialization
        static unsigned char ivStatic[EVP_MAX_IV_LENGTH];
        const EVP_CIPHER* cipher;
        cipher = EVP_get_cipherbyname("aes-128-cbc");

        EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
        EVP_CIPHER_CTX_init(ctx);
        EVP_EncryptInit_ex(ctx, cipher, nullptr, reinterpret_cast<const unsigned char*>(key.c_str()), iv);

        int len = 0, ciphertext_len = 0;
        int plaintext_len = plaintext.length();

        std::string ciphertext(plaintext_len + EVP_CIPHER_block_size(cipher), '\0');
        unsigned char* ciphertext_ptr = reinterpret_cast<unsigned char*>(&ciphertext[0]);

        EVP_EncryptUpdate(ctx, ciphertext_ptr, &len, reinterpret_cast<const unsigned char*>(plaintext.c_str()), plaintext_len);
        ciphertext_len += len;

        EVP_EncryptFinal_ex(ctx, ciphertext_ptr + len, &len);
        ciphertext_len += len;

        EVP_CIPHER_CTX_free(ctx);

        string outString = ciphertext.substr(0, ciphertext_len);
        auto outputInBytes = outString.size();
        cout<<outputInBytes<<endl;
        vector<uint64_t> output_vector;
        for (int i = 0; i < outputInBytes; i+=8) {
            uint64_t temp = 0;
            auto rest = outputInBytes%8;
            if(rest != 0 && i >= outputInBytes - 8 ){
                for (int j = 0; j < rest; ++j) {
                    temp += (uint64_t)ciphertext[i+j] << (j*8);
                }
            } else {
                for (int j = 0; j < 8; ++j) {
                    temp += (uint64_t)ciphertext[i+j] << (j*8);
                }
            }
            output_vector.push_back(temp);
        }
        return output_vector;
        ///return ciphertext.substr(0, ciphertext_len);

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
        vec[block][mod64(i)] = bit;
        return vec;
    }

    static  string
    sliceVecL2R(const vint& X_00, const vint& X_01, const vint& X_10, const vint& X_11, int j) {
        int x00j = ithBitL2R(X_00,j);
        int x01j = ithBitL2R(X_01,j);
        int x10j = ithBitL2R(X_10,j);
        int x11j = ithBitL2R(X_11,j);
        return to_string(x00j).append(to_string(x01j)).append(to_string(x10j)).append(to_string(x11j));
    }

    static string
    sliceVecL2RAtecaFreeXorSpecial(const vint& globalDelta, const vint& X_00, const vint& X_01, const vint& X_10, const vint& X_11, int jprime, int j) {
        int gdj = ithBitL2R(globalDelta,jprime);
        int x00j = ithBitL2R(X_00,j);
        int x01j = ithBitL2R(X_01,j);
        int x10j = ithBitL2R(X_10,j);
        int x11j = ithBitL2R(X_11,j);
        return to_string(gdj).append(to_string(x00j)).append(to_string(x01j)).append(to_string(x10j)).append(to_string(x11j));
    }


    static int getBits(string &f) {
        int numberOfInputBits= 0;
        auto split = util::split(f, ' ');
        int numberOfInputWires = stoi(split[0]);
        for (int i = 0; i < numberOfInputWires; ++i) {
            int numberOfBits = stoi(split[i + 1]);
            numberOfInputBits += numberOfBits;
        }
        return numberOfInputBits;
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

    static halfLabels halfLabelXOR(halfLabels &input1, halfLabels &input2) {
        //auto [left1, right1] = input1;
        //auto [left2, right2] = input2;
        //auto newLeft = vecXOR(left1, left2);
        //auto newRight = vecXOR(right1, right2);
        auto newLeft = vecXOR(get<0>(input1), get<0>(input2));
        auto newRight = vecXOR(get<1>(input1), get<1>(input2));
        return {newLeft, newRight};
    }
    static vint prependBitToVint(vint &label, uint64_t bit) {
        label.push_back(bit);
        return label;
    }
    static string halfLabelsToFullLabelString(halfLabels &halfLabel){
        auto [left, right] = halfLabel;
        return uintVec2Str(left) + uintVec2Str(right);
    }

    static vector<int> genFunctionInput(int bits) {
        vector<int> x;
        x.reserve(bits);
        for (int i = 0; i < bits; ++i) {
            x.emplace_back(rand()%2);
        }
        return x;
    }

    static ::uint64_t averageFweight(const vector<vint>&F){
        uint64_t uintsWBits =0;
        for (const vint& delta : F) {
            for (::uint64_t j : delta) {
                if (j!=0){
                    uintsWBits +=1;
                }
            }
        }
        uint64_t xd = uintsWBits*64;
        return xd/F.size();
    }

    /* This projection is left to right meaning the projected bits are placed at the left most index of the bitset
     * Example:
     *     a 1010 1010 0000 ...
     *     b 0111 0110 0000 ...
     * a o b  010  01       ...
     * moved up
     * a o b 0100 1000 0000 ...
     */
    static vint projection(const vint& a,const vint& b){
        //projection A o B means take the bit A[i] if B[i]=1
        int k = util::vecHW(b);
        int uintsNeeded = k / 64 + ((k % 64 != 0) ? 1 : 0);
        auto projection = bitset<64>(0);
        auto res = vint(uintsNeeded);
        int bitsProjected =0; int j =0; int blockNum =0;
        do {
            if (util::ithBitL2R(b,j)==1){
                auto ithBitA = util::ithBitL2R(a,j);
                projection[(63-(bitsProjected % 64))]= ithBitA;
                bitsProjected++;
                if (bitsProjected == k||(bitsProjected%64==0 && bitsProjected !=0)){
                    uint64_t projUint = projection.to_ullong();
                    res[blockNum] = projUint;
                    blockNum++;
                }
            }
            j++;
        }while(bitsProjected != k);
        return res;
    }

    static vint fastproj(const vint& a, const vint& b, const int& k) {
        //projection A o B means take the bit A[i] if B[i]=1
        //int k = util::vecHW(b);
        int uintsNeeded = k / 64 + ((mod64(k) != 0) ? 1 : 0);
        uint64_t projection =0;
        auto res = vint(uintsNeeded);
        int bitsProjected =0; int j =0; int blockNum =0;
        do {
            auto blockIndex = j / 64;
            int intIndex = (63 - mod64(j));
            //if the ith bit of b ==1 find the ith bit in a and set the projection bit to that value
            if (util::checkBitL2R(b[blockIndex], intIndex) == 1){
                auto ithBitA = util::checkBitL2R(a[blockIndex],intIndex);
                if (ithBitA==1){
                    projection = util::setBit1L2R(projection,bitsProjected);
                }
                bitsProjected++;
                if (bitsProjected == k||(mod64(bitsProjected)==0 && bitsProjected !=0)){
                    res[blockNum] = projection;
                    projection=0;
                    blockNum++;
                }
            }
            j++;
        }while(bitsProjected != k);
        return res;
    }
    static ::uint64_t fast_modulo(const uint64_t input, const uint64_t ceil) {
        // apply the modulo operator only when needed
        // (i.e. when the input is greater than the ceiling)
        return input >= ceil ? input % ceil : input;
        // NB: the assumption here is that the numbers are positive
    }
    static vint vecConcat(vint a, const vint& b){
        a.insert(a.end(), b.begin(), b.end());
        return a;
    }

    static inline ::uint64_t mod64(const ::uint64_t in){
        return (in & (63));
    }
};



#endif //GARBLINGGATES_UTIL_H
