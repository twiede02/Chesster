#pragma once

#include <cstdint>
#include <unordered_map>
#include <vector>

#include "utils.h"

template <typename T>
class History {
public:
    History() : size_(0) {}

    T* begin() { return &data_[0]; }
    const T* begin() const { return &data_[0]; }

    T* end() { return &data_[size_]; }
    const T* end() const { return &data_[size_]; }

    void add(const T& value) {
        data_[size_] = value;
        ++size_;
    }

    const T& last() { return data_[size_ - 1]; };
    void pop_last() { size_--; }

    T& operator[](std::size_t index) { return data_[index]; }
    const T& operator[](std::size_t index) const { return data_[index]; }

    std::size_t size() const { return size_; }

    void clear() { size_ = 0; }

    constexpr static int Capacity = 512;
private:
    T data_[Capacity];
    std::size_t size_;
};

struct Movelog {
    int from = -1;
    int to = -1;
    Piece promotion = Piece::Empty;
    Piece captured_piece = Piece::Empty;
    int previous_moves_since_pawnmove_or_capture = 0;
    int previous_en_passent_square = -1;
    Castling castling = Castling::None;
    bool rook_destroyed_castle = false;
    bool king_destroyed_short_castle = false;
    bool king_destroyed_long_castle = false;
};

struct Position {
    Position();
    Position(std::string fen_position);

    Position(const Position& other) = delete;

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
    Piece piece_table[64];
    Color color_table[64];

    History<Move> move_history;
    History<uint64_t> hash_history;

    int moves_since_panwmove_or_capture = 0;
    int en_passent_square = -1;

    uint64_t hash = 0ULL;

    bool is_check();

    bool position_is_legal();

    void set_piece(Piece piece, int index, Color col);

    Movelog make_move(Move& m);

    void unmake_move(Movelog& previous);
};

extern uint64_t zobrist_table[12][64];
// extern std::unordered_map<uint64_t, int> transposition_table;

void print_full_board(Position &p);
bool is_consistant(Position &p);

bool is_capture(Position &p, Move &m);
bool is_en_passent(Position &p, Move &m);
bool is_castle(Position &p, Move &m);
