//
// Created by svend on 020, 20-02-2023.
//

#ifndef GARBLINGGATES_BASEGARBLE_H
#define GARBLINGGATES_BASEGARBLE_H

#include <vector>
#include <string>

using namespace std;
class baseGarble {
public:
    static int garble(int k, vector<string> f);
    static int encode(int e, int x);
    static int eval(int F, int X);
    static int decode(int d, int Y);
};


#endif //GARBLINGGATES_BASEGARBLE_H
