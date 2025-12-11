#pragma once

#include <cstdint>
#include <array>

#include "bitboard.h"
#include "piece_table.h"
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
    Position() {};
    Position(std::string fen_position);

    Position(const Position& other);
    Position& operator=(const Position& other);

    bool white_kingside_castling_right = false;
    bool white_queenside_castling_right = false;
    bool black_kingside_castling_right = false;
    bool black_queenside_castling_right = false;
    Color side_to_move = Color::White;

    // Bitboards
    // White pieces
    Bitboard white_pawns;
    Bitboard white_knights;
    Bitboard white_bishops;
    Bitboard white_rooks;
    Bitboard white_queens;
    Bitboard white_kings;

    // Black pieces
    Bitboard black_pawns;
    Bitboard black_knights;
    Bitboard black_bishops;
    Bitboard black_rooks;
    Bitboard black_queens;
    Bitboard black_kings;

    // Full Board Bitboards
    Bitboard empty_squares;
    Bitboard occupied_squares;
    Bitboard enemy_pieces;

    // 8x8 Board
    PieceTable<Piece> piece_table;
    PieceTable<Color> color_table;
    History<uint64_t> hash_history;

    int moves_since_panwmove_or_capture = 0;
    Square en_passent_square;

    uint64_t hash = 0ULL;

    bool is_check();

    bool position_is_legal();

    void set_piece(const Piece piece, const Square sq, const Color col);

    Movelog make_move(Move& m);

    void unmake_move(Movelog& previous);
};

extern uint64_t zobrist_table[12][64];
// extern std::unordered_map<uint64_t, int> transposition_table;

bool is_capture(Position &p, Move &m);
bool is_en_passent(Position &p, Move &m);

bool is_move_valid(Move &m, Position &p);
