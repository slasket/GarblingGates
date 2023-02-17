cmake_minimum_required(VERSION 3.13)
project(GarblingGates)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS -O2)

add_executable(GarblingGates main.cpp bloodcompatibility.cpp bloodcompatibility.h util/otUtil/OT.cpp util/otUtil/OT.h util/otUtil/util.cpp util/otUtil/util.h util/otUtil/elgamal.cpp util/otUtil/elgamal.h garblingScheme.cpp garblingScheme.h toyExample/toyGarblingScheme.cpp toyExample/toyGarblingScheme.h)
add_executable(Tests tests/test_class.cpp toyExample/toyGarblingScheme.cpp)

#local
file(GLOB SOURCES
        *.hpp
        *.cpp
        )

#boost
find_package(Boost REQUIRED)
target_link_libraries(GarblingGates ${Boost_LIBRARIES})
include_directories(${Boost_INCLUDE_DIR})

#cryptotool
find_package(cryptoTools REQUIRED HINTS "C:/Program Files/cryptotools/lib")
target_link_libraries(GarblingGates PRIVATE oc::cryptoTools)

#cryptoPP
find_package(cryptopp REQUIRED)
target_link_libraries(GarblingGates PRIVATE cryptopp::cryptopp)