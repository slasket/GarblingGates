cmake_minimum_required(VERSION 3.13)
project(GarblingGates)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS -O2)
#set(CMAKE_CXX_FLAGS -D_GLIBCXX_DEBUG)


set(SOURCES
        bloodcompatibility.cpp
        bloodcompatibility.h
        util/circuitParser.cpp
        util/circuitParser.h
        util/util.h
        util/otUtil/elgamal.cpp
        util/otUtil/elgamal.h
        util/otUtil/OT.cpp
        util/otUtil/OT.h
        util/otUtil/otUtil.cpp
        util/otUtil/otUtil.h
        HashFunctions.cpp
        HashFunctions.h
        schemes/atecaGarble.cpp
        schemes/atecaGarble.h
        schemes/atecaFreeXOR.cpp
        schemes/atecaFreeXOR.h
        schemes/toyExample/toyGarblingScheme.cpp
        schemes/toyExample/toyGarblingScheme.h
        schemes/baseGarble.cpp
        schemes/baseGarble.h
        schemes/threeHalves.cpp schemes/threeHalves.h)

set(TESTS
        tests/test_class.cpp
        tests/circuitParserTest.cpp
        tests/test_toyExample.cpp
        schemes/baseGarble.cpp
        schemes/baseGarble.h
        tests/test_baseline.cpp
        tests/test_util.cpp
        schemes/atecaGarble.cpp
        schemes/atecaGarble.h
        HashFunctions.cpp
        schemes/atecaGarble.cpp
        tests/test_ateca.cpp
        tests/test_threeHalves.cpp schemes/atecaFreeXOR.cpp schemes/atecaFreeXOR.h tests/test_atecaFreeXOR.cpp)


add_executable(GarblingGates ${SOURCES} main.cpp)
add_executable(Tests ${TESTS} ${SOURCES})

#boost
find_package(Boost REQUIRED)
target_link_libraries(GarblingGates ${Boost_LIBRARIES})
include_directories(${Boost_INCLUDE_DIR})

##cryptotool
#find_package(cryptoTools REQUIRED HINTS "C:/Program Files/cryptotools/lib")
#target_link_libraries(GarblingGates PRIVATE oc::cryptoTools)
#target_link_libraries(Tests PRIVATE oc::cryptoTools)

#cryptoPP
find_package(cryptopp REQUIRED)
target_link_libraries(GarblingGates PRIVATE cryptopp::cryptopp)
target_link_libraries(Tests PRIVATE cryptopp::cryptopp)


#openSSL
find_package(OpenSSL 3 REQUIRED)
target_link_libraries(GarblingGates PRIVATE OpenSSL::Crypto)
target_link_libraries(Tests PRIVATE OpenSSL::Crypto)