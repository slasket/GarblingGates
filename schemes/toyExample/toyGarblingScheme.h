//
// Created by svend on 017, 17-02-2023.
//

#ifndef GARBLINGGATES_TOYGARBLINGSCHEME_H
#define GARBLINGGATES_TOYGARBLINGSCHEME_H
#include "tuple"
using namespace std;

class toyGarblingScheme{
public:
    static tuple<int, int, int> garble(int k, int f);
    static int encode(int e, int x);
    static int eval(int F, int X);
    static int decode(int d, int Y);
};


#endif //GARBLINGGATES_TOYGARBLINGSCHEME_H
