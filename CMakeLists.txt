cmake_minimum_required(VERSION 3.16)
project(la-gang-gang VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Qt setup
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

include(FetchContent)

FetchContent_Declare(
  sodium
  GIT_REPOSITORY https://github.com/robinlinden/libsodium-cmake.git
  GIT_TAG e5b985ad0dd235d8c4307ea3a385b45e76c74c6a
)

FetchContent_MakeAvailable(sodium)


# Tell CMake where to find OpenSSL
set(OPENSSL_ROOT_DIR "C:/Program Files/OpenSSL-Win64")
set(OPENSSL_INCLUDE_DIR "${OPENSSL_ROOT_DIR}/include")
set(OPENSSL_LIBRARIES "${OPENSSL_ROOT_DIR}/lib")

# Find OpenSSL
find_package(OpenSSL REQUIRED)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

# Archivos del filesystem (lógica)
set(FILESYSTEM_SOURCES
    src/filesystem.cpp
    include/filesystem.h
)

# Archivos de la GUI (Qt)
set(GUI_SOURCES
    gui/main.cpp
    gui/mainwindow.cpp
    gui/mainwindow.h
    gui/mainwindow.ui
    gui/menuwindow.cpp
    gui/menuwindow.h
    gui/menuwindow.ui
    gui/Images.qrc
)

# Ejecutable principal con GUI
if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(la-gang-gang
        MANUAL_FINALIZATION
        ${FILESYSTEM_SOURCES}
        ${GUI_SOURCES}
        include/security.h
        src/security.cpp
        include/encryptation.h
        src/encryptation.cpp
    )
else()
    add_executable(la-gang-gang
        ${FILESYSTEM_SOURCES}
        ${GUI_SOURCES}
    )
endif()

target_link_libraries(la-gang-gang PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

# Link OpenSSL to your target
target_link_libraries(la-gang-gang PRIVATE OpenSSL::SSL OpenSSL::Crypto)

target_link_libraries(la-gang-gang PRIVATE sodium)

target_include_directories(la-gang-gang PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Tests en consola (opcional, separados de la GUI)
add_executable(filesystem-tests
    src/mainTests.cpp
    ${FILESYSTEM_SOURCES}
)
target_link_libraries(filesystem-tests PRIVATE Qt${QT_VERSION_MAJOR}::Core)

target_include_directories(filesystem-tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Salidas
set_target_properties(la-gang-gang PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
set_target_properties(filesystem-tests PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(la-gang-gang)
endif()
