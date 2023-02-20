//
// Created by svend on 017, 17-02-2023.
//

#include "toyGarblingScheme.h"

    tuple<int, int, int> toyGarblingScheme::garble(int k, int f) {
        int e = 1;
        int d = -1;
        return make_tuple(f, e, d);
    }
    int toyGarblingScheme::encode(int e, int x) {
        int X = e + x;
        return X;
    }
    int toyGarblingScheme::eval(int F, int X) {
        int Y = F + X;
        return Y;
    }
    int toyGarblingScheme::decode(int d, int Y) {
        int y = d + Y;
        return y;
    }


