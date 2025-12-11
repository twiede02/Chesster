#pragma once

#include "square.h"
#include <cstdint>

enum class Piece { Empty = 0, Pawn, Rook, Knight, Bishop, Queen, King };
enum class Color { Empty = 0, White, Black };
enum class Castling { WhiteShort, WhiteLong, BlackShort, BlackLong, None };
enum class MoveType { Normal, Castling, EnPassent, Promotion };

// layout left to right
// 2 bits: move type
// 00: normal
// 01: castle
// 10: en passent
// 11: promotion
// 2 bits: promotion piece
// 00: knight
// 01: bishop
// 10: rook
// 11: queen
// 6 bits: from square
// 6 bits: to square
class Move {
   public:
    Move() : move_(0) {}
    Move(Square from, Square to, MoveType m = MoveType::Normal, Piece p = Piece::Knight) {
        std::uint16_t leftmost4bits = static_cast<uint16_t>(static_cast<int>(m) << 2 | static_cast<int>(p));
        move_ = static_cast<uint16_t>((leftmost4bits << 12) | ((from.value() & 0x3F) << 6) | (to.value() & 0x3F));
    }

    Square from() const { return Square((move_ >> 6) & 0x3F); }
    Square to() const { return Square(move_ & 0x3F); }
    constexpr MoveType type() const { return static_cast<MoveType>((move_ >> 14) & 0x3); }
    constexpr Piece promotedPiece() const { return static_cast<Piece>((move_ >> 12) & 0x3); }

   private:
    std::uint16_t move_;
};

