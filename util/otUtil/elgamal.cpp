/*
//
// Created by a on 15/02/2023.
//

#include "elgamal.h"
#include "cryptopp/elgamal.h"
#include "cryptopp/osrng.h"


tuple<Integer, Integer> elgamal::KeyGen(int keySize, Integer &mod, Integer &g) {
    AutoSeededRandomPool prng;
    Integer x;
    x.Randomize(prng, keySize);
    Integer h = a_exp_b_mod_c(g, x, mod);
    return {h, x};
}

ElGamal::PrivateKey elgamal::InitializeGroupParameters(int keySize) {
    AutoSeededRandomPool prng;
    ElGamal::PrivateKey privateKey;
    privateKey.GenerateRandomWithKeySize(prng, keySize);
    return privateKey;
}


tuple<Integer, Integer, Integer> elgamal::OGen(const Integer& mod, Integer g, int keySize) {
    AutoSeededRandomPool prng;

    Integer x = CryptoPP::Integer(prng, 2, mod);
    //Integer g = a_exp_b_mod_c(x, 2, mod);

    while(g == 1 || a_exp_b_mod_c(g, (mod-1) / 2, mod) != 1){
        g = CryptoPP::Integer(prng, 2, mod);
    }


    Integer r = CryptoPP::Integer(prng, 1, Integer::Power2(2*keySize));
    Integer h = a_exp_b_mod_c(r, 2, mod);

    return {mod, g, h};
}

string elgamal::Encrypt(string msg, const Integer& mod, const Integer& g, const Integer& h){
    ElGamal::PublicKey::DL_PublicKey_GFP publicKey;
    publicKey.Initialize(mod, g, h);
    AutoSeededRandomPool prng;
    ElGamal::Encryptor encryptor(publicKey);
    string cipher;
    StringSource cipherSource(msg, true,
                              new PK_EncryptorFilter(prng, encryptor,
                                                     new StringSink(cipher)));
    return cipher;
}

string elgamal::Decrypt(string cipher, const Integer& mod, const Integer& g, const Integer& x){
    AutoSeededRandomPool prng;
    ElGamal::PrivateKey::DL_PrivateKey_GFP privateKey;
    privateKey.Initialize(mod, g, x);
    ElGamal::Decryptor decryptor(privateKey);
    string recovered;
    StringSource plaintextSource(cipher, true,
                                 new PK_DecryptorFilter(prng, decryptor,
                                                        new StringSink(recovered)));

    return recovered;
}*/
