cmake_minimum_required(VERSION 3.10)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_COMPILER "g++")

set(EXECUTABLE_OUTPUT_PATH bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY bin)

# set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -Wextra -O3 -g3")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -Wextra -Og -g3")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall -Wextra -O3 -g0")

# set the project name and version
project(GraphDB VERSION 0.1)

include_directories(${CMAKE_SOURCE_DIR}/src)

file(GLOB_RECURSE SRCS src/*.cc)

set(sources ${SRCS})
list(REMOVE_ITEM sources ${CMAKE_CURRENT_SOURCE_DIR}/src/main/*)
add_library(graphdb STATIC ${sources})

# add the executables
add_executable(CreateDB ${CMAKE_SOURCE_DIR}/src/main/create_db.cc)
target_link_libraries(CreateDB graphdb ssl crypto stdc++fs)

add_executable(Grammar ${CMAKE_SOURCE_DIR}/src/main/grammar.cc)
target_link_libraries(Grammar graphdb ssl crypto stdc++fs)

add_executable(Query ${CMAKE_SOURCE_DIR}/src/main/execute_query.cc)
target_link_libraries(Query graphdb ssl crypto stdc++fs)
