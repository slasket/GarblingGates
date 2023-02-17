//
// Created by a on 17/02/2023.
//

#include <fstream>
#include "circuitParser.h"
#include "util.h"

tuple<vector<::uint64_t>, vector<string>> circuitParser::parseCircuit(const string &circuitPath) {
    std::ifstream file(circuitPath);
    vector<::uint64_t > wires;
    vector<string> gates;
    if (file.is_open()) {
        std::string line;
        //amount of wires and gates
        std::getline(file, line);

        std::vector<std::string> wiresngates = util::split(line, ' ');
        wires = vector<uint64_t>(stoi( wiresngates[1]));
        //the first string is i/o info rest is gates
        gates = vector<string>(stoi(wiresngates[1])+2);
        //input line
        std::getline(file, line);
        string inputInfo = line;//if there is a thrid input its always hardcoded to 1
        //output line
        std::getline(file, line);
        string outputInfo = line;

        //the output is defined as the last wires in the scheme, thus and output of 3 wires would be the last 3
        gates[0] = inputInfo;
        gates[1]= outputInfo;

        //the empty line
        std::getline(file, line);
        int i = 2;
        while (std::getline(file, line)) {
            // using printf() in all tests for consistency
            gates[i] = line;
            //printf("%s", line.c_str()); printf("\n");
            i++;

        }
        file.close();
    }else{
        printf("Error reading file at: " , circuitPath.c_str());
    }

    return {wires,gates};
}
