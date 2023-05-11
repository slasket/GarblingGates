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

namespace parserSpace {
    typedef vector<tuple<vector<int>,vector<int>,string>> circuit;
}
using namespace parserSpace;
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
    static vector<tuple<vector<int>,vector<int>,string>> parse(const string &circuitPath) {
        std::ifstream file(circuitPath);
        vector<tuple<vector<int>,vector<int>,string>> c;
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
            std::vector<std::string> wiresngates = split(line, ' ');
            //the first string is i/o info rest is gates
            c = vector<tuple<vector<int>,vector<int>,string>>(stoi(wiresngates[0])+2);
            c[0] = {{stoi(wiresngates[0])},{stoi(wiresngates[1])},""};
            //input line
            std::getline(file, line);
            line.erase(std::remove(line.begin(), line.end(), '\r'), line.cend());
            vector<string> inputInfo = split(line, ' ');//if there is a thrid input its always hardcoded to 1
            //output line
            std::getline(file, line);
            line.erase(std::remove(line.begin(), line.end(), '\r'), line.cend());
            vector<string> outputInfo = split(line, ' ');
            int inputsize =0;
            for (int i = 1; i < inputInfo.size(); ++i) {
                inputsize+=stoi(inputInfo[i]);
            }
            int outsize = stoi(outputInfo[1]);
            c[1] = {{inputsize},{outsize},""};
            //the empty line
            std::getline(file, line);
            int i = 2;
            while (std::getline(file, line)) {
                line.erase(std::remove(line.begin(), line.end(), '\r'), line.cend());
                if (line.empty()){
                    break;
                }
                tuple<vector<int>,vector<int>,string> val;
                wiresngates = split(line, ' ');
                if (wiresngates[0]=="2"){
                    c[i]={{stoi(wiresngates[2]),stoi(wiresngates[3])},{stoi(wiresngates[4])},wiresngates[5]};
                }else{
                    c[i]={{stoi(wiresngates[2])},{stoi(wiresngates[3])},wiresngates[4]};
                }
                //printf("%s", line.c_str(),"\n");
                i++;
            }
            file.close();
        }else{
            string error = "Error reading file at: "+circuitPath;
            printf("%s\n",error.c_str());
            exit(2);
        }
        return c;
    }

    //template for splitting strings taken from:
    // https://stackoverflow.com/questions/236129/how-do-i-iterate-over-the-words-of-a-string
    template <typename Out>
    static  void split(const std::string &s, char delim, Out result) {
        std::istringstream iss(s);
        std::string item;
        while (std::getline(iss, item, delim)) {
            *result++ = item;
        }
    }
    static  std::vector<std::string> split(const std::string &s, char delim) {
        std::vector<std::string> elems;
        split(s, delim, std::back_inserter(elems));
        return elems;
    }


    static void printCircuit(const string& path){
        auto res = circuitParser::parseCircuit(path);
        util::printStrVec(res);
    }
    static int getInputSize(circuit & c){
        return get<0>(c[1])[0];
    }

    static int getWires(circuit& c){
        return get<1>(c[0])[0];
    }
    static int getGates(circuit& c){
        return get<0>(c[0])[0];
    }
    static int getOutBits(circuit& c){
        return get<1>(c[1])[0];
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
