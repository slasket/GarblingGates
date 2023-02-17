//
// Created by a on 17/02/2023.
//

#ifndef GARBLINGGATES_CIRCUITPARSER_H
#define GARBLINGGATES_CIRCUITPARSER_H

#include <vector>
#include <string>

using namespace std;

class circuitParser {
public:
    static tuple<vector<::uint64_t>,vector<string>> parseCircuit(const string& circuitPath);
};


#endif //GARBLINGGATES_CIRCUITPARSER_H
