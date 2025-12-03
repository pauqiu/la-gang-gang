cmake_minimum_required(VERSION 3.16)
project(la-gang-gang VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Qt setup
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

include(FetchContent)

# libsodium
FetchContent_Declare(
  sodium
  GIT_REPOSITORY https://github.com/robinlinden/libsodium-cmake.git
  GIT_TAG e5b985ad0dd235d8c4307ea3a385b45e76c74c6a
)
FetchContent_MakeAvailable(sodium)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

# -----------------------------
# Fuentes del filesystem
# -----------------------------
set(FILESYSTEM_SOURCES
    src/filesystem.cpp
    include/filesystem.h
    include/logger.h
)

# -----------------------------
# Fuentes de RightsValidation
# -----------------------------
set(RIGHTS_SOURCES
    src/RightsValidation/RightsValidation.cpp
    include/RightsValidation/RightsValidation.h
    src/RightsValidation/RolesFileManager.cpp
    include/RightsValidation/RolesFileManager.h
    src/RightsValidation/RoleManager.cpp
    include/RightsValidation/RoleManager.h
    src/RightsValidation/PermissionsManager.cpp
    include/RightsValidation/PermissionsManager.h
    include/permissions_constants.h
)

# -----------------------------
# Fuentes de GUI
# -----------------------------
set(GUI_SOURCES
    gui/main.cpp
    gui/mainwindow.cpp
    gui/mainwindow.h
    gui/mainwindow.ui
    gui/menuwindow.cpp
    gui/menuwindow.h
    gui/menuwindow.ui
    gui/Images.qrc
    gui/adduserwindow.cpp
    gui/adduserwindow.h
    gui/adduserwindow.ui
    gui/addrolewindow.h
    gui/addrolewindow.cpp
    gui/addrolewindow.ui
)

# -----------------------------
# Fuentes de seguridad / encryption
# -----------------------------
set(SECURITY_SOURCES
    include/security.h
    src/security.cpp
    include/encryptation.h
    src/encryptation.cpp
    gui/Role.h
)

# -----------------------------
# Fuentes de sockets / comunicación
# -----------------------------
set(SOCKETS_SOURCES
    sockets/message_dispatcher.h
    sockets/messages.h
    sockets/node_base.h
    sockets/nodeClient.h
    sockets/nodeAuth.h
    sockets/communication.h
    sockets/nodeStorage.h
    sockets/nodeReceptor.h
    sockets/nodeProxy.h
    sockets/endpoints.h
    sockets/endpoints.cpp
)

# =====================================
# Ejecutable GUI principal
# =====================================
if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(la-gang-gang
        MANUAL_FINALIZATION
        ${FILESYSTEM_SOURCES}
        ${RIGHTS_SOURCES}
        ${GUI_SOURCES}
        ${SECURITY_SOURCES}
        ${SOCKETS_SOURCES}
        sockets/nodeReceptor.h  # solo NodeClient se usa aquí
    )
else()
    add_executable(la-gang-gang
        ${FILESYSTEM_SOURCES}
        ${RIGHTS_SOURCES}
        ${GUI_SOURCES}
        ${SECURITY_SOURCES}
        ${SOCKETS_SOURCES}
    )
endif()

target_link_libraries(la-gang-gang PRIVATE Qt${QT_VERSION_MAJOR}::Widgets sodium)
target_include_directories(la-gang-gang PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(la-gang-gang)
endif()

# =====================================
# Ejecutable nodo Auth independiente
# =====================================
add_executable(nodeAuthMain
    sockets/authMain.cpp
    src/security.cpp
    src/filesystem.cpp
    src/encryptation.cpp
    sockets/endpoints.cpp
)
target_include_directories(nodeAuthMain PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(nodeAuthMain PRIVATE sodium Qt${QT_VERSION_MAJOR}::Core)

# =====================================
# Ejecutable nodo Storage independiente
# =====================================
add_executable(nodeStorageMain
    sockets/storageMain.cpp
    src/filesystem.cpp
    sockets/endpoints.cpp
)
target_include_directories(nodeStorageMain PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(nodeStorageMain PRIVATE Qt${QT_VERSION_MAJOR}::Core)
set_target_properties(nodeStorageMain PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# =====================================
# Ejecutable nodo Receptor
# =====================================
add_executable(nodeReceptor
    sockets/ReceptorMain.cpp
    src/filesystem.cpp
)
target_include_directories(nodeReceptor PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(nodeReceptor PRIVATE Qt${QT_VERSION_MAJOR}::Core)
set_target_properties(nodeReceptor PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# =====================================
# Cliente de prueba para Storage
# =====================================
add_executable(storageTestClient
    sockets/storageTestClient.cpp
    src/filesystem.cpp
)
target_include_directories(storageTestClient PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(storageTestClient PRIVATE Qt${QT_VERSION_MAJOR}::Core)
set_target_properties(storageTestClient PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# =====================================
# Cliente de prueba para solicitud de logs
# =====================================
add_executable(test_log_request
    sockets/test_log_request.cpp
    sockets/endpoints.cpp
)
target_include_directories(test_log_request PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(test_log_request PRIVATE Qt${QT_VERSION_MAJOR}::Core)
set_target_properties(test_log_request PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# =====================================
# Test de health check
# =====================================
add_executable(test_health_check
    sockets/test_health_check.cpp
    sockets/endpoints.cpp
)
target_include_directories(test_health_check PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(test_health_check PRIVATE Qt${QT_VERSION_MAJOR}::Core)
set_target_properties(test_health_check PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# =====================================
# Test de round-robin
# =====================================
add_executable(test_round_robin
    sockets/test_round_robin.cpp
    sockets/endpoints.cpp
)
target_include_directories(test_round_robin PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(test_round_robin PRIVATE Qt${QT_VERSION_MAJOR}::Core)
set_target_properties(test_round_robin PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# =====================================
# Ejecutable nodo Proxy independiente
# =====================================
add_executable(proxyMain
    sockets/proxyMain.cpp
    src/filesystem.cpp
    sockets/endpoints.cpp
)
target_include_directories(proxyMain PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(proxyMain PRIVATE Qt${QT_VERSION_MAJOR}::Core)
set_target_properties(proxyMain PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# =====================================
# Tests de filesystem (opcional)
# =====================================
add_executable(filesystem-tests
    src/mainTests.cpp
    ${FILESYSTEM_SOURCES}
)
target_link_libraries(filesystem-tests PRIVATE Qt${QT_VERSION_MAJOR}::Core)
target_include_directories(filesystem-tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

# =====================================
# Salidas
# =====================================
set_target_properties(la-gang-gang PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
set_target_properties(nodeAuthMain PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
set_target_properties(filesystem-tests PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# =====================================
# Copiar endpoints.txt al directorio bin
# =====================================
configure_file(
    ${CMAKE_SOURCE_DIR}/endpoints.txt
    ${CMAKE_BINARY_DIR}/bin/endpoints.txt
    COPYONLY
)
