cmake_minimum_required(VERSION 3.16)

project(Chesster VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_compile_options(-Wall -Wextra -Wpedantic -Wshadow -Wconversion)
option(ENABLE_SANITIZERS "Enable runtime sanitizers for debugging" ON)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type (Debug or Release)" FORCE)
endif()

# Include directories
include_directories(include)

# Add source files
file(GLOB SOURCES CONFIGURE_DEPENDS "src/*.cpp")

# Add executable target
add_executable(chesster ${SOURCES})

if (ENABLE_SANITIZERS AND CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(chesster PRIVATE -fsanitize=address,undefined)
    target_link_options(chesster PRIVATE -fsanitize=address,undefined)
endif()

if ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
    message(STATUS "Configuring Debug build with sanitizers")

    # Debug flags
    target_compile_options(chesster PRIVATE -O0 -g3 -fno-omit-frame-pointer)
    target_link_options(chesster PRIVATE -O0 -g3 -fno-omit-frame-pointer)

    if (ENABLE_SANITIZERS)
        message(STATUS "Sanitizers enabled")
        target_compile_options(chesster PRIVATE -fsanitize=address,undefined)
        target_link_options(chesster PRIVATE -fsanitize=address,undefined)
    endif()

elseif (CMAKE_BUILD_TYPE STREQUAL "Release")
    message(STATUS "Configuring Release build")

    target_compile_options(chesster PRIVATE -O3 -DNDEBUG)
    target_link_options(chesster PRIVATE -O3)

endif()


# Enable to compile ctests for more detailed perft
if (false)
    # Add the test executable (assuming you have a test file)
    add_executable(perft_pos1_d1 src/board.cpp src/perft.cpp src/movegen.cpp src/attack_masks.cpp tests/perft_pos1_d1.cpp)
    add_executable(perft_pos1_d2 src/board.cpp src/perft.cpp src/movegen.cpp src/attack_masks.cpp tests/perft_pos1_d2.cpp)
    add_executable(perft_pos1_d3 src/board.cpp src/perft.cpp src/movegen.cpp src/attack_masks.cpp tests/perft_pos1_d3.cpp)
    add_executable(perft_pos1_d4 src/board.cpp src/perft.cpp src/movegen.cpp src/attack_masks.cpp tests/perft_pos1_d4.cpp)
    add_executable(perft_pos1_d5 src/board.cpp src/perft.cpp src/movegen.cpp src/attack_masks.cpp tests/perft_pos1_d5.cpp)

    add_executable(qperft_pos1_d5 src/board.cpp src/perft.cpp src/movegen.cpp src/attack_masks.cpp tests/qperft_pos1_d5.cpp)
    add_executable(qperft_pos1_d6 src/board.cpp src/perft.cpp src/movegen.cpp src/attack_masks.cpp tests/qperft_pos1_d6.cpp)
    add_executable(qperft_pos1_d7 src/board.cpp src/perft.cpp src/movegen.cpp src/attack_masks.cpp tests/qperft_pos1_d7.cpp)

    add_executable(perft_pos2_d1 src/board.cpp src/perft.cpp src/movegen.cpp src/attack_masks.cpp tests/perft_pos2_d1.cpp)
    add_executable(perft_pos2_d2 src/board.cpp src/perft.cpp src/movegen.cpp src/attack_masks.cpp tests/perft_pos2_d2.cpp)
    add_executable(perft_pos2_d3 src/board.cpp src/perft.cpp src/movegen.cpp src/attack_masks.cpp tests/perft_pos2_d3.cpp)
    add_executable(perft_pos2_d4 src/board.cpp src/perft.cpp src/movegen.cpp src/attack_masks.cpp tests/perft_pos2_d4.cpp)

    add_executable(qperft_pos2_d5 src/board.cpp src/perft.cpp src/movegen.cpp src/attack_masks.cpp tests/qperft_pos2_d5.cpp)

    add_executable(perft_pos3_d1 src/board.cpp src/perft.cpp src/movegen.cpp src/attack_masks.cpp tests/perft_pos3_d1.cpp)
    add_executable(perft_pos3_d2 src/board.cpp src/perft.cpp src/movegen.cpp src/attack_masks.cpp tests/perft_pos3_d2.cpp)
    add_executable(perft_pos3_d3 src/board.cpp src/perft.cpp src/movegen.cpp src/attack_masks.cpp tests/perft_pos3_d3.cpp)
    add_executable(perft_pos3_d4 src/board.cpp src/perft.cpp src/movegen.cpp src/attack_masks.cpp tests/perft_pos3_d4.cpp)
    add_executable(perft_pos3_d5 src/board.cpp src/perft.cpp src/movegen.cpp src/attack_masks.cpp tests/perft_pos3_d5.cpp)

    add_executable(perft_pos4_d1 src/board.cpp src/perft.cpp src/movegen.cpp src/attack_masks.cpp tests/perft_pos4_d1.cpp)
    add_executable(perft_pos4_d2 src/board.cpp src/perft.cpp src/movegen.cpp src/attack_masks.cpp tests/perft_pos4_d2.cpp)
    add_executable(perft_pos4_d3 src/board.cpp src/perft.cpp src/movegen.cpp src/attack_masks.cpp tests/perft_pos4_d3.cpp)
    add_executable(perft_pos4_d4 src/board.cpp src/perft.cpp src/movegen.cpp src/attack_masks.cpp tests/perft_pos4_d4.cpp)
    add_executable(perft_pos4_d5 src/board.cpp src/perft.cpp src/movegen.cpp src/attack_masks.cpp tests/perft_pos4_d5.cpp)

    add_executable(qperft_pos5_d1 src/board.cpp src/perft.cpp src/movegen.cpp src/attack_masks.cpp tests/qperft_pos5_d1.cpp)
    add_executable(qperft_pos5_d2 src/board.cpp src/perft.cpp src/movegen.cpp src/attack_masks.cpp tests/qperft_pos5_d2.cpp)
    add_executable(qperft_pos5_d3 src/board.cpp src/perft.cpp src/movegen.cpp src/attack_masks.cpp tests/qperft_pos5_d3.cpp)
    add_executable(qperft_pos5_d4 src/board.cpp src/perft.cpp src/movegen.cpp src/attack_masks.cpp tests/qperft_pos5_d4.cpp)
    add_executable(qperft_pos5_d5 src/board.cpp src/perft.cpp src/movegen.cpp src/attack_masks.cpp tests/qperft_pos5_d5.cpp)

    add_executable(qperft_pos6_d1 src/board.cpp src/perft.cpp src/movegen.cpp src/attack_masks.cpp tests/qperft_pos6_d1.cpp)
    add_executable(qperft_pos6_d2 src/board.cpp src/perft.cpp src/movegen.cpp src/attack_masks.cpp tests/qperft_pos6_d2.cpp)
    add_executable(qperft_pos6_d3 src/board.cpp src/perft.cpp src/movegen.cpp src/attack_masks.cpp tests/qperft_pos6_d3.cpp)
    add_executable(qperft_pos6_d4 src/board.cpp src/perft.cpp src/movegen.cpp src/attack_masks.cpp tests/qperft_pos6_d4.cpp)
    add_executable(qperft_pos6_d5 src/board.cpp src/perft.cpp src/movegen.cpp src/attack_masks.cpp tests/qperft_pos6_d5.cpp)


    # Enable testing with CTest
    enable_testing()

    # Register the test with CTest
    add_test(NAME perft_pos1_d1 COMMAND perft_pos1_d1)
    add_test(NAME perft_pos1_d2 COMMAND perft_pos1_d2)
    add_test(NAME perft_pos1_d3 COMMAND perft_pos1_d3)
    add_test(NAME perft_pos1_d4 COMMAND perft_pos1_d4)
    add_test(NAME perft_pos1_d5 COMMAND perft_pos1_d5)

    add_test(NAME qperft_pos1_d5 COMMAND qperft_pos1_d5)
    add_test(NAME qperft_pos1_d6 COMMAND qperft_pos1_d6)
    add_test(NAME qperft_pos1_d7 COMMAND qperft_pos1_d7)

    add_test(NAME perft_pos2_d1 COMMAND perft_pos2_d1)
    add_test(NAME perft_pos2_d2 COMMAND perft_pos2_d2)
    add_test(NAME perft_pos2_d3 COMMAND perft_pos2_d3)
    add_test(NAME perft_pos2_d4 COMMAND perft_pos2_d4)

    add_test(NAME qperft_pos2_d5 COMMAND qperft_pos2_d5)

    add_test(NAME perft_pos3_d1 COMMAND perft_pos3_d1)
    add_test(NAME perft_pos3_d2 COMMAND perft_pos3_d2)
    add_test(NAME perft_pos3_d3 COMMAND perft_pos3_d3)
    add_test(NAME perft_pos3_d4 COMMAND perft_pos3_d4)
    add_test(NAME perft_pos3_d5 COMMAND perft_pos3_d5)

    add_test(NAME perft_pos4_d1 COMMAND perft_pos4_d1)
    add_test(NAME perft_pos4_d2 COMMAND perft_pos4_d2)
    add_test(NAME perft_pos4_d3 COMMAND perft_pos4_d3)
    add_test(NAME perft_pos4_d4 COMMAND perft_pos4_d4)
    add_test(NAME perft_pos4_d5 COMMAND perft_pos4_d5)

    add_test(NAME qperft_pos5_d1 COMMAND qperft_pos5_d1)
    add_test(NAME qperft_pos5_d2 COMMAND qperft_pos5_d2)
    add_test(NAME qperft_pos5_d3 COMMAND qperft_pos5_d3)
    add_test(NAME qperft_pos5_d4 COMMAND qperft_pos5_d4)
    add_test(NAME qperft_pos5_d5 COMMAND qperft_pos5_d5)

    add_test(NAME qperft_pos6_d1 COMMAND qperft_pos6_d1)
    add_test(NAME qperft_pos6_d2 COMMAND qperft_pos6_d2)
    add_test(NAME qperft_pos6_d3 COMMAND qperft_pos6_d3)
    add_test(NAME qperft_pos6_d4 COMMAND qperft_pos6_d4)
    add_test(NAME qperft_pos6_d5 COMMAND qperft_pos6_d5)
endif()
