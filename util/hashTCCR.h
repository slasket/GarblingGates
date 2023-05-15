//
// Created by a on 14/04/2023.
//

#ifndef GARBLINGGATES_HASHTCCR_H
#define GARBLINGGATES_HASHTCCR_H
#include <utility>
#include <emmintrin.h>
#include <immintrin.h>
#include <algorithm>
#include "util.h"
#include "string.h"
#include <numeric>

using namespace std;
class hashTCCR{
public:
    //vint key;
    vint iv = {0};
    vint key;
    vint u1;
    vint u2;
    util::hashtype hashtype = util::RO;

    EVP_CIPHER_CTX *e;

    const vint &getIv() const {
        return iv;
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

    void setHash(util::hashtype hash) {
        hashTCCR::hashtype = hash;
    }

    hashTCCR(int k){//, vint key={0}){
        this->iv = util::genBitsNonCrypto(128);
        this->key= util::genBitsNonCrypto(128);
        this->u1 = util::genBitsNonCrypto((k/2));
        this->u2 = util::genBitsNonCrypto((k/2));
        this->e = AES_vint_init(key,iv);
        this->hashtype = util::fast;
    }
    hashTCCR(){
    }

    static  EVP_CIPHER_CTX * AES_vint_init(vint& key, vint& iv){
        EVP_CIPHER_CTX *e;
        e = EVP_CIPHER_CTX_new();

        int keylen = key.size() * sizeof(uint64_t);
        //key from key
        auto *aes_key = static_cast<unsigned char *>(malloc(keylen));
        memcpy(aes_key, key.data(), keylen);
        //iv from iv        16 block times 8 bits = 128 bits
        auto *aes_iv = static_cast<unsigned char *>(malloc(AES_BLOCK_SIZE));
        memcpy(aes_iv, iv.data(), AES_BLOCK_SIZE);
        EVP_EncryptInit_ex(e, EVP_aes_128_cbc(), NULL, aes_key, aes_iv);
        EVP_CIPHER_CTX_set_padding(e, 0);
        free(aes_key);
        free(aes_iv);
        return e;
    }


    static  vint AES_vint_encrypt(vint input,vint key, vint& iv, EVP_CIPHER_CTX *e){
        if(input.size() < 2){ //2 is hardcoded for 128 bit input
            int size = 2-input.size();
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
        //iv from iv        16 block times 8 bits = 128 bits
        auto *aes_iv = static_cast<unsigned char *>(malloc(AES_BLOCK_SIZE));
        memcpy(aes_iv, iv.data(), AES_BLOCK_SIZE);


        //int keylen = key.size() * sizeof(uint64_t);
        //auto *aes_key = static_cast<unsigned char *>(malloc(keylen));
        //memcpy(aes_key, key.data(), keylen);
        //EVP_EncryptInit_ex(e, EVP_aes_128_cbc(), NULL, aes_key, aes_iv);
        memcpy((void *) EVP_CIPHER_CTX_iv(e), aes_iv, AES_BLOCK_SIZE); // set new IV value
        EVP_EncryptUpdate(e, ciphertext, &c_len, plaintext, len);
        EVP_EncryptFinal_ex(e, ciphertext + c_len, &f_len);
        //convert ciphertext to vint
        vint res(len/sizeof(uint64_t), 0);
        memcpy(res.data(), ciphertext, len);
        free(ciphertext);
        free(plaintext);
        free(aes_iv);
        return res;
    }

    static  vint AES_vint_decrypt(vint input, vint key, vint iv, EVP_CIPHER_CTX *e){
        if(input.size() != 2){ //4 is hardcoded for 256 bit input
            int size = 2-input.size();
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

        EVP_DecryptInit_ex(e, EVP_aes_128_cbc(), NULL, aes_key, aes_iv);
        //memcpy((void *) EVP_CIPHER_CTX_iv(e), aes_iv, AES_BLOCK_SIZE); // set new IV value
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
    static  uint64_t gfmulPCF(uint64_t a, uint64_t b){
        auto aepi64 = _mm_set_epi64x(0,a);
        auto bepi64 = _mm_set_epi64x(0,b);
        auto res = _mm_clmulepi64_si128(aepi64, bepi64, 0);
        return _mm_extract_epi64(res, 0);
    }

//may not be correct
    static vint gfmulPCF(vint a, vint b){
        vint res;
        for (int i = 0; i < a.size(); ++i) {
            res.push_back(gfmulPCF(a[i], b[i]));
        }
        return res;
    }



     vint hash(vint& x, vint& y, vint tweak, int internalLength){
        //split Y perform hashRTCCR::gfmulPCF(Y/2,Y/2)
        vint yFirstHalf(y.begin(),y.begin()+(y.size()/2));
        vint ySecondHalf(y.begin()+(y.size()/2),y.end());
        auto y0 = gfmulPCF(u1, yFirstHalf);
        auto y1 = gfmulPCF(u2, ySecondHalf);
        //compute e ^ U(Y)
        y0.reserve(y0.size()*2);
        y0.insert(y0.end(), y1.begin(),y1.end());
        vint block0 = util::vecXOR(x,y0);
        //block0.emplace_back(tweak);
        if (!tweak.empty()){
        block0.insert(block0.end(),tweak.begin(),tweak.end());
        }
        //create the counters as 64 bit blocks
        vint input(internalLength/64);
        std::iota(input.begin(), input.end(), 1);
        input = util::vecXOR(input,block0);
        //initwith key
        vint res = AES_vint_encrypt(input,key,iv, e);

        return res;
    }

    vint prfHash(vint& x , vint & y, vint tweak, int internalLength){
        vint block0 = util::vecXOR(util::vecXOR(x,y), tweak);
        vint input(internalLength/64);
        std::iota(input.begin(), input.end(), 1);
        input= util::vecXOR(input,block0);
        return AES_vint_encrypt(input,key,iv, e);
    }

     vint decypthash(vint ciphertext){
        //split Y perform hashRTCCR::gfmulPCF(Y/2,Y/2)
         vint res = AES_vint_decrypt(std::move(ciphertext),key,iv,e);
        return res;
    }
};

#endif //GARBLINGGATES_HASHTCCR_H
