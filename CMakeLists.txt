cmake_minimum_required(VERSION 3.13)
project(GarblingGates)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS -O2)
#set(CMAKE_CXX_FLAGS -D_GLIBCXX_DEBUG)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mpclmul")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mpclmul")

set(SOURCES
        bloodcompatibility.cpp
        bloodcompatibility.h
        util/circuitParser.h
        util/util.h
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
        schemes/threeHalves.cpp
        schemes/threeHalves.h
        util/hashRTCCR.h
        )

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
        tests/test_threeHalves.cpp
        schemes/atecaFreeXOR.cpp
        schemes/atecaFreeXOR.h
        tests/test_atecaFreeXOR.cpp
        tests/test_hashTCCR.cpp)


add_executable(GarblingGates ${SOURCES} main.cpp)
add_executable(Tests ${TESTS} ${SOURCES})

#boost
find_package(Boost REQUIRED)
target_link_libraries(GarblingGates ${Boost_LIBRARIES})
include_directories(${Boost_INCLUDE_DIR})

#openSSL
find_package(OpenSSL 3 REQUIRED)
target_link_libraries(GarblingGates PRIVATE OpenSSL::Crypto)
target_link_libraries(Tests PRIVATE OpenSSL::Crypto)