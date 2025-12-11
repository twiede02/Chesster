#pragma once

#include <cstdint>
#include <array>

extern const std::array<uint64_t, 64> knight_masks;
extern const std::array<uint64_t, 64> king_masks;
extern const std::array<uint64_t, 64> rook_blocker_masks;
extern const std::array<uint64_t, 64> bishop_blocker_masks;

extern const std::array<uint64_t, 64> rook_attack_masks;
extern const std::array<uint64_t, 64> bishop_attack_masks;

