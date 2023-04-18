/*
//
// Created by a on 15/02/2023.
//
#include <iostream>
#include <cryptopp/integer.h>
#include <cryptopp/elgamal.h>
#include "cryptopp./elgamal.h"
#include "cryptopp./osrng.h"
#include "cassert"

#ifndef GARBLINGGATES_ELGAMAL_H
#define GARBLINGGATES_ELGAMAL_H


using namespace CryptoPP;
using namespace std;

class elgamal {


public:

    static tuple<Integer, Integer, Integer> OGen(const Integer& mod, Integer g, int keySize);

    static string Encrypt(string msg, const CryptoPP::Integer &mod, const CryptoPP::Integer &g, const CryptoPP::Integer &h);

    static string Decrypt(string cipher, const Integer &mod, const Integer &g, const Integer &x);

    static ElGamal::PrivateKey InitializeGroupParameters(int keySize);

    static tuple<Integer, Integer> KeyGen(int keySize, Integer &mod, Integer &g);
};

#endif //GARBLINGGATES_ELGAMAL_H
*/
