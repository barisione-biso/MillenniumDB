cmake_minimum_required(VERSION 3.10)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_COMPILER "g++")

set(EXECUTABLE_OUTPUT_PATH bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY bin)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -Wextra -g")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall -Wextra -O2")

# set the project name and version
project(GraphDB VERSION 0.1)

include_directories(${CMAKE_SOURCE_DIR}/src)

file(GLOB_RECURSE SRCS src/*.cc)
add_library(graphdb_shared SHARED ${SRCS})

# add the executables
add_executable(CreateDB ${CMAKE_SOURCE_DIR}/main/create_db.cc)
target_link_libraries(CreateDB graphdb_shared ssl crypto stdc++fs)

add_executable(Grammar ${CMAKE_SOURCE_DIR}/main/grammar.cc)
target_link_libraries(Grammar graphdb_shared ssl crypto stdc++fs)

add_executable(Query ${CMAKE_SOURCE_DIR}/main/execute_query.cc)
target_link_libraries(Query graphdb_shared ssl crypto stdc++fs)