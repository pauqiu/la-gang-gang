cmake_minimum_required(VERSION 3.16)
project(la-gang-gang)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Qt6 components
find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

# Enable automatic UIC, MOC, and RCC processing
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Create executable
add_executable(la-gang-gang main.cpp)

# Link Qt6 libraries
target_link_libraries(la-gang-gang Qt6::Core Qt6::Widgets)

# Set the executable output directory
set_target_properties(la-gang-gang PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)