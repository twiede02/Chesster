#pragma once

#include <cstdint>
#include <vector>

struct Magic {
    uint64_t mask;
    uint64_t magic;
    int shift;
    uint64_t* attacks;
};

extern std::vector<uint64_t> rook_magics;
extern std::vector<uint64_t> bishop_magics;

