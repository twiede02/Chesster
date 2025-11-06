#pragma once

#include <cassert>
#include <iostream>
#include <optional>
#include <string>

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

struct TimeControl {
    std::optional<int> wtime;
    std::optional<int> btime;
    std::optional<int> movestogo;
    std::optional<int> movetime;
};

std::string inline to_string(Piece p) {
    switch (p) {
        case Piece::Pawn:
            return "Pawn";
        case Piece::Rook:
            return "Rook";
        case Piece::Knight:
            return "Knight";
        case Piece::Bishop:
            return "Bishop";
        case Piece::Queen:
            return "Queen";
        case Piece::King:
            return "King";
        case Piece::Empty:
            return "Empty";
        default:
            return "PIECE NOT FOUND 1";
    }
}

std::string inline to_string(Color c) {
    switch (c) {
        case Color::White:
            return "White";
        case Color::Black:
            return "Black";
        case Color::Empty:
            return "Empty";
        default:
            return "Color NOT FOUND";
    }
}

std::string inline to_string(Move m) {
    return std::to_string(m.from) + std::to_string(m.to);
}

int inline get_index(char file, int rank) {
    int index = file - 'a' + 8 * (rank - 1);
    assert(index >= 0);
    assert(index < 64);
    return index;
}

std::string inline get_coords_from_index(int index) {
    if (index < 0 || index >= 64) {
        // Handle invalid index
        return "Invalid index";
    }

    // Calculate file (column) and rank (row)
    char file = 'a' + (index % 8); // File 'a' to 'h'
    int rank = (index / 8) + 1;    // Rank 1 to 8

    // Concatenate file and rank into a string
    return std::string(1, file) + std::to_string(rank);
}

void inline print_coords_from_index(int index) {
    std::cout << (char)('a' + index % 8) << (index / 8) + 1;
}

void inline print_bitboard(uint64_t bitboard) {
    for (int rank = 7; rank >= 0; --rank) {
        for (int file = 0; file < 8; ++file) {
            int square = rank * 8 + file;
            if (bitboard & (1ULL << square)) {
                std::cout << "1 ";
            } else {
                std::cout << ". ";
            }
        }
        std::cout << std::endl;
    }
}

union long_double {
    double x;
    long y;
};

int inline fast_log_2(double num) {
    union long_double x = {num};
    return ((x.y >> 52) + 1) & 0x3ff;
}

void inline print_move(Move m) {
    std::cout << "\nFrom: " << m.from << " to: " << m.to
        << "\nPromotion: " << to_string(m.promotion)
        << "\nCaptured: " << to_string(m.captured_piece);
}

void inline print_move_compact(Move m) {
    std::cout << get_coords_from_index(m.from) << get_coords_from_index(m.to);
    switch (m.promotion) {
        case Piece::Rook:
            std::cout << "r";
            break;
        case Piece::Knight:
            std::cout << "n";
            break;
        case Piece::Bishop:
            std::cout << "b";
            break;
        case Piece::Queen:
            std::cout << "q";
            break;
        default:
            break;
    }
}
