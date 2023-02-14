#include <boost/lambda/lambda.hpp>
#include <iostream>
#include <iterator>
#include <algorithm>
#include <cryptoTools/Common/Defines.h>

int main() {
    std::cout << "Hello, World!" << std::endl;

    using namespace boost::lambda;
    typedef std::istream_iterator<int> in;

    oc::u64 a = 0;
    oc::u64 b = 1;
    oc::u64 c = a + b;
    std::cout << c << std::endl;


    std::for_each(
            in(std::cin), in(), std::cout << (_1 * 3) << " " );


    return 0;
}
