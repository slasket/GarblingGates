//
// Created by svend on 014, 14-02-2023.
//

#include <iostream>
#include "bloodcompatibility.h"



int bloodcompatibility::protocol(int i, int j) {
    return 0; //Here goes the Garbled Gates protocol for blood compatibility
}

int bloodcompatibility::bloodCompLookup(int i, int j) {
    int table[8][8] = {
            {1, 0, 0, 0, 0, 0, 0, 0},  // o- /0
            {1, 1, 0, 0, 0, 0, 0, 0},  // o+ /1
            {1, 0, 1, 0, 0, 0, 0, 0},  // b- /2
            {1, 1, 1, 1, 0, 0, 0, 0},  // b+ /3
            {1, 0, 0, 0, 1, 0, 0, 0},  // a- /4
            {1, 1, 0, 0, 1, 1, 0, 0},  // a+ /5
            {1, 0, 1, 0, 1, 0, 1, 0},  // ab-/6
            {1, 1, 1, 1, 1, 1, 1, 1},  // ab+/7
    };
    return table[i][j];
}

//define AND gate
int AND(int a, int b) {
    return a & b;
}

//define OR gate
int OR(int a, int b) {
    return a | b;
}

int check_nth_bit(int num, int n){  //# From https://stackoverflow.com/questions/18111488/convert-integer-to-binary-in-python-and-compare-the-bits
    return (num >> n) & 1;
}

int bloodCompTest(int d, int r) {
    int dA = check_nth_bit(d, 2);
    int dB = check_nth_bit(d, 1);
    int dPos = check_nth_bit(d, 0);

    int rA = check_nth_bit(r, 2);
    int rB = check_nth_bit(r, 1);
    int rPos = check_nth_bit(r, 0);

    return AND(AND( OR(dPos, 1^ rPos),OR(dB, 1^ rB)),OR(dA, 1^ rA));  // Makes sure r is "less than" d on each bit
}

int bloodcompatibility::testAllCombinations() {
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            //int protolres = protocol(i, j);
            int protolres = bloodCompTest(i, j);
            if (protolres != bloodCompLookup(i,j)) {
                std::cout << "Error: " << i << " & " << j << " != " << protolres << std::endl;
            }
        }
    }
    return 0;
}
