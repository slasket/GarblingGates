//
// Created by a on 14/04/2023.
//

#ifndef GARBLINGGATES_HASHTCCR_H
#define GARBLINGGATES_HASHTCCR_H
#include <utility>
#include <emmintrin.h>
#include <immintrin.h>
#include "util.h"
#include "string.h"

using namespace std;
class hashTCCR{
public:
    //vint key;
    vint iv = {0};
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
    const int &isEmpty() const{
        if (iv[0] == 0){
            return 1;
        }else{
            return 0;
        }
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
        this->iv = util::genBitsNonCrypto(k);
        //this->key= std::move(key);
        this->u1 = util::genBitsNonCrypto((k/2));
        this->u2 = util::genBitsNonCrypto((k/2));
        this->e = AES_vint_init();
        this->hashtype = util::fast;
    }
    hashTCCR(){
    }

    static inline EVP_CIPHER_CTX * AES_vint_init(){
        EVP_CIPHER_CTX *e;
        e = EVP_CIPHER_CTX_new();
        //EVP_EncryptInit_ex(e, EVP_aes_256_cbc(), NULL, aes_key, iv);
        //EVP_DecryptInit_ex(e, EVP_aes_256_cbc(), NULL, aes_key, iv);
        EVP_CIPHER_CTX_set_padding(e, 0);
        return e;
    }


    static inline vint AES_vint_encrypt(vint input, vint key, vint iv, EVP_CIPHER_CTX *e){
        if(input.size() != 2){ //2 is hardcoded for 128 bit input
            int size = 2-input.size();
            for (int i = 0; i < size; ++i) {
                input.emplace_back(0);
            }
        }
        int len = input.size() * sizeof(uint64_t);
        int keylen = key.size() * sizeof(uint64_t);
        int c_len = len + AES_BLOCK_SIZE;
        int f_len = 0;
        auto *ciphertext = static_cast<unsigned char *>(malloc(c_len));
        //plaintext from input
        auto *plaintext = static_cast<unsigned char *>(malloc(len));
        memcpy(plaintext, input.data(), len);

        //key from key
        auto *aes_key = static_cast<unsigned char *>(malloc(keylen));
        memcpy(aes_key, key.data(), keylen);
        //iv from iv        16 block times 8 bits = 128 bits
        auto *aes_iv = static_cast<unsigned char *>(malloc(AES_BLOCK_SIZE));
        memcpy(aes_iv, iv.data(), AES_BLOCK_SIZE);

        EVP_EncryptInit_ex(e, EVP_aes_128_cbc(), NULL, aes_key, aes_iv);
        EVP_EncryptUpdate(e, ciphertext, &c_len, plaintext, len);
        EVP_EncryptFinal_ex(e, ciphertext + c_len, &f_len);
        //convert ciphertext to vint
        vint res(len/sizeof(uint64_t), 0);
        memcpy(res.data(), ciphertext, len);
        free(ciphertext);
        free(plaintext);
        free(aes_key);
        free(aes_iv);
        return res;
    }

    static inline vint AES_vint_decrypt(vint input, vint key, vint iv, EVP_CIPHER_CTX *e){
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
    static inline uint64_t gfmulPCF(uint64_t a, uint64_t b){
        auto aepi64 = _mm_set_epi64x(0,a);
        auto bepi64 = _mm_set_epi64x(0,b);
        auto res = _mm_clmulepi64_si128(aepi64, bepi64, 0);
        return _mm_extract_epi64(res, 0);
    }

//may not be correct
    static inline vint gfmulPCF(vint a, vint b){
        vint res;
        for (int i = 0; i < a.size(); ++i) {
            res.push_back(gfmulPCF(a[i], b[i]));
        }
        return res;
    }



    static inline vint hash(vint &x, vint &y, const vint& iv, EVP_CIPHER_CTX *e, const vint &u1, const vint &u2, int tweak, int internalLength){
        //split Y perform hashRTCCR::gfmulPCF(Y/2,Y/2)
        vint yFirstHalf(y.begin(),y.begin()+(y.size()/2));
        vint ySecondHalf(y.begin()+(y.size()/2),y.end());
        auto y0 = gfmulPCF(u1, yFirstHalf);
        auto y1 = gfmulPCF(u2, ySecondHalf);
        //compute X ^ U(Y)
        y0.insert(y0.end(), y1.begin(),y1.end());
        vint key = util::vecXOR(x,y0);
        vint input;
        //create the counters as 64 bit blocks
        for (int i = 0; i < (internalLength/64); ++i) {
            if (i==0){
                input.emplace_back(i^tweak);
            }else{
                input.emplace_back(i);
            }
        }
        //initwith key
        vint res = AES_vint_encrypt(input,key,iv,e);

        return res;
    }

    static inline vint decypthash(vint &x, vint &y, const vint& iv, EVP_CIPHER_CTX *e, const vint &u1, const vint &u2, vint ciphertext){
        //split Y perform hashRTCCR::gfmulPCF(Y/2,Y/2)
        vint yFirstHalf(y.begin(),y.begin()+(y.size()/2));
        vint ySecondHalf(y.begin()+(y.size()/2),y.end());
        auto y0 = gfmulPCF(u1, yFirstHalf);
        auto y1 = gfmulPCF(u2, ySecondHalf);
        //compute X ^ U(Y)
        y0.insert(y0.end(), y1.begin(),y1.end());
        vint key = util::vecXOR(x,y0);
        vint res = AES_vint_decrypt(std::move(ciphertext),key,iv,e);
        return res;
    }
};

#endif //GARBLINGGATES_HASHTCCR_H
