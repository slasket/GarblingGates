//
// Created by a on 17/02/2023.
//

#ifndef GARBLINGGATES_CIRCUITPARSER_H
#define GARBLINGGATES_CIRCUITPARSER_H

#include <openssl/evp.h>
#include <openssl/aes.h>
#include <openssl/sha.h>
#include <random>
#include <bitset>
#include <iostream>
#include <iterator>
#include <sstream>
#include <vector>
#include <string>
#include <fstream>
#include <algorithm>
#include "util.h"

using namespace std;

class circuitParser {
public:
    static vector<string> parseCircuit(const string &circuitPath) {
        std::ifstream file(circuitPath);
        vector<string> circuit;

        if (file.is_open()) {
            std::string line;
            //amount of wires and gates
            std::getline(file, line);

            //skip comments
            while (line.substr(0,2)=="//"){
                std::getline(file,line);
            }

            line.erase(std::remove(line.begin(), line.end(), '\r'), line.cend());
            string gatesAndWires= (string) line;
            std::vector<std::string> wiresngates = util::split(line, ' ');
            //the first string is i/o info rest is gates
            circuit = vector<string>(stoi(wiresngates[0])+3);
            //input line
            std::getline(file, line);
            line.erase(std::remove(line.begin(), line.end(), '\r'), line.cend());
            string inputInfo = line;//if there is a thrid input its always hardcoded to 1
            //output line
            std::getline(file, line);
            line.erase(std::remove(line.begin(), line.end(), '\r'), line.cend());
            string outputInfo = line;

            //the output is defined as the last wires in the scheme, thus and output of 3 wires would be the last 3
            circuit[0] = gatesAndWires;
            circuit[1] = inputInfo;
            circuit[2]= outputInfo;

            //the empty line
            std::getline(file, line);
            int i = 3;
            while (std::getline(file, line)) {
                line.erase(std::remove(line.begin(), line.end(), '\r'), line.cend());
                if (line.empty()){
                    break;
                }
                circuit[i] = line;
                //printf("%s", line.c_str(),"\n");
                i++;
            }
            file.close();
        }else{
            string error = "Error reading file at: "+circuitPath;
            printf("%s\n",error.c_str());
            exit(2);
        }
        return circuit;
    }

    static void printCircuit(const string& path){
        auto res = circuitParser::parseCircuit(path);
        util::printStrVec(res);
    }
    static int inputsize(vector<string> circuit){
        int inputsize =0;
        vector<string> inputs = util::split(circuit[1],' ');
        for (int i = 1; i < inputs.size(); ++i) {
            inputsize += stoi(inputs[i]);
        }
        return inputsize;
    }
};


#endif //GARBLINGGATES_CIRCUITPARSER_H
