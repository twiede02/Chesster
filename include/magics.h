#pragma once

#include <cstdint>
#include <array>
#include "bitboard.h"
#include "square.h"

extern std::array<uint64_t, 64> rook_magics;
extern std::array<uint64_t, 64> bishop_magics;

extern std::array<uint64_t[4096], 64> rook_moves_precomp;
extern std::array<uint64_t[4096], 64> bishop_moves_precomp;

void generate_magic_tables();
Bitboard lookup_rook_move(Square s, Bitboard b);
Bitboard lookup_bishop_move(Square s, Bitboard b);
