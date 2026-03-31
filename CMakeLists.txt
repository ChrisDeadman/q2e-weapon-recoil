cmake_minimum_required(VERSION 3.20)
project(Q2EWeaponRecoil LANGUAGES CXX)

include(CTest)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

set(RERELEASE_DIR "${CMAKE_SOURCE_DIR}/sdk/rerelease")

if(NOT IS_DIRECTORY "${RERELEASE_DIR}")
    message(FATAL_ERROR "Missing rerelease SDK at ${RERELEASE_DIR}; run git submodule update --init --recursive")
endif()

if(NOT IS_DIRECTORY "${CMAKE_SOURCE_DIR}/external/fmt")
    message(FATAL_ERROR "Missing fmt dependency at ${CMAKE_SOURCE_DIR}/external/fmt; run git submodule update --init --recursive")
endif()

if(NOT IS_DIRECTORY "${CMAKE_SOURCE_DIR}/external/jsoncpp")
    message(FATAL_ERROR "Missing jsoncpp dependency at ${CMAKE_SOURCE_DIR}/external/jsoncpp; run git submodule update --init --recursive")
endif()

set(FMT_DOC OFF CACHE BOOL "" FORCE)
set(FMT_INSTALL OFF CACHE BOOL "" FORCE)
set(FMT_TEST OFF CACHE BOOL "" FORCE)

set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(BUILD_STATIC_LIBS ON CACHE BOOL "" FORCE)
set(BUILD_OBJECT_LIBS OFF CACHE BOOL "" FORCE)
set(JSONCPP_WITH_TESTS OFF CACHE BOOL "" FORCE)
set(JSONCPP_WITH_POST_BUILD_UNITTEST OFF CACHE BOOL "" FORCE)
set(JSONCPP_WITH_WARNING_AS_ERROR OFF CACHE BOOL "" FORCE)
set(JSONCPP_WITH_STRICT_ISO OFF CACHE BOOL "" FORCE)

add_subdirectory(external/fmt EXCLUDE_FROM_ALL)
add_subdirectory(external/jsoncpp EXCLUDE_FROM_ALL)

add_library(recoil_model STATIC
    sdk/rerelease/recoil_model.cpp
)

target_include_directories(recoil_model PUBLIC
    "${RERELEASE_DIR}"
)

file(GLOB_RECURSE Q2E_SOURCES CONFIGURE_DEPENDS
    "${RERELEASE_DIR}/*.cpp"
)

list(REMOVE_ITEM Q2E_SOURCES "${RERELEASE_DIR}/recoil_model.cpp")

add_library(game SHARED ${Q2E_SOURCES})

set_property(TARGET game PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)

target_include_directories(game PRIVATE
    "${RERELEASE_DIR}"
)

target_compile_definitions(game PRIVATE
    KEX_Q2_GAME
    KEX_Q2GAME_EXPORTS
    KEX_Q2GAME_DYNAMIC
    NO_FMT_SOURCE
    _CRT_SECURE_NO_WARNINGS
)

target_link_libraries(game PRIVATE
    recoil_model
    fmt::fmt
    jsoncpp_static
)

target_compile_options(game PRIVATE
    $<$<CXX_COMPILER_ID:GNU>:-O3>
    $<$<CXX_COMPILER_ID:GNU>:-ffunction-sections>
    $<$<CXX_COMPILER_ID:GNU>:-fdata-sections>
    $<$<CXX_COMPILER_ID:GNU>:-fno-semantic-interposition>
    $<$<CXX_COMPILER_ID:GNU>:-fno-math-errno>
    $<$<CXX_COMPILER_ID:GNU>:-fno-trapping-math>
    $<$<CXX_COMPILER_ID:GNU>:-ffp-contract=fast>
    $<$<CXX_COMPILER_ID:Clang>:-O3>
    $<$<CXX_COMPILER_ID:Clang>:-ffunction-sections>
    $<$<CXX_COMPILER_ID:Clang>:-fdata-sections>
    $<$<CXX_COMPILER_ID:Clang>:-fno-math-errno>
    $<$<CXX_COMPILER_ID:Clang>:-fno-trapping-math>
    $<$<CXX_COMPILER_ID:Clang>:-ffp-contract=fast>
)

target_link_options(game PRIVATE
    -static-libgcc
    -static-libstdc++
    -Wl,--gc-sections
)

set_target_properties(game PROPERTIES
    OUTPUT_NAME "game_x64"
    PREFIX ""
)

set(DIST_DIR "${CMAKE_BINARY_DIR}/dist/q2e-weapon-recoil")
set(LEGACY_STUB_DLL "${CMAKE_BINARY_DIR}/gamex64.dll")
set(LEGACY_STUB_IMPORT_LIB "${CMAKE_BINARY_DIR}/libgamex64.dll.a")

add_custom_command(TARGET game POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E make_directory "${DIST_DIR}"
    COMMAND "${CMAKE_COMMAND}" -E rm -f "${LEGACY_STUB_DLL}"
    COMMAND "${CMAKE_COMMAND}" -E rm -f "${LEGACY_STUB_IMPORT_LIB}"
    COMMAND "${CMAKE_COMMAND}" -E copy_if_different "$<TARGET_FILE:game>" "${DIST_DIR}/$<TARGET_FILE_NAME:game>"
    COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CMAKE_SOURCE_DIR}/description.txt" "${DIST_DIR}/description.txt"
    VERBATIM
)

if(BUILD_TESTING AND NOT CMAKE_CROSSCOMPILING)
    add_subdirectory(external/fmt/test/gtest EXCLUDE_FROM_ALL)

    add_executable(recoil_model_tests
        tests/recoil_model_tests.cpp
    )

    target_include_directories(recoil_model_tests PRIVATE
        "${RERELEASE_DIR}"
    )

    target_link_libraries(recoil_model_tests PRIVATE
        recoil_model
        gtest
    )

    add_test(NAME recoil_model_tests COMMAND recoil_model_tests)
endif()
