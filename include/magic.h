#pragma once

#include <cstdint>
#include <vector>

struct Magic {
    uint64_t mask;
    uint64_t magic;
    int shift;
    uint64_t* attacks;
};

extern uint64_t rook_magics[64];
extern uint64_t bishop_magics[64];

