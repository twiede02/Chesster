#pragma once

#include <climits>
#include <cstdint>
#include <array>
#include <istream>

#include "bitboard.h"
#include "piece_table.h"
#include "square.h"
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

enum class CastlingRights : uint8_t {
    None           = 0,
    WhiteKingside  = 1 << 0,
    WhiteQueenside = 1 << 1,
    BlackKingside  = 1 << 2,
    BlackQueenside = 1 << 3,
};

inline std::string to_string(CastlingRights cr) {
    if (cr == CastlingRights::None) return "-";

    std::string s;

    if (static_cast<uint8_t>(cr) & static_cast<uint8_t>(CastlingRights::WhiteKingside))
        s += 'K';
    if (static_cast<uint8_t>(cr) & static_cast<uint8_t>(CastlingRights::WhiteQueenside))
        s += 'Q';
    if (static_cast<uint8_t>(cr) & static_cast<uint8_t>(CastlingRights::BlackKingside))
        s += 'k';
    if (static_cast<uint8_t>(cr) & static_cast<uint8_t>(CastlingRights::BlackQueenside))
        s += 'q';

    return s;
}

class Movelog {
public:
    Move m;
    Piece captured_piece = Piece::Empty;
    Square previous_en_passent_square = Square(Square::Value::NO_SQR);
    int last_moves_since_pawn_or_capture = 0;
    CastlingRights previousCastlingRights;

    explicit operator bool() const { return m.from() != m.to(); }
};

inline std::string to_string(const Movelog& log) {
    if (!log) return "null";

    std::string s;

    // Basic move (from -> to, with promotion if any)
    s += log.m.from().to_string();
    s += log.m.to().to_string();

    if (log.m.type() == MoveType::Promotion) {
        switch (log.m.promotedPiece()) {
            case Piece::Knight: s += 'n'; break;
            case Piece::Bishop: s += 'b'; break;
            case Piece::Rook:   s += 'r'; break;
            case Piece::Queen:  s += 'q'; break;
            default: break;
        }
    }

    // Captured piece (if any)
    if (log.captured_piece != Piece::Empty) {
        s += " x";
        s += to_string(log.captured_piece); // assumes you have to_string(Piece)
    }

    // Previous en passant square (if any)
    if (log.previous_en_passent_square) {
        s += " ep:";
        s += log.previous_en_passent_square.to_string();
    }

    // Last moves since pawn move or capture
    s += " lm:";
    s += std::to_string(log.last_moves_since_pawn_or_capture);

    // Previous castling rights
    s += " cr:";
    s += to_string(log.previousCastlingRights); // assumes you have to_string(CastlingRights)

    return s;
}

enum class GameResult {
    Ongoing,
    Checkmate,
    Stalemate,
    Draw50Move,
    DrawInsufficientMaterial,
    DrawThreefold
};

struct Position {
    Position() {};
    Position(std::string fen_position);

    Position(const Position& other);
    Position& operator=(const Position& other);

    CastlingRights castlingRights = CastlingRights::None;
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
    Bitboard empty_squares = Bitboard(ULLONG_MAX);
    Bitboard occupied_squares;
    Bitboard white_pieces;
    Bitboard black_pieces;

    // 8x8 Board
    PieceTable<Piece> piece_table;
    PieceTable<Color> color_table;
    History<uint64_t> hash_history;

    int moves_since_pawnmove_or_capture = 0;
    Square en_passent_square;

    uint64_t hash = 0ULL;

    Bitboard friendly_pieces();
    Bitboard enemy_pieces();

    bool is_square_attacked(const Square sq, Color by);

    bool has_castling_right(CastlingRights r);
    void add_castling_right(CastlingRights r);
    void remove_castling_right(CastlingRights r);

    bool is_check(Color c);
    bool is_game_over();
    bool is_insufficient_material();
    bool is_50_move_draw();
    bool has_legal_move();
    bool is_threefold_repetition();

    GameResult game_result();

    void set_piece(const Piece piece, const Square sq, const Color col);

    void do_castling_rook_move(Move m);
    void update_castling_rights(Move m);

    Movelog make_move(Move m);
    void unmake_move(const Movelog& previous);
};

extern uint64_t zobrist_table[12][64];
// extern std::unordered_map<uint64_t, int> transposition_table;

bool is_capture(Position &p, Move &m);
bool is_en_passent(Position &p, Move &m);

void print_position(const Position& p);
void validate_position(const Position& p);

