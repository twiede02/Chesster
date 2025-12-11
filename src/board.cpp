#include <algorithm>
#include <sstream>
#include <vector>

#include "attack_masks.h"
#include "board.h"
#include "move.h"
#include "perft.h"
#include "utils.h"

// std::unordered_map<uint64_t, int> transposition_table(1 << 20);

Position::Position(std::string fen_position) {

    std::stringstream fen_stream(fen_position);

    std::string token;
    std::vector<std::string> fen_elements;
    char delimiter = ' ';

    while (getline(fen_stream, token, delimiter)) {
        fen_elements.push_back(token);
    }

    std::string board = fen_elements[0];

    char file = 'a';
    int rank = 8;
    for (char c : board) {
        if (c == '/') {
            file = 'a';
            rank--;
            continue;
        }

        if (c >= '1' && c <= '8') {
            file = (char) (file + c - (int) '0');
            continue;
        }

        // Black Pieces
        if (c == 'r') {
            this->set_piece(Piece::Rook, Square(file, rank), Color::Black);
            file++;
            continue;
        }
        if (c == 'n') {
            this->set_piece(Piece::Knight, Square(file, rank), Color::Black);
            file++;
            continue;
        }
        if (c == 'b') {
            this->set_piece(Piece::Bishop, Square(file, rank), Color::Black);
            file++;
            continue;
        }
        if (c == 'q') {
            this->set_piece(Piece::Queen, Square(file, rank), Color::Black);
            file++;
            continue;
        }
        if (c == 'k') {
            this->set_piece(Piece::King, Square(file, rank), Color::Black);
            file++;
            continue;
        }
        if (c == 'p') {
            this->set_piece(Piece::Pawn, Square(file, rank), Color::Black);
            file++;
            continue;
        }

        // White Pieces
        if (c == 'R') {
            this->set_piece(Piece::Rook, Square(file, rank), Color::White);
            file++;
            continue;
        }
        if (c == 'N') {
            this->set_piece(Piece::Knight, Square(file, rank), Color::White);
            file++;
            continue;
        }
        if (c == 'B') {
            this->set_piece(Piece::Bishop, Square(file, rank), Color::White);
            file++;
            continue;
        }
        if (c == 'Q') {
            this->set_piece(Piece::Queen, Square(file, rank), Color::White);
            file++;
            continue;
        }
        if (c == 'K') {
            this->set_piece(Piece::King, Square(file, rank), Color::White);
            file++;
            continue;
        }
        if (c == 'P') {
            this->set_piece(Piece::Pawn, Square(file, rank), Color::White);
            file++;
            continue;
        }
    }

    if (fen_elements[1] == "w") {
        side_to_move = Color::White;
    } else if (fen_elements[1] == "b") {
        side_to_move = Color::Black;
    } else {
        std::cout << "FEN error, invalid side to move\n";
        exit(1);
    }

    if (fen_elements[2].size() == 4) {
        white_kingside_castling_right = true;
        white_queenside_castling_right = true;
        black_kingside_castling_right = true;
        black_queenside_castling_right = true;
    } else {
        for (auto c : fen_elements[2]) {
            if (c == 'K')
                white_kingside_castling_right = true;
            if (c == 'Q')
                white_queenside_castling_right = true;

            if (c == 'k')
                black_kingside_castling_right = true;
            if (c == 'q')
                black_queenside_castling_right = true;
        }
    }
}

Position::Position(const Position& other)
    : white_kingside_castling_right(other.white_kingside_castling_right),
      white_queenside_castling_right(other.white_queenside_castling_right),
      black_kingside_castling_right(other.black_kingside_castling_right),
      black_queenside_castling_right(other.black_queenside_castling_right),
      side_to_move(other.side_to_move),

      white_pawns(other.white_pawns),
      white_knights(other.white_knights),
      white_bishops(other.white_bishops),
      white_rooks(other.white_rooks),
      white_queens(other.white_queens),
      white_kings(other.white_kings),

      black_pawns(other.black_pawns),
      black_knights(other.black_knights),
      black_bishops(other.black_bishops),
      black_rooks(other.black_rooks),
      black_queens(other.black_queens),
      black_kings(other.black_kings),

      empty_squares(other.empty_squares),
      occupied_squares(other.occupied_squares),
      enemy_pieces(other.enemy_pieces),

      piece_table(other.piece_table),
      color_table(other.color_table),
      hash_history(other.hash_history),

      moves_since_panwmove_or_capture(other.moves_since_panwmove_or_capture),
      en_passent_square(other.en_passent_square),

      hash(other.hash)
{}


Position& Position::operator=(const Position& other)
{
    if (this == &other) {
        return *this;
    }

    white_kingside_castling_right  = other.white_kingside_castling_right;
    white_queenside_castling_right = other.white_queenside_castling_right;
    black_kingside_castling_right  = other.black_kingside_castling_right;
    black_queenside_castling_right = other.black_queenside_castling_right;

    side_to_move = other.side_to_move;

    white_pawns   = other.white_pawns;
    white_knights = other.white_knights;
    white_bishops = other.white_bishops;
    white_rooks   = other.white_rooks;
    white_queens  = other.white_queens;
    white_kings   = other.white_kings;

    black_pawns   = other.black_pawns;
    black_knights = other.black_knights;
    black_bishops = other.black_bishops;
    black_rooks   = other.black_rooks;
    black_queens  = other.black_queens;
    black_kings   = other.black_kings;

    empty_squares    = other.empty_squares;
    occupied_squares = other.occupied_squares;
    enemy_pieces     = other.enemy_pieces;

    piece_table = other.piece_table;
    color_table = other.color_table;

    hash_history = other.hash_history;

    moves_since_panwmove_or_capture = other.moves_since_panwmove_or_capture;
    en_passent_square = other.en_passent_square;

    hash = other.hash;

    return *this;
}



bool Position::is_check() {
    if (side_to_move == Color::White) {
        side_to_move = Color::Black;
    } else {
        side_to_move = Color::White;
    }

    bool res = position_is_legal() == false;

    if (side_to_move == Color::White) {
        side_to_move = Color::Black;
    } else {
        side_to_move = Color::White;
    }

    return res;
}

bool Position::position_is_legal() {
    return true;
}

bool is_move_valid(Move &m, Position &p) {
    return true;
}

void Position::set_piece(Piece piece, Square sq, Color col) {

    // remove the previous piece from bitboards
    if (piece_table[sq] != Piece::Empty) {
        if (color_table[sq] == Color::White) {
            switch (piece_table[sq]) {
                case Piece::Pawn:
                    white_pawns.remove(sq);
                    empty_squares.add(sq);
                    occupied_squares.remove(sq);
                    hash ^= zobrist_table[0][sq.value()];
                    break;
                case Piece::Rook:
                    white_rooks.remove(sq);
                    empty_squares.add(sq);
                    occupied_squares.remove(sq);
                    hash ^= zobrist_table[1][sq.value()];
                    break;
                case Piece::Knight:
                    white_knights.remove(sq);
                    empty_squares.add(sq);
                    occupied_squares.remove(sq);
                    hash ^= zobrist_table[2][sq.value()];
                    break;
                case Piece::Bishop:
                    white_bishops.remove(sq);
                    empty_squares.add(sq);
                    occupied_squares.remove(sq);
                    hash ^= zobrist_table[3][sq.value()];
                    break;
                case Piece::Queen:
                    white_queens.remove(sq);
                    empty_squares.add(sq);
                    occupied_squares.remove(sq);
                    hash ^= zobrist_table[4][sq.value()];
                    break;
                case Piece::King:
                    white_kings.remove(sq);
                    empty_squares.add(sq);
                    occupied_squares.remove(sq);
                    hash ^= zobrist_table[5][sq.value()];
                    break;
                case Piece::Empty:
                    break;
            }
        } else {
            switch (piece_table[sq]) {
                case Piece::Pawn:
                    black_pawns.remove(sq);
                    empty_squares.add(sq);
                    occupied_squares.remove(sq);
                    hash ^= zobrist_table[6][sq.value()];
                    break;
                case Piece::Rook:
                    black_rooks.remove(sq);
                    empty_squares.add(sq);
                    occupied_squares.remove(sq);
                    hash ^= zobrist_table[7][sq.value()];
                    break;
                case Piece::Knight:
                    black_knights.remove(sq);
                    empty_squares.add(sq);
                    occupied_squares.remove(sq);
                    hash ^= zobrist_table[8][sq.value()];
                    break;
                case Piece::Bishop:
                    black_bishops.remove(sq);
                    empty_squares.add(sq);
                    occupied_squares.remove(sq);
                    hash ^= zobrist_table[9][sq.value()];
                    break;
                case Piece::Queen:
                    black_queens.remove(sq);
                    empty_squares.add(sq);
                    occupied_squares.remove(sq);
                    hash ^= zobrist_table[10][sq.value()];
                    break;
                case Piece::King:
                    black_kings.remove(sq);
                    empty_squares.add(sq);
                    occupied_squares.remove(sq);
                    hash ^= zobrist_table[11][sq.value()];
                    break;
                case Piece::Empty:
                    break;
            }
        }
    }

    piece_table[sq] = piece;
    color_table[sq] = col;

    // innserting in Bitboards
    if (col == Color::White) {
        switch (piece) {
            case Piece::Pawn:
                white_pawns.add(sq);
                empty_squares.remove(sq);
                occupied_squares.add(sq);
                hash ^= zobrist_table[0][sq.value()];
                break;
            case Piece::Rook:
                white_rooks.add(sq);
                empty_squares.remove(sq);
                occupied_squares.add(sq);
                hash ^= zobrist_table[1][sq.value()];
                break;
            case Piece::Knight:
                white_knights.add(sq);
                empty_squares.remove(sq);
                occupied_squares.add(sq);
                hash ^= zobrist_table[2][sq.value()];
                break;
            case Piece::Bishop:
                white_bishops.add(sq);
                empty_squares.remove(sq);
                occupied_squares.add(sq);
                hash ^= zobrist_table[3][sq.value()];
                break;
            case Piece::Queen:
                white_queens.add(sq);
                empty_squares.remove(sq);
                occupied_squares.add(sq);
                hash ^= zobrist_table[4][sq.value()];
                break;
            case Piece::King:
                white_kings.add(sq);
                empty_squares.remove(sq);
                occupied_squares.add(sq);
                hash ^= zobrist_table[5][sq.value()];
                break;
            case Piece::Empty:
                break;
        }
    } else {
        switch (piece) {
            case Piece::Pawn:
                black_pawns.add(sq);
                empty_squares.remove(sq);
                occupied_squares.add(sq);
                hash ^= zobrist_table[6][sq.value()];
                break;
            case Piece::Rook:
                black_rooks.add(sq);
                empty_squares.remove(sq);
                occupied_squares.add(sq);
                hash ^= zobrist_table[7][sq.value()];
                break;
            case Piece::Knight:
                black_knights.add(sq);
                empty_squares.remove(sq);
                occupied_squares.add(sq);
                hash ^= zobrist_table[8][sq.value()];
                break;
            case Piece::Bishop:
                black_bishops.add(sq);
                empty_squares.remove(sq);
                occupied_squares.add(sq);
                hash ^= zobrist_table[9][sq.value()];
                break;
            case Piece::Queen:
                black_queens.add(sq);
                empty_squares.remove(sq);
                occupied_squares.add(sq);
                hash ^= zobrist_table[10][sq.value()];
                break;
            case Piece::King:
                black_kings.add(sq);
                empty_squares.remove(sq);
                occupied_squares.add(sq);
                hash ^= zobrist_table[11][sq.value()];
                break;
            case Piece::Empty:
                break;
        }
    }
}

Movelog Position::make_move(Move& m) {
    Movelog movelog;
    return movelog;
}

void Position::unmake_move(Movelog& previous) {
}

void print_all_bitboards(Position &p) {

    std::cout << "white pawns\n";
    print_bitboard(p.white_pawns);
    std::cout << "white knights\n";
    print_bitboard(p.white_knights);
    std::cout << "white bishops\n";
    print_bitboard(p.white_bishops);
    std::cout << "white rooks\n";
    print_bitboard(p.white_rooks);
    std::cout << "white queen\n";
    print_bitboard(p.white_queens);
    std::cout << "white king\n";
    print_bitboard(p.white_kings);

    std::cout << "black pawns\n";
    print_bitboard(p.black_pawns);
    std::cout << "black knights\n";
    print_bitboard(p.black_knights);
    std::cout << "black bishops\n";
    print_bitboard(p.black_bishops);
    std::cout << "black rooks\n";
    print_bitboard(p.black_rooks);
    std::cout << "black queen\n";
    print_bitboard(p.black_queens);
    std::cout << "black king\n";
    print_bitboard(p.black_kings);
}

void go_through_all_knight_masks() {
    for (int i = 0; i < 64; i++) {
        uint64_t index = 1ULL << i;
        print_bitboard(index);
        std::cout << "\n";
        print_bitboard(knight_masks[i]);
        std::cout << "\npress to confirm\n";
        std::string s;
        std::cin >> s;
    }
}

void go_through_all_king_masks() {
    for (int i = 0; i < 64; i++) {
        uint64_t index = 1ULL << i;
        print_bitboard(index);
        std::cout << "\n" << i << "\n";
        print_bitboard(king_masks[i]);
        std::cout << "\npress to confirm\n";
        std::string s;
        std::cin >> s;
    }
}

bool is_capture(Position &p, Move &m) {
    if (p.piece_table[m.to()] != Piece::Empty)
        return true;
    return is_en_passent(p, m);
}

bool is_en_passent(Position &p, Move &m) {
    return (p.piece_table[m.from()] == Piece::Pawn) 
        && p.en_passent_square == m.to();
}

