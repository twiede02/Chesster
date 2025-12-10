#pragma once

#include "square.h"

enum class Piece { Pawn, Rook, Knight, Bishop, Queen, King, Empty };
enum class Color { White, Black, Empty };
enum class Castling { WhiteShort, WhiteLong, BlackShort, BlackLong, None };


struct Move {
    Move(int f, int t, Piece p = Piece::Empty)
        : from(f), to(t), promotion(p), captured_piece(Piece::Empty),
        previous_moves_since_pawnmove_or_capture(0),
        previous_en_passent_square(-1), castling(Castling::None),
        rook_destroyed_castle(false), king_destroyed_short_castle(false),
        king_destroyed_long_castle(false) {}
    Move() {}
    Move(Square f, Square t, Piece p = Piece::Empty)
        : from(f.value()), to(t.value()), promotion(p), captured_piece(Piece::Empty),
        previous_moves_since_pawnmove_or_capture(0),
        previous_en_passent_square(-1), castling(Castling::None),
        rook_destroyed_castle(false), king_destroyed_short_castle(false),
        king_destroyed_long_castle(false) {}

    int from;
    int to;
    Piece promotion;
    Piece captured_piece;
    int previous_moves_since_pawnmove_or_capture;
    int previous_en_passent_square;
    Castling castling;
    bool rook_destroyed_castle;
    bool king_destroyed_short_castle;
    bool king_destroyed_long_castle;
};


