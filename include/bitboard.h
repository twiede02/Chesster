#pragma once

#include <cstdint>
#include "square.h"

class Bitboard {
public:
    Bitboard() : val_(0ULL) {}
    Bitboard(uint64_t val) : val_(val) {}
    Bitboard(const Bitboard& other) : val_(other.val_) {}

    constexpr uint64_t value() const { return val_; }

    Square msb() const { return Square( val_ ? 
            63 - __builtin_clzl(val_) : 
            static_cast<int>(Square::Value::NO_SQR)); }
    Square msb_pop() {
        Square res = msb();
        if (res)
            val_ ^= 1ULL << res.value();
        return res;
    }

    Bitboard& masked_by(const Bitboard mask) { val_ &= mask.val_; return *this; }
    Bitboard& masked_by(const Square mask) { val_ &= 1ULL << mask.value(); return *this; }

    Bitboard& blocked_by(const Bitboard mask) { val_ &= ~mask.val_; return *this; }

    Bitboard& shift_rank_up() { val_ <<= 8; return *this; }
    Bitboard& shift_rank_down() { val_ >>= 8; return *this; }
    Bitboard& shift_left() { val_ <<= 1; return *this; }
    Bitboard& shift_right() { val_ >>= 1; return *this; }

    Bitboard& remove(Square sq) { val_ ^= 1ULL << sq.value(); return *this; }
    Bitboard& add(Square sq) { val_ |= 1ULL << sq.value(); return *this; }

    bool operator==(const Bitboard& rhs) const { return val_ == rhs.val_; }
    bool operator!=(const Bitboard& rhs) const { return val_ != rhs.val_; }

    constexpr operator bool() const { return val_; }

private:
    uint64_t val_;
};


