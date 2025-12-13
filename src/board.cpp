#include <algorithm>
#include <sstream>
#include <vector>

#include "bitboard.h"
#include "magics.h"
#include "attack_masks.h"
#include "board.h"
#include "move.h"
#include "movegen.h"
#include "perft.h"
#include "square.h"
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
        castlingRights = CastlingRights::None;
        add_castling_right(CastlingRights::WhiteKingside);
        add_castling_right(CastlingRights::WhiteQueenside);
        add_castling_right(CastlingRights::BlackKingside);
        add_castling_right(CastlingRights::BlackQueenside);
    } else {
        for (auto c : fen_elements[2]) {
            if (c == 'K')
                add_castling_right(CastlingRights::WhiteKingside);
            if (c == 'Q')
                add_castling_right(CastlingRights::WhiteQueenside);

            if (c == 'k')
                add_castling_right(CastlingRights::BlackKingside);
            if (c == 'q')
                add_castling_right(CastlingRights::BlackQueenside);
        }
    }
}

Position::Position(const Position& other)
    : castlingRights(other.castlingRights),
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

      moves_since_pawnmove_or_capture(other.moves_since_pawnmove_or_capture),
      en_passent_square(other.en_passent_square),

      hash(other.hash)
{}


Position& Position::operator=(const Position& other)
{
    if (this == &other) {
        return *this;
    }

    castlingRights = other.castlingRights;

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

    moves_since_pawnmove_or_capture = other.moves_since_pawnmove_or_capture;
    en_passent_square = other.en_passent_square;

    hash = other.hash;

    return *this;
}

bool Position::has_castling_right(CastlingRights r) {
    return (static_cast<uint8_t>(castlingRights) & static_cast<uint8_t>(r)) != 0;
}

void Position::add_castling_right(CastlingRights r) {
    castlingRights = static_cast<CastlingRights>(
        static_cast<uint8_t>(castlingRights) | static_cast<uint8_t>(r)
    );
}

void Position::remove_castling_right(CastlingRights r) {
    castlingRights = static_cast<CastlingRights>(
        static_cast<uint8_t>(castlingRights) & ~static_cast<uint8_t>(r)
    );
}

bool Position::is_check(Color c) {
    if (c == Color::White)
        return is_square_attacked(white_kings.msb(), Color::Black);
    return is_square_attacked(black_kings.msb(), Color::White);
}

bool Position::is_square_attacked(Square sq, Color attacking_col) {

    Bitboard friendly_blockers = 
        side_to_move != attacking_col ? friendly_pieces() : enemy_pieces();


    Bitboard rook_blockers = Bitboard(occupied_squares)
        .masked_by(rook_attack_masks[sq.value()]);

    Bitboard possible_rook_squares = lookup_rook_move(sq, rook_blockers);
    possible_rook_squares.blocked_by(friendly_blockers);

    while (possible_rook_squares) {
        Square to = possible_rook_squares.msb_pop();
        Piece seeing_square = piece_table[to];
        if (seeing_square == Piece::Queen || seeing_square == Piece::Rook)
            return true;
    }


    Bitboard bishop_blockers = Bitboard(occupied_squares)
        .masked_by(bishop_attack_masks[sq.value()]);

    Bitboard possible_bishop_squares = lookup_bishop_move(sq, bishop_blockers);
    possible_bishop_squares.blocked_by(friendly_blockers);

    while (possible_bishop_squares) {
        Square to = possible_bishop_squares.msb_pop();
        Piece seeing_square = piece_table[to];
        if (seeing_square == Piece::Queen || seeing_square == Piece::Bishop)
            return true;
    }


    Bitboard possible_knight_squares = Bitboard(knight_masks[sq.value()]);
    possible_knight_squares.blocked_by(friendly_blockers);

    while (possible_knight_squares) {
        Square to = possible_knight_squares.msb_pop();
        Piece seeing_square = piece_table[to];
        if (seeing_square == Piece::Knight)
            return true;
    }


    Bitboard possible_king_squares = Bitboard(king_masks[sq.value()]);
    possible_king_squares.blocked_by(friendly_blockers);

    while (possible_king_squares) {
        Square to = possible_king_squares.msb_pop();
        Piece seeing_square = piece_table[to];
        if (seeing_square == Piece::King)
            return true;
    }


    if (attacking_col == Color::White) {
        // attacking pawn left of square
        if (sq.file() > 0 && sq.rank() > 0) {
            Square potential_attacking_pawn(sq);
            potential_attacking_pawn
                .shift_file_left()
                .shift_rank_down();
            if (piece_table[potential_attacking_pawn] == Piece::Pawn &&
                color_table[potential_attacking_pawn] == Color::White)
                return true;
        }

        // attacking pawn right of square
        if (sq.file() < 7 && sq.rank() > 0) {
            Square potential_attacking_pawn(sq);
            potential_attacking_pawn
                .shift_file_right()
                .shift_rank_down();
            if (piece_table[potential_attacking_pawn] == Piece::Pawn &&
                color_table[potential_attacking_pawn] == Color::White)
                return true;
        }
    } else {
        // attacking pawn left of square
        if (sq.file() > 0 && sq.rank() < 7) {
            Square potential_attacking_pawn(sq);
            potential_attacking_pawn
                .shift_file_left()
                .shift_rank_up();
            if (piece_table[potential_attacking_pawn] == Piece::Pawn &&
                color_table[potential_attacking_pawn] == Color::Black)
                return true;
        }

        // attacking pawn right of square
        if (sq.file() < 7 && sq.rank() < 7) {
            Square potential_attacking_pawn(sq);
            potential_attacking_pawn
                .shift_file_right()
                .shift_rank_up();
            if (piece_table[potential_attacking_pawn] == Piece::Pawn &&
                color_table[potential_attacking_pawn] == Color::Black)
                return true;

        }
    }

    return false;
}

bool Position::has_legal_move() {
    Movelist ml = generate_moves(*this);
    for (auto m : ml) {
        Movelog log = make_move(m);
        if (log) {
            unmake_move(log);
            return true;
        }
    }
    return false;
}

bool Position::is_threefold_repetition() {
    // TODO: implement
    return false;
}

GameResult Position::game_result() {
    if (is_50_move_draw())
        return GameResult::Draw50Move;

    if (is_insufficient_material())
        return GameResult::DrawInsufficientMaterial;

    if (is_threefold_repetition())
        return GameResult::DrawThreefold;

    bool hasLegalMove = has_legal_move();

    if (hasLegalMove)
        return GameResult::Ongoing;

    if (is_check(side_to_move))
        return GameResult::Checkmate;

    return GameResult::Stalemate;
}

bool Position::is_50_move_draw() {
    return moves_since_pawnmove_or_capture >= 100;
}

bool Position::is_insufficient_material() {
    auto pieces = count_1s(occupied_squares.value());

    // only kings
    if (pieces == 2) return true;
    // only bishop or knight
    if (pieces == 3) return (white_bishops.value() || 
                             white_knights.value() ||
                             black_bishops.value() ||
                             black_knights.value());
    return false;
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

void Position::update_castling_rights(Move m) {
    Piece movingPiece = piece_table[m.from()];
    Piece capturedPiece = piece_table[m.to()];
    Color movingPieceColor = color_table[m.from()];
    Color capturedPieceColor = color_table[m.to()];

    // King moves: lose both rights
    if (movingPiece == Piece::King && movingPieceColor == Color::White) {
        remove_castling_right(CastlingRights::WhiteKingside);
        remove_castling_right(CastlingRights::WhiteQueenside);
    } else if (movingPiece == Piece::King && movingPieceColor == Color::Black) {
        remove_castling_right(CastlingRights::BlackKingside);
        remove_castling_right(CastlingRights::BlackQueenside);
    }

    // Rook moves: lose corresponding side
    if (movingPiece == Piece::Rook && movingPieceColor == Color::White) {
        if (m.from() == Square::Value::H1)
            remove_castling_right(CastlingRights::WhiteKingside);
        if (m.from() == Square::Value::A1)
            remove_castling_right(CastlingRights::WhiteQueenside);
    } else if (movingPiece == Piece::Rook && movingPieceColor == Color::Black) {
        if (m.from() == Square::Value::H8)
            remove_castling_right(CastlingRights::BlackKingside);
        if (m.from() == Square::Value::A8)
            remove_castling_right(CastlingRights::BlackQueenside);
    }

    // Rook captured: lose corresponding side
    if (capturedPiece == Piece::Rook && capturedPieceColor == Color::White) {
        if (m.to() == Square::Value::H1)
            remove_castling_right(CastlingRights::WhiteKingside);
        if (m.to() == Square::Value::A1)
            remove_castling_right(CastlingRights::WhiteQueenside);
    } else if (capturedPiece == Piece::Rook && capturedPieceColor == Color::Black) {
        if (m.to() == Square::Value::H8)
            remove_castling_right(CastlingRights::BlackKingside);
        if (m.to() == Square::Value::A8)
            remove_castling_right(CastlingRights::BlackQueenside);
    }
}

void Position::do_castling_rook_move(Move m) {
    Square to = m.to();
    Color kingColor = color_table[m.from()];

    if (kingColor == Color::White) {
        if (to == Square::Value::G1) {
            set_piece(Piece::Rook, Square::Value::F1, Color::White);
            set_piece(Piece::Empty, Square::Value::H1, Color::Empty);
        } else if (to == Square::Value::C1) {
            set_piece(Piece::Rook, Square::Value::D1, Color::White);
            set_piece(Piece::Empty, Square::Value::A1, Color::Empty);
        }
    } else {
        if (to == Square::Value::G8) {
            set_piece(Piece::Rook, Square::Value::F8, Color::Black);
            set_piece(Piece::Empty, Square::Value::H8, Color::Empty);
        } else if (to == Square::Value::C8) {
            set_piece(Piece::Rook, Square::Value::D8, Color::Black);
            set_piece(Piece::Empty, Square::Value::A8, Color::Empty);
        }
    }
}


Movelog Position::make_move(Move m) {
    Movelog log;
    log.m = m;
    log.captured_piece = piece_table[m.to()]; 
    log.previousCastlingRights = castlingRights;
    log.last_moves_since_pawn_or_capture = moves_since_pawnmove_or_capture;

    if (piece_table[m.from()] == Piece::Pawn || log.captured_piece != Piece::Empty) {
        moves_since_pawnmove_or_capture = 0;
    } else {
        moves_since_pawnmove_or_capture++;
    }

    update_castling_rights(m);

    // TODO: Handle en passant target
    // if (board[m.from()] == Piece::Pawn && abs(m.to() - m.from()) == 16) {
    //     enPassantSquare = Square((m.from() + m.to()) / 2);
    // } else {
    //     enPassantSquare = Square(Square::Value::NO_SQR);
    // }

    en_passent_square = Square(Square::Value::NO_SQR);

    set_piece(piece_table[m.from()], m.to(), side_to_move);
    set_piece(Piece::Empty, m.from(), Color::Empty);

    if (m.type() == MoveType::EnPassent) {
        Square capSq = m.to();
        if (side_to_move == Color::White)
            capSq.shift_rank_down();
        else 
            capSq.shift_rank_up();
        log.captured_piece = piece_table[capSq];
        set_piece(Piece::Empty, capSq, Color::Empty);
    }

    if (m.type() == MoveType::Promotion) {
        set_piece(m.promotedPiece(), m.to(), side_to_move);
    }

    if (m.type() == MoveType::Castling) {
        do_castling_rook_move(m);
    }

    side_to_move = (side_to_move == Color::White) ? Color::Black : Color::White;

    // TODO: Zobrist update

    return log;
}

void Position::unmake_move(const Movelog& log) {
    Move m = log.m;

    side_to_move = (side_to_move == Color::White) ? Color::Black : Color::White;
    castlingRights = log.previousCastlingRights;
    moves_since_pawnmove_or_capture = log.last_moves_since_pawn_or_capture;

    // castling rook move reversal
    if (m.type() == MoveType::Castling) {
        Square kingTo = m.to();
        Color kingColor = side_to_move;
        if (kingColor == Color::White) {
            if (kingTo == Square::Value::G1) {
                set_piece(Piece::Rook, Square::Value::H1, Color::White);
                set_piece(Piece::Empty, Square::Value::F1, Color::Empty);
            } else if (kingTo == Square::Value::C1) {
                set_piece(Piece::Rook, Square::Value::A1, Color::White);
                set_piece(Piece::Empty, Square::Value::D1, Color::Empty);
            }
        } else {
            if (kingTo == Square::Value::G8) {
                set_piece(Piece::Rook, Square::Value::H8, Color::Black);
                set_piece(Piece::Empty, Square::Value::F8, Color::Empty);
            } else if (kingTo == Square::Value::C8) {
                set_piece(Piece::Rook, Square::Value::A8, Color::Black);
                set_piece(Piece::Empty, Square::Value::D8, Color::Empty);
            }
        }
    }

    if (m.type() == MoveType::EnPassent) {
        Square capSq = m.to();
        if (side_to_move == Color::White) 
            capSq.shift_rank_down();
        else 
            capSq.shift_rank_up();
        set_piece(Piece::Pawn, capSq, (side_to_move == Color::White) ? Color::Black : Color::White);
    }

    if (m.type() == MoveType::Promotion) {
        set_piece(Piece::Pawn, m.to(), side_to_move);
    }

    set_piece(piece_table[m.to()], m.from(), side_to_move);
    set_piece(log.captured_piece, m.to(), 
            side_to_move == Color::White ?
                Color::Black :
                Color::White);
}


void check_bitboards_against_tables(const Position& p) {

    // Make local copies so we can msb_pop()
    Bitboard wp = p.white_pawns;
    Bitboard wn = p.white_knights;
    Bitboard wb = p.white_bishops;
    Bitboard wr = p.white_rooks;
    Bitboard wq = p.white_queens;
    Bitboard wk = p.white_kings;

    Bitboard bp = p.black_pawns;
    Bitboard bn = p.black_knights;
    Bitboard bb = p.black_bishops;
    Bitboard br = p.black_rooks;
    Bitboard bq = p.black_queens;
    Bitboard bk = p.black_kings;

    auto check = [&](Bitboard& bb, Piece expected_piece, Color expected_color) {
        while (bb) {
            Square sq = bb.msb_pop();

            Piece pt = p.piece_table[sq];
            Color ct = p.color_table[sq];

            if (pt != expected_piece || ct != expected_color) {
                std::cout << "BITBOARD MISMATCH at square "
                          << sq.value() << " : bitboard says "
                          << (expected_color == Color::White ? "White " : "Black ")
                          << static_cast<int>(expected_piece)
                          << " but table says piece="
                          << static_cast<int>(pt)
                          << " color="
                          << static_cast<int>(ct)
                          << "\n";
            }
        }
    };

    check(wp, Piece::Pawn,   Color::White);
    check(wn, Piece::Knight, Color::White);
    check(wb, Piece::Bishop, Color::White);
    check(wr, Piece::Rook,   Color::White);
    check(wq, Piece::Queen,  Color::White);
    check(wk, Piece::King,   Color::White);

    check(bp, Piece::Pawn,   Color::Black);
    check(bn, Piece::Knight, Color::Black);
    check(bb, Piece::Bishop, Color::Black);
    check(br, Piece::Rook,   Color::Black);
    check(bq, Piece::Queen,  Color::Black);
    check(bk, Piece::King,   Color::Black);

    // At this point, every local bitboard has been consumed to zero
}

void print_position(const Position& p) {
    for (int rank = 7; rank >= 0; --rank) {
        for (int file = 0; file < 8; ++file) {
            Square s(file, rank);
            Piece pc = p.piece_table[s];
            Color c  = p.color_table[s];

            if (pc == Piece::Empty) {
                std::cout << " .";
                continue;
            }

            char ch = '?';
            switch (pc) {
                case Piece::Pawn:   ch = 'p'; break;
                case Piece::Knight: ch = 'n'; break;
                case Piece::Bishop: ch = 'b'; break;
                case Piece::Rook:   ch = 'r'; break;
                case Piece::Queen:  ch = 'q'; break;
                case Piece::King:   ch = 'k'; break;
                default: break;
            }

            if (c == Color::White)
                ch = std::toupper(ch);

            std::cout << " " << ch;
        }
        std::cout << "\n";
    }
    std::cout << "\n";

    check_bitboards_against_tables(p);
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

