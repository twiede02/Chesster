#include <algorithm>
#include <sstream>
#include <vector>

#include "magics.h"
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

    int rank = 7;
    int file = 0;

    for (char c : board) {
        if (c == '/') {
            rank--;
            file = 0;
            continue;
        }

        if (c >= '1' && c <= '8') {
            file += c - '0';
            continue;
        }

        Piece piece;
        Color color;

        if (c >= 'A' && c <= 'Z') {
            color = Color::White;
        } else {
            color = Color::Black;
        }

        switch (std::tolower(c)) {
            case 'p': piece = Piece::Pawn;   break;
            case 'n': piece = Piece::Knight; break;
            case 'b': piece = Piece::Bishop; break;
            case 'r': piece = Piece::Rook;   break;
            case 'q': piece = Piece::Queen;  break;
            case 'k': piece = Piece::King;   break;
            default: continue;
        }

        set_piece(piece, Square(file, rank), color);
        file++;
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
      white_pieces(other.white_pieces),
      black_pieces(other.black_pieces),

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
    white_pieces     = other.white_pieces;
    black_pieces     = other.black_pieces;

    piece_table = other.piece_table;
    color_table = other.color_table;

    hash_history = other.hash_history;

    moves_since_panwmove_or_capture = other.moves_since_panwmove_or_capture;
    en_passent_square = other.en_passent_square;

    hash = other.hash;

    return *this;
}

bool Position::is_check() {

    Square from = side_to_move == Color::White ?
        white_kings.msb() :
        black_kings.msb();


    Bitboard rook_blockers = Bitboard(occupied_squares)
        .masked_by(rook_attack_masks[from.value()]);

    Bitboard possible_rook_squares = lookup_rook_move(from, rook_blockers);
    possible_rook_squares.masked_by(friendly_pieces());

    while (possible_rook_squares) {
        Square to = possible_rook_squares.msb_pop();
        Piece seeing_king = piece_table[to];
        if (seeing_king == Piece::Queen || seeing_king == Piece::Rook)
            return true;
    }


    Bitboard bishop_blockers = Bitboard(occupied_squares)
        .masked_by(bishop_attack_masks[from.value()]);

    Bitboard possible_bishop_squares = lookup_bishop_move(from, bishop_blockers);
    possible_bishop_squares.masked_by(friendly_pieces());

    while (possible_bishop_squares) {
        Square to = possible_bishop_squares.msb_pop();
        Piece seeing_king = piece_table[to];
        if (seeing_king == Piece::Queen || seeing_king == Piece::Bishop)
            return true;
    }

    Bitboard possible_knight_squares = Bitboard(knight_masks[from.value()]);
    possible_knight_squares.masked_by(friendly_pieces());

    while (possible_knight_squares) {
        Square to = possible_knight_squares.msb_pop();
        Piece seeing_king = piece_table[to];
        if (seeing_king == Piece::Knight)
            return true;
    }

    return false;
}

bool Position::position_is_legal() {
    assert(count_1s(white_kings.value()) == 1);
    assert(count_1s(black_kings.value()) == 1);
    return true;
}

bool is_move_valid(Move &m, Position &p) {
    return true;
}

Bitboard Position::friendly_pieces() {
    return side_to_move == Color::White ?
        white_pieces :
        black_pieces;
}

Bitboard Position::enemy_pieces() {
    return side_to_move == Color::Black ?
        white_pieces :
        black_pieces;
}

void Position::set_piece(const Piece piece, const Square sq, const Color col) {

    // remove the previous piece from bitboards
    if (piece_table[sq] != Piece::Empty) {
        if (color_table[sq] == Color::White) {
            white_pieces.remove(sq);
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
            black_pieces.remove(sq);
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
        white_pieces.add(sq);
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
        black_pieces.add(sq);
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

