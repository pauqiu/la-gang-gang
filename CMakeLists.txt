cmake_minimum_required(VERSION 3.16)
project(la-gang-gang)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Create executable
add_executable(la-gang-gang
    src/main.cpp
    src/filesystem.cpp
    test/test.cpp
)

target_include_directories(la-gang-gang PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/test
)

# Set the executable output directory
set_target_properties(la-gang-gang PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)