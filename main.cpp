#include <boost/lambda/lambda.hpp>
#include <iostream>
#include <iterator>
#include <algorithm>
#include <cryptoTools/Common/Defines.h>
#include "bloodcompatibility.h"

int main() {
    bloodcompatibility bc;
    bc.testAllCombinations();

    return 0;
}
