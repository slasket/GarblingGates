//
// Created by a on 17/02/2023.
//

#ifndef GARBLINGGATES_UTIL_H
#define GARBLINGGATES_UTIL_H
#include <string>
#include <sstream>
#include <vector>
#include <iterator>
#include <iostream>
#include "circuitParser.h"

class util {
public:
    //template for splitting strings taken from:
    // https://stackoverflow.com/questions/236129/how-do-i-iterate-over-the-words-of-a-string
    template <typename Out>
    static void split(const std::string &s, char delim, Out result) {
        std::istringstream iss(s);
        std::string item;
        while (std::getline(iss, item, delim)) {
            *result++ = item;
        }
    }
    static std::vector<std::string> split(const std::string &s, char delim) {
        std::vector<std::string> elems;
        split(s, delim, std::back_inserter(elems));
        return elems;
    }
    static void printUintVec(const std::vector<uint64_t>& vec){
        for (auto const& i: vec) {
        std::cout<< i << ' ';
        }
        std::cout<< std::endl;
    }
    static void printStrVec(const std::vector<std::string>& vec){
        for (auto const& i: vec) {
            std::cout<< i << std::endl;
        }
    }

    static void printCircuit(const std::string& path){
        auto res = circuitParser::parseCircuit(path);
        util::printStrVec(res);
    }
};


#endif //GARBLINGGATES_UTIL_H
