#pragma once

#include "random.h"

inline uint64_t zobrist_table[12][64];
inline void initialize_zobrist() {
    std::uniform_int_distribution<uint64_t> uniform_dist;

    // Populate the Zobrist table with random 64-bit values
    for (int piece = 0; piece < 12; ++piece) {
        for (int square = 0; square < 64; ++square) {
            zobrist_table[piece][square] = uniform_dist(gen);
        }
    }
}
