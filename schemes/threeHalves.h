//
// Created by svend on 001, 01-03-2023.
//

#ifndef GARBLINGGATES_THREEHALVES_H
#define GARBLINGGATES_THREEHALVES_H

#include <tuple>

using namespace std;

class threeHalves {
public:
    static tuple<int, int, int> garble(int k, int f);
    static int encode(int e, int x);
    static int eval(int F, int X);
    static int decode(int d, int Y);
};


#endif //GARBLINGGATES_THREEHALVES_H
