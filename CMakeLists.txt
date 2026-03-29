cmake_minimum_required(VERSION 3.20)
project(Q2ERestoreRecoil LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SDK_DIR "${CMAKE_SOURCE_DIR}/sdk")

if(IS_DIRECTORY "${SDK_DIR}" AND EXISTS "${SDK_DIR}/CMakeLists.txt")
    # Full build: compile SDK sources with our weapon file replacement
    file(GLOB SDK_SOURCES
        "${SDK_DIR}/*.cpp"
    )

    # Exclude SDK's p_weapon.cpp — we provide our own with recoil restored
    list(FILTER SDK_SOURCES EXCLUDE REGEX ".*p_weapon\\.cpp$")

    add_library(game SHARED
        ${SDK_SOURCES}
        src/p_weapon.cpp
    )

    target_include_directories(game PRIVATE
        ${SDK_DIR}
    )
else()
    # Stub build: verify cross-compilation toolchain works
    message(STATUS "SDK not found at ${SDK_DIR} — building stub DLL")
    message(STATUS "Clone SDK: git submodule update --init")

    add_library(game SHARED
        src/stub.cpp
    )
endif()

set_target_properties(game PROPERTIES
    OUTPUT_NAME "gamex64"
    PREFIX ""
)
