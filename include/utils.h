#pragma once

#include <cstdint>
#include <cassert>
#include <iostream>
#include <optional>
#include <string>

#include "bitboard.h"
#include "move.h"
#include "square.h"

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

std::string inline to_string(MoveType t) {
    switch (t) {
        case MoveType::Normal:
            return "Nomal";
        case MoveType::Castling:
            return "Castling";
        case MoveType::EnPassent:
            return "EnPassent";
        case MoveType::Promotion:
            return "Promotion";
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

inline std::string to_string(Move m) {
    std::string s;

    s += m.from().to_string(); 
    s += m.to().to_string();

    if (m.type() == MoveType::Promotion) {
        switch (m.promotedPiece()) {
            case Piece::Knight: s += 'n'; break;
            case Piece::Bishop: s += 'b'; break;
            case Piece::Rook:   s += 'r'; break;
            case Piece::Queen:  s += 'q'; break;
            default: break;
        }
    }

    return s;
}


int inline get_index(char file, int rank) {
    int index = file - 'a' + 8 * (rank - 1);
    assert(index >= 0);
    assert(index < 64);
    return index;
}

void inline print_coords_from_index(int index) {
    std::cout << (char)('a' + index % 8) << (index / 8) + 1;
}

void inline print_bitboard(Bitboard b) {
    uint64_t bitboard = b.value();
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

int inline fast_log_2(uint64_t num) {
    auto t = __builtin_clzl(num | 1);
    return 63 - t;
}

void inline print_move(Move m) {
    std::cout << "\nFrom: " << m.from().to_string() << " to: " << m.to().to_string()
        << "\nPromotion: " << to_string(m.promotedPiece())
        << "\nType: " << to_string(m.type());
}

void inline print_move_compact(Move m) {
    std::cout << m.from().to_string() << m.to().to_string();
    switch (m.promotedPiece()) {
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
