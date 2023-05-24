//
// Created by svend on 024, 24-03-2023.
//

#ifndef GARBLINGGATES_HASHRTCCR_H
#define GARBLINGGATES_HASHRTCCR_H

#include "util.h"
#include <emmintrin.h>
#include <immintrin.h>
#include <string.h>

#include <utility>
#include <chrono>


using namespace std;

using std::chrono::high_resolution_clock;
using std::chrono::duration_cast;
using std::chrono::duration;
using std::chrono::milliseconds;

class hashRTCCR {
public:
    vint key;
    vint iv;
    vint alpha;
    vint u1;
    vint u2;
    EVP_CIPHER_CTX *e;
    util::hashtype hashtype = util::RO;

    const vint &getKey() const {
        return key;
    }

    const vint &getIv() const {
        return iv;
    }

    const vint &getAlpha() const {
        return alpha;
    }

    const vint &getU1() const {
        return u1;
    }

    const vint &getU2() const {
        return u2;
    }

    EVP_CIPHER_CTX *getE() const {
        return e;
    }
    util::hashtype getHash() const {
        return hashtype;
    }

    hashRTCCR(const vint& key, const vint& iv, int k){
        if(k < 256) k=256;
        this->key = key;
        this->iv = iv;
        this->alpha = util::genBitsNonCrypto((k/2)+1);
        this->u1 =    util::genBitsNonCrypto((k/2)+1);
        this->u2 =    util::genBitsNonCrypto((k/2)+1);
        this-> e = AES_vint_init(key, iv);
        this->hashtype = util::fast;
    }
    hashRTCCR(const vint& key, const vint& iv, int k, int base){
        if(k < 256) k=256;
        this->key = key;
        this->iv = iv;
        this->alpha = util::genBitsNonCrypto(   (k/2));
        this->u1 = util::genBitsNonCrypto(      (k/2));
        this->u2 = util::genBitsNonCrypto(      (k/2));
        this-> e = AES_vint_init(key, iv);
        this->hashtype = util::fast;
    }
    hashRTCCR(){
    }

    static uint64_t gfmulPCF(uint64_t a, uint64_t b){
        auto aepi64 = _mm_set_epi64x(0,a);
        auto bepi64 = _mm_set_epi64x(0,b);
        auto res = _mm_clmulepi64_si128(aepi64, bepi64, 0);
        return _mm_extract_epi64(res, 0);
    }

//may not be correct
    static vint gfmulPCF(vint& a, vint& b){
        vint res;
        int sizeDiff = a.size() - b.size();
        if(sizeDiff > 0){
            for (int i = 0; i < sizeDiff; ++i) {
                b.emplace_back(0);
            }
        } else if(sizeDiff < 0){
            for (int i = 0; i < -sizeDiff; ++i) {
                a.emplace_back(0);
            }
        }
        for (int i = 0; i < a.size(); ++i) {
            res.push_back(gfmulPCF(a[i], b[i]));
        }
        return res;
    }


    static vint gfmul(uint64_t a, uint64_t b){
        vint res = {0};
        uint64_t mask = 1;
        for (int i = 0; i < 64; ++i) {
            if (b & mask){
                res[0] ^= a;
            }
            if (a & 0x8000000000000000){
                a = (a << 1) ^ 0x87;
            } else{
                a = a << 1;
            }
            b = b >> 1;
        }
        return res;
    }

    static vint gfmul(vint a, vint b){
        vint res = {0,0};
        for (int i = 0; i < a.size(); ++i) {
            vint temp = gfmul(a[i], b[i]);
            res[0] ^= temp[0];
            res[1] ^= temp[1];
        }
        return res;
    }

    static vint randomGF2(int blocks){
        int len = blocks * 64;
        //generate random GF(2^len) element
        return util::genBitsNonCrypto(len);
    }

//change to halflabels
    static vint sigmaFunc(vint& labelLeft, vint & labelRight, vint& alpha){
        //gfmul half of input with alpha
        //vint halfInput1;
        //vint halfInput2;
        //for (int i = 0; i < input.size()/2; ++i) {
        //    halfInput1.push_back(input[i]);
        //    halfInput2.push_back(input[i + input.size()/2]);
        //}
        vint res1 = gfmulPCF(labelLeft, alpha);
        vint res2 = gfmulPCF(labelRight, alpha);
        res1.reserve(2*res1.size());
        res1.insert(res1.end(), res2.begin(), res2.end());
        return res1;
    }

    static EVP_CIPHER_CTX * AES_vint_init(vint key, vint iv){
        if(key.size() != 4){ //4 is hardcoded for 256 bit input
            int size = 4-key.size();
            for (int i = 0; i < size; ++i) {
                key.emplace_back(0);
            }
        }
        int len = key.size() * sizeof(uint64_t);
        //key from key
        auto *aes_key = static_cast<unsigned char *>(malloc(len));
        memcpy(aes_key, key.data(), len);
        //iv
        auto *aes_iv = static_cast<unsigned char *>(malloc(AES_BLOCK_SIZE));
        memcpy(aes_iv, iv.data(), AES_BLOCK_SIZE);
        EVP_CIPHER_CTX *e;
        e = EVP_CIPHER_CTX_new();
        EVP_EncryptInit_ex(e, EVP_aes_128_cbc(), NULL, aes_key, aes_iv);

        //EVP_DecryptInit_ex(e, EVP_aes_256_cbc(), NULL, aes_key, iv);
        EVP_CIPHER_CTX_set_padding(e, 0);
        free(aes_key);
        free(aes_iv);
        return e;
    }



    static vint AES_vint_encrypt(vint input, vint key, vint iv, EVP_CIPHER_CTX *e){
        if(input.size() != 4){ //4 is hardcoded for 256 bit input
            int size = 4-input.size();
            for (int i = 0; i < size; ++i) {
                input.emplace_back(0);
            }
        }
        int len = input.size() * sizeof(uint64_t);
        int c_len = len + AES_BLOCK_SIZE;
        int f_len = 0;
        auto *ciphertext = static_cast<unsigned char *>(malloc(c_len));
        //plaintext from input
        auto *plaintext = static_cast<unsigned char *>(malloc(len));
        memcpy(plaintext, input.data(), len);
        //iv
        auto *aes_iv = static_cast<unsigned char *>(malloc(AES_BLOCK_SIZE));
        memcpy(aes_iv, iv.data(), AES_BLOCK_SIZE);

        //set iv without init
        //EVP_CIPHER_CTX_ctrl(e, EVP_CTRL_GCM_SET_IVLEN, AES_BLOCK_SIZE, aes_iv);
        //EVP_CIPHER_CTX_ctrl(e, EVP_CTRL_AEAD_SET_IV_FIXED, AES_BLOCK_SIZE, aes_iv);

        memcpy((void *) EVP_CIPHER_CTX_iv(e), aes_iv, AES_BLOCK_SIZE); // set new IV value
        //EVP_EncryptInit_ex(e, EVP_aes_256_cbc(), nullptr, aes_key, aes_iv);
        EVP_EncryptUpdate(e, ciphertext, &c_len, plaintext, len);
        EVP_EncryptFinal_ex(e, ciphertext + c_len, &f_len);
        //convert ciphertext to vint
        vint res(len/sizeof(uint64_t), 0);
        memcpy(res.data(), ciphertext, len);
        free(ciphertext);
        free(plaintext);
        free(aes_iv);
        //EVP_CIPHER_CTX_cleanup(e);
        return res;
    }

    static vint AES_vint_decrypt(vint input, vint key, vint iv, EVP_CIPHER_CTX *e){
        if(input.size() != 4){ //4 is hardcoded for 256 bit input
            int size = 4-input.size();
            for (int i = 0; i < size; ++i) {
                input.emplace_back(0);
            }
        }
        int len = input.size() * sizeof(uint64_t);
        int p_len = len;
        int f_len = 0;
        auto *plaintext = static_cast<unsigned char *>(malloc(len));
        //ciphertext from input
        auto *ciphertext = static_cast<unsigned char *>(malloc(len));
        memcpy(ciphertext, input.data(), len);

        //key from key
        auto *aes_key = static_cast<unsigned char *>(malloc(len));
        memcpy(aes_key, key.data(), len);

        //iv from iv        16 block times 8 bits = 128 bits
        auto *aes_iv = static_cast<unsigned char *>(malloc(AES_BLOCK_SIZE));
        memcpy(aes_iv, iv.data(), AES_BLOCK_SIZE);

        //EVP_DecryptInit_ex(e, EVP_aes_256_cbc(), NULL, aes_key, aes_iv);
        EVP_DecryptUpdate(e, plaintext, &p_len, ciphertext, len);
        EVP_DecryptFinal_ex(e, plaintext + p_len, &f_len);
        //convert plaintext to vint
        vint res(len/sizeof(uint64_t), 0);
        memcpy(res.data(), plaintext, len);
        free(ciphertext);
        free(plaintext);
        free(aes_key);
        free(aes_iv);
        return res;
    }



//randomized tweakable circular correlation robust hash function
    vint hash(halfLabels &input, vint& tweak){
        //generate two random GF(2^len/2) elements
        //vint u1 = randomGF2(len/2);
        //vint u2 = randomGF2(len/2);
        //instantiate sigma as a random GF(2^len/2) element
        auto u1tweak = gfmulPCF((u1), tweak);
        auto u2tweak = gfmulPCF((u2), tweak);
        //u1tweak.insert(u1tweak.end(), u2tweak.begin(), u2tweak.end());
        //vint Utweak = u1tweak;
        //halfLabels Utweak = {u1tweak, u2tweak};
        //auto XxorUtweak = util::halfLabelXOR(input, Utweak);

        auto XxorUtweakL = util::vecXOR(get<0>(input), u1tweak);
        auto XxorUtweakR = util::vecXOR(get<1>(input), u2tweak);

        vint XxorUtweakVint; XxorUtweakVint.reserve(XxorUtweakL.size()*2);
        XxorUtweakVint.insert(XxorUtweakVint.end(), XxorUtweakL.begin(), XxorUtweakL.end());
        XxorUtweakVint.insert(XxorUtweakVint.end(), XxorUtweakR.begin(), XxorUtweakR.end());
        auto AESXxorUtweak = AES_vint_encrypt(XxorUtweakVint, key, iv, e);
        return util::vecXOR(sigmaFunc(XxorUtweakL,XxorUtweakR, alpha), AESXxorUtweak);
    }

    vint hashVint(vint input, vint tweak){
        auto middle = input.begin()+input.size()/2;
        vector<uint64_t> firstHalf(input.begin(), middle);
        vector<uint64_t> secondHalf(middle, input.end());
        //vector<uint64_t> firstHalf = {input[0]};
        //vector<uint64_t> secondHalf = {input[1]};
        //make sure all input is at least 256 bits
        //if(secondHalf.size() < 2){
        //    firstHalf.emplace_back(0);
        //    secondHalf.emplace_back(0);
        //}
        //if(tweak.size() < 2){
        //    tweak.emplace_back(0);
        //}
        halfLabels in = {firstHalf,secondHalf};
        auto res =  hash(in, tweak);
        vector<uint64_t> resfirstHalf(input.begin(), middle);
        vector<uint64_t> ressecondHalf(middle, input.end());
        //pop back if resfirstHalf is larger than hafl of input
        if(resfirstHalf.size() > input.size()/2){
            resfirstHalf.pop_back();
            ressecondHalf.pop_back();
        }
        resfirstHalf.reserve(resfirstHalf.size() + ressecondHalf.size());
        resfirstHalf.insert(resfirstHalf.end(), ressecondHalf.begin(), ressecondHalf.end());

        //int size = res.size() - input.size();
        //if(size > 0){
        //    for (int i = 0; i < size; ++i) {
        //        res.pop_back();
        //    }
        //}
        return resfirstHalf;
    }


    static void testDecrypt() {
        int k = 256;
        auto key = util::genBitsNonCrypto(k);
        vint iv = util::genBitsNonCrypto(128);
        auto e = AES_vint_init(key, iv);
        auto input = util::genBitsNonCrypto(k);
        cout << "input:             " << util::uintVec2Str(input) << endl;
        auto res = AES_vint_encrypt(input, key, iv, e);
        cout << "encrypted input:   " << util::uintVec2Str(res) << endl;
        //res = AES_vint_decrypt(res, key, iv, d);
        //cout << "decrypted input:   " << util::uintVec2Str(res) << endl;
    }

    //static void testHashRTCCR() {
    //    int k = 256;
    //    auto key = util::genBitsNonCrypto(k);
    //    vint iv = util::genBitsNonCrypto(128);
    //    auto e = AES_vint_init(key, iv);
    //    auto inputL = util::genBitsNonCrypto(k/2);
    //    auto inputR = util::genBitsNonCrypto(k/2);
    //    halfLabels input = make_tuple(inputL, inputR);
    //    auto tweak = util::genBitsNonCrypto(k/2);
    //    vint sigmaValue = util::genBitsNonCrypto(k/2);
    //    vint u1 = util::genBitsNonCrypto(k/2);
    //    vint u2 = util::genBitsNonCrypto(k/2);
    //    auto ctx1.hashRTCCR(key,iv,k);
    //    auto res = hash(input, tweak, key, iv, e, sigmaValue, u1, u2);
    //    cout << "res: " << util::uintVec2Str(res) << endl;
    //    util::printUintVec(res);
    //    res = hash(input, tweak, key, iv, e, sigmaValue, u1, u2);
    //    cout << "res: " << util::uintVec2Str(res) << endl;
    //    util::printUintVec(res);
    //    auto inputL2 = util::genBitsNonCrypto(k/2);
    //    auto inputR2 = util::genBitsNonCrypto(k/2);
    //    halfLabels input2 = make_tuple(inputL2, inputR2);
    //    auto tweak2 = util::genBitsNonCrypto(k/2);
    //    auto key2 = util::genBitsNonCrypto(k);
    //    auto res2 = hash(input2, tweak2, key, iv, e, sigmaValue, u1, u2);
    //    cout << "res2: " << util::uintVec2Str(res2) << endl;
    //    util::printUintVec(res2);
    //    auto res1 = hash(input, tweak, key, iv, e, sigmaValue, u1, u2);
    //    cout << "res: " << util::uintVec2Str(res1) << endl;
    //    util::printUintVec(res1);
    //    auto res3 = hash(input2, tweak2, key, iv, e, sigmaValue, u1, u2);
    //    cout << "res2: " << util::uintVec2Str(res3) << endl;
    //    util::printUintVec(res3);
    //}
};

#endif //GARBLINGGATES_HASHRTCCR_H
