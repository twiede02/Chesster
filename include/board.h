#pragma once

#include <cstdint>
#include <unordered_map>
#include <vector>

#include "utils.h"

struct Position {
    Position();

    Position(std::string fen_position);

    bool white_kingside_castling_right = false;
    bool white_queenside_castling_right = false;
    bool black_kingside_castling_right = false;
    bool black_queenside_castling_right = false;
    Color side_to_move = Color::White;

    // Bitboards
    // White pieces
    uint64_t white_pawns = 0ULL;
    uint64_t white_knights = 0ULL;
    uint64_t white_bishops = 0ULL;
    uint64_t white_rooks = 0ULL;
    uint64_t white_queens = 0ULL;
    uint64_t white_kings = 0ULL;

    // Black pieces
    uint64_t black_pawns = 0ULL;
    uint64_t black_knights = 0ULL;
    uint64_t black_bishops = 0ULL;
    uint64_t black_rooks = 0ULL;
    uint64_t black_queens = 0ULL;
    uint64_t black_kings = 0ULL;

    // Full Board Bitboards
    uint64_t empty_squares = ~0ULL;
    uint64_t occupied_squares = 0ULL;

    // 8x8 Board
    std::vector<Piece> piece_table;
    std::vector<Color> color_table;

    std::vector<Move> move_history;
    std::vector<uint64_t> hash_history;

    int moves_since_panwmove_or_capture = 0;
    int en_passent_square = -1;

    uint64_t hash = 0ULL;

    bool is_check();

    bool position_is_legal();

    void set_piece(Piece piece, int index, Color col);

    void make_move(Move m);

    void unmake_move();
};

extern uint64_t zobrist_table[12][64];
// extern std::unordered_map<uint64_t, int> transposition_table;

void print_full_board(Position &p);
bool is_consistant(Position &p);

bool is_capture(Position &p, Move &m);
bool is_en_passent(Position &p, Move &m);
bool is_castle(Position &p, Move &m);
