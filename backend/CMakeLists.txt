cmake_minimum_required(VERSION 3.20)
project(forge VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")

if(CMAKE_BUILD_TYPE MATCHES Debug)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0")
elseif(CMAKE_BUILD_TYPE MATCHES Release)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -DNDEBUG")
endif()

# Dependencies
find_package(Threads REQUIRED)
find_package(OpenSSL REQUIRED)

include(FetchContent)

# Crow HTTP framework (v1.0+5 - uses Boost.Asio, matches our middleware API)
FetchContent_Declare(
    crow
    GIT_REPOSITORY https://github.com/CrowCpp/Crow.git
    GIT_TAG v1.0+5
)
FetchContent_MakeAvailable(crow)

# nlohmann/json (name must match what jwt-cpp looks for to avoid duplicate targets)
FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
)
FetchContent_MakeAvailable(nlohmann_json)

# jwt-cpp
FetchContent_Declare(
    jwt-cpp
    GIT_REPOSITORY https://github.com/Thalhammer/jwt-cpp.git
    GIT_TAG v0.7.0
)
FetchContent_MakeAvailable(jwt-cpp)

# Google Test
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.14.0
)
FetchContent_MakeAvailable(googletest)

# Source files - only services directory (not recursive to avoid picking up stubs)
file(GLOB SOURCES src/services/*.cpp)

# Create library for testability
add_library(forge_lib STATIC ${SOURCES})
target_include_directories(forge_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Try find_package for pqxx first, fallback to direct linking
find_package(libpqxx QUIET)
if(libpqxx_FOUND)
    target_link_libraries(forge_lib PUBLIC libpqxx::pqxx)
else()
    target_link_libraries(forge_lib PUBLIC pq pqxx)
endif()

target_link_libraries(forge_lib PUBLIC
    Crow::Crow
    nlohmann_json::nlohmann_json
    jwt-cpp::jwt-cpp
    OpenSSL::SSL
    OpenSSL::Crypto
    Threads::Threads
    curl
)

# Main executable
add_executable(forge src/main.cpp)
target_link_libraries(forge PRIVATE forge_lib)

# Tests
enable_testing()

file(GLOB_RECURSE TEST_SOURCES tests/*.cpp)
if(TEST_SOURCES)
    add_executable(forge_tests ${TEST_SOURCES})
    target_link_libraries(forge_tests PRIVATE
        forge_lib
        GTest::gtest_main
        GTest::gmock
    )

    include(GoogleTest)
    gtest_discover_tests(forge_tests)
endif()
