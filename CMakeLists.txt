cmake_minimum_required(VERSION 3.14)
project(pbqp-papa LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(PBQP_USE_GUROBI "Enable Gurobi solver support" OFF)
option(PBQP_USE_GVC "Enable Graphviz visualization support" ON)

add_library(pbqp-core STATIC
    src/CInterface.cpp
    src/debug/DebugTimer.cpp
    src/debug/StatKeeper.cpp
    src/InfinityWrapper.cpp
    src/io/TypeSerializers.cpp
)

target_include_directories(pbqp-core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/submodules/json/single_include
)

if(MSVC)
    target_compile_options(pbqp-core PRIVATE /W4)
else()
    target_compile_options(pbqp-core PRIVATE -Wall -Wextra)
endif()

# Graphviz
if(PBQP_USE_GVC)
    set(BUILD_SHARED_LIBS ON CACHE BOOL "Build shared libraries" FORCE)
    set(ENABLE_TCL OFF CACHE STRING "Build TCL components" FORCE)
    set(ENABLE_SWIG OFF CACHE STRING "Build language bindings" FORCE)
    set(WITH_GVEDIT OFF CACHE STRING "Build GVEdit interactive graph editor" FORCE)
    set(WITH_SMYRNA OFF CACHE STRING "Build Smyrna large graph viewer" FORCE)
    set(GRAPHVIZ_CLI OFF CACHE BOOL "Build Graphviz command line tools" FORCE)
    add_subdirectory(submodules/graphviz)
    target_link_libraries(pbqp-core PUBLIC gvc cgraph gvplugin_core gvplugin_dot_layout)
    target_compile_definitions(pbqp-core PUBLIC PBQP_USE_GVC=1)
    if(WIN32)
        target_compile_definitions(pbqp-core PUBLIC GVDLL)
    endif()
endif()

# Gurobi
if(PBQP_USE_GUROBI)
    if(NOT DEFINED ENV{GUROBI_HOME})
        message(FATAL_ERROR "GUROBI_HOME environment variable not set")
    endif()
    target_include_directories(pbqp-core PUBLIC $ENV{GUROBI_HOME}/include)
    target_link_directories(pbqp-core PUBLIC $ENV{GUROBI_HOME}/lib)
    target_link_libraries(pbqp-core PUBLIC gurobi_c++ gurobi81)
    target_compile_definitions(pbqp-core PUBLIC PBQP_USE_GUROBI=1)
endif()

enable_testing()

set(TESTS
    test/debug/GraphVisualizerTests.cpp
    test/graph/GraphTests.cpp
    test/graph/MatrixTests.cpp
    test/graph/PBQPSolutionTests.cpp
    test/math/GurobiConverterTest.cpp
    test/math/InfinityWrapperTests.cpp
    test/reduction/ConnectedComponentSeparatorTests.cpp
    test/reduction/DegreeOneReducerTests.cpp
    test/reduction/DegreeTwoReducerTests.cpp
    test/reduction/DegreeZeroReducerTests.cpp
    test/reduction/InfinityFilterTests.cpp
    test/solve/BruteForceSolverTests.cpp
    test/solve/StepByStepSolverTest.cpp
)

foreach(test_source ${TESTS})
    file(RELATIVE_PATH test_rel
        ${CMAKE_CURRENT_SOURCE_DIR}/test
        ${CMAKE_CURRENT_SOURCE_DIR}/${test_source}
    )
    string(REPLACE "/" "_" test_name ${test_rel})
    string(REPLACE ".cpp" "" test_name ${test_name})

    add_executable(${test_name} ${test_source})
    target_link_libraries(${test_name} PRIVATE pbqp-core)
    target_include_directories(${test_name} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/submodules/doctest
    )

    add_test(
        NAME ${test_name}
        COMMAND ${test_name}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    )

    if(WIN32 AND test_name STREQUAL "debug_GraphVisualizerTests")
        set_tests_properties(${test_name} PROPERTIES DISABLED TRUE)
    endif()
endforeach()
