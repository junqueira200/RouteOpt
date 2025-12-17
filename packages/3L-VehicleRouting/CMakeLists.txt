cmake_minimum_required(VERSION 3.10)
project(threeL_VRP)

add_library(THREE_L_VRP_INCLUDES INTERFACE)
target_include_directories(THREE_L_VRP_INCLUDES INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/
)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
find_package(Boost REQUIRED)
find_package(GUROBI REQUIRED)
include_directories(${GUROBI_INCLUDE_DIRS})
find_package(ortools CONFIG REQUIRED)

file(GLOB_RECURSE BRANCHING_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
)
add_subdirectory(CvrpSep)
include_directories(DW_Decomp/include)

target_sources(THREE_L_VRP_INCLUDES INTERFACE ${BRANCHING_SOURCES})
target_link_libraries(THREE_L_VRP_INCLUDES INTERFACE gurobi_c++ gurobi ortools::ortools)
