cmake_minimum_required(VERSION 3.13)
project(GarblingGates)

set(CMAKE_CXX_STANDARD 17)

add_executable(GarblingGates main.cpp bloodcompatibility.cpp bloodcompatibility.h)
add_executable(Tests tests/test_class.cpp)
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
find_package(cryptoTools REQUIRED HINTS "C:/Program Files/cryptotools")
target_link_libraries(GarblingGates oc::cryptoTools)