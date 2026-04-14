cmake_minimum_required(VERSION 3.16)
project(osint-x VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(CURL REQUIRED)

include_directories(
    src
    include
    /workspaces/osint-x/include/crow/include
)

# CLI binary
add_executable(osint-x
    src/main.cpp
)
target_link_libraries(osint-x CURL::libcurl pthread)

# Web server binary
add_executable(osint-web
    src/web/WebServer.cpp
)
target_link_libraries(osint-web CURL::libcurl pthread)

# Compiler flags
target_compile_options(osint-x   PRIVATE -Wall -O2)
target_compile_options(osint-web PRIVATE -Wall -O2)