//
// Created by svend on 020, 20-02-2023.
//

#include "baseGarble.h"

//k is security parameter and f is the function to be garbled with 3 lines of metadata
int baseGarble::garble(int k, vector<string> f) {
    string &metadata1 = f[0];
    string &metadata2 = f[1];
    string &metadata3 = f[2];
    //perform gate by gate garbling
    for (int i = 3; i < f.size(); ++i) {
        string &line = f[i];
        int numInputWires = stoi(line.substr(0, line.find(" ")));
        int numOutputWires = stoi(line.substr(line.find(" ") + 1, line.find(" ", line.find(" ") + 1)));
        //handle input wires
        for (int j = 0; j < numInputWires; ++j) {
            //get next input wire label/index
            
        }
    }
    return 0;
}

int baseGarble::encode(int e, int x) {
    return 0;
}

int baseGarble::eval(int F, int X) {
    return 0;
}

int baseGarble::decode(int d, int Y) {
    return 0;
}

}
