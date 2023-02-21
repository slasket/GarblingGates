//
// Created by a on 21/02/2023.
//

#ifndef GARBLINGGATES_ATECAGARBELING_H
#define GARBLINGGATES_ATECAGARBELING_H


#include <vector>
#include <string>

using namespace std;
class atecaGarbeling {

public:
    class scheme {
        static string garble(int secParam, vector<std::string> circuit, vector<::uint64_t> input);
        static vector<::uint64_t> encode(string randomness, vector<::uint64_t> input);
    private:
        static string init(vector<std::string> circuit, int externalLength);
    };

};

#endif //GARBLINGGATES_ATECAGARBELING_H
