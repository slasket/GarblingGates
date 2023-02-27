//
// Created by a on 17/02/2023.
//

#include <fstream>
#include "circuitParser.h"
#include "util.h"

vector<string> circuitParser::parseCircuit(const string &circuitPath) {
    std::ifstream file(circuitPath);
    vector<string> circuit;

    if (file.is_open()) {
        std::string line;
        //amount of wires and gates
        std::getline(file, line);

        string gatesAndWires= (string) line;
        std::vector<std::string> wiresngates = util::split(line, ' ');
        //the first string is i/o info rest is gates
        circuit = vector<string>(stoi(wiresngates[0])+3);
        //input line
        std::getline(file, line);
        string inputInfo = line;//if there is a thrid input its always hardcoded to 1
        //output line
        std::getline(file, line);
        string outputInfo = line;

        //the output is defined as the last wires in the scheme, thus and output of 3 wires would be the last 3
        circuit[0] = gatesAndWires;
        circuit[1] = inputInfo;
        circuit[2]= outputInfo;

        //the empty line
        std::getline(file, line);
        int i = 3;
        while (std::getline(file, line)) {
            if (line.empty()){
                break;
            }
            circuit[i] = line;
            //printf("%s", line.c_str(),"\n");
            i++;
        }
        file.close();
    }else{
        printf("Error reading file at: " , circuitPath.c_str());
    }

    return circuit;
}
