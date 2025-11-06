#include <bits/stdc++.h>
#include <sstream>

#include "attack_masks.h"
#include "board.h"
#include "perft.h"
#include "utils.h"

// std::unordered_map<uint64_t, int> transposition_table(1 << 20);

Position::Position() {
    piece_table.assign(64, Piece::Empty);
    color_table.assign(64, Color::Empty);
}

Position::Position(std::string fen_position) {
    piece_table.assign(64, Piece::Empty);
    color_table.assign(64, Color::Empty);

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
            this->set_piece(Piece::Rook, get_index(file, rank), Color::Black);
            file++;
            continue;
        }
        if (c == 'n') {
            this->set_piece(Piece::Knight, get_index(file, rank), Color::Black);
            file++;
            continue;
        }
        if (c == 'b') {
            this->set_piece(Piece::Bishop, get_index(file, rank), Color::Black);
            file++;
            continue;
        }
        if (c == 'q') {
            this->set_piece(Piece::Queen, get_index(file, rank), Color::Black);
            file++;
            continue;
        }
        if (c == 'k') {
            this->set_piece(Piece::King, get_index(file, rank), Color::Black);
            file++;
            continue;
        }
        if (c == 'p') {
            this->set_piece(Piece::Pawn, get_index(file, rank), Color::Black);
            file++;
            continue;
        }

        // White Pieces
        if (c == 'R') {
            this->set_piece(Piece::Rook, get_index(file, rank), Color::White);
            file++;
            continue;
        }
        if (c == 'N') {
            this->set_piece(Piece::Knight, get_index(file, rank), Color::White);
            file++;
            continue;
        }
        if (c == 'B') {
            this->set_piece(Piece::Bishop, get_index(file, rank), Color::White);
            file++;
            continue;
        }
        if (c == 'Q') {
            this->set_piece(Piece::Queen, get_index(file, rank), Color::White);
            file++;
            continue;
        }
        if (c == 'K') {
            this->set_piece(Piece::King, get_index(file, rank), Color::White);
            file++;
            continue;
        }
        if (c == 'P') {
            this->set_piece(Piece::Pawn, get_index(file, rank), Color::White);
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
    // Test if side_to_move could capture the king

    uint64_t king = side_to_move == Color::Black ? white_kings : black_kings;
    uint64_t enemy_king =
        side_to_move == Color::White ? white_kings : black_kings;

    int king_index = fast_log_2(king);
    int enemy_king_index = fast_log_2(enemy_king);

    // King
    uint64_t enemy_king_moves = king_masks[enemy_king_index];
    while (enemy_king_moves != 0ULL) {
        int current_enemy_index = fast_log_2(enemy_king_moves);

        if (current_enemy_index == king_index)
            return false;

        enemy_king_moves ^= 1ULL << current_enemy_index;
    }

    // Pawns
    int index = king_index;

    if (index % 8 < 7) {
        // rightwards (from white's perspective)
        int attacked_index = side_to_move == Color::White ? index - 7 : index + 9;
        if (piece_table[attacked_index] == Piece::Pawn &&
                color_table[attacked_index] == side_to_move) {
            return false;
        }
    }
    if (index % 8 > 0) {
        // leftwards (from white's perspective)
        int attacked_index = side_to_move == Color::White ? index - 9 : index + 7;
        if (piece_table[attacked_index] == Piece::Pawn &&
                color_table[attacked_index] == side_to_move) {
            return false;
        }
    }
    // Pawns survived

    // Knights
    uint64_t possible_knight_checks = knight_masks[king_index];
    while (possible_knight_checks) {
        int current_index = fast_log_2(possible_knight_checks);
        if (color_table[current_index] != side_to_move) {
            // no danger here
        } else if (piece_table[current_index] == Piece::Knight) {
            // king dead
            return false;
        }
        possible_knight_checks ^= 1ULL << current_index;
    }

    // king survived knights

    // now "queen move gen" for king

    // first bishop style

    index = king_index;
    int down_steps = index / 8;
    int up_steps = 7 - down_steps;
    int left_steps = index % 8;
    int right_steps = 7 - left_steps;

    int left_down_steps = std::min(index / 8, index % 8);
    int left_up_steps = std::min(7 - (index / 8), index % 8);
    int right_down_steps = std::min(index / 8, 7 - (index % 8));
    int right_up_steps = std::min(7 - (index / 8), 7 - (index % 8));

    int current_index;

    for (int offset = 1; offset <= left_steps; offset++) {
        current_index = index - offset;
        if (color_table[current_index] != Color::Empty) {
            if (color_table[current_index] == side_to_move &&
                    (piece_table[current_index] == Piece::Queen ||
                     piece_table[current_index] == Piece::Rook)) {
                return false;
            } else {
                offset = left_steps + 1;
            }
        }
    }

    for (int offset = 1; offset <= right_steps; offset++) {
        current_index = index + offset;
        if (color_table[current_index] != Color::Empty) {
            if (color_table[current_index] == side_to_move &&
                    (piece_table[current_index] == Piece::Queen ||
                     piece_table[current_index] == Piece::Rook)) {
                return false;
            } else {
                offset = right_steps + 1;
            }
        }
    }

    for (int offset = 1; offset <= up_steps; offset++) {
        current_index = index + offset * 8;
        if (color_table[current_index] != Color::Empty) {
            if (color_table[current_index] == side_to_move &&
                    (piece_table[current_index] == Piece::Queen ||
                     piece_table[current_index] == Piece::Rook)) {
                return false;
            } else {
                offset = up_steps + 1;
            }
        }
    }

    for (int offset = 1; offset <= down_steps; offset++) {
        current_index = index - offset * 8;
        if (color_table[current_index] != Color::Empty) {
            if (color_table[current_index] == side_to_move &&
                    (piece_table[current_index] == Piece::Queen ||
                     piece_table[current_index] == Piece::Rook)) {
                return false;
            } else {
                offset = down_steps + 1;
            }
        }
    }

    for (int offset = 1; offset <= left_down_steps; offset++) {
        current_index = index - offset * 9;
        if (color_table[current_index] != Color::Empty) {
            if (color_table[current_index] == side_to_move &&
                    (piece_table[current_index] == Piece::Queen ||
                     piece_table[current_index] == Piece::Bishop)) {
                return false;
            } else {
                offset = left_down_steps + 1;
            }
        }
    }

    for (int offset = 1; offset <= left_up_steps; offset++) {
        current_index = index + offset * 7;
        if (color_table[current_index] != Color::Empty) {
            if (color_table[current_index] == side_to_move &&
                    (piece_table[current_index] == Piece::Queen ||
                     piece_table[current_index] == Piece::Bishop)) {
                return false;
            } else {
                offset = left_up_steps + 1;
            }
        }
    }

    for (int offset = 1; offset <= right_down_steps; offset++) {
        current_index = index - offset * 7;
        if (color_table[current_index] != Color::Empty) {
            if (color_table[current_index] == side_to_move &&
                    (piece_table[current_index] == Piece::Queen ||
                     piece_table[current_index] == Piece::Bishop)) {
                return false;
            } else {
                offset = right_down_steps + 1;
            }
        }
    }

    for (int offset = 1; offset <= right_up_steps; offset++) {
        current_index = index + offset * 9;
        if (color_table[current_index] != Color::Empty) {
            if (color_table[current_index] == side_to_move &&
                    (piece_table[current_index] == Piece::Queen ||
                     piece_table[current_index] == Piece::Bishop)) {
                return false;
            } else {
                offset = right_up_steps + 1;
            }
        }
    }

    return true;
}

void Position::set_piece(Piece piece, int index, Color col) {
    uint64_t bit = 1ULL << index;

    // remove the previous piece from bitboards
    if (piece_table[index] != Piece::Empty) {
        if (color_table[index] == Color::White) {
            switch (piece_table[index]) {
                case Piece::Pawn:
                    white_pawns ^= bit;
                    empty_squares |= bit;
                    hash ^= zobrist_table[0][index];
                    break;
                case Piece::Rook:
                    white_rooks ^= bit;
                    empty_squares |= bit;
                    hash ^= zobrist_table[1][index];
                    break;
                case Piece::Knight:
                    white_knights ^= bit;
                    empty_squares |= bit;
                    hash ^= zobrist_table[2][index];
                    break;
                case Piece::Bishop:
                    white_bishops ^= bit;
                    empty_squares |= bit;
                    hash ^= zobrist_table[3][index];
                    break;
                case Piece::Queen:
                    white_queens ^= bit;
                    empty_squares |= bit;
                    hash ^= zobrist_table[4][index];
                    break;
                case Piece::King:
                    white_kings ^= bit;
                    empty_squares |= bit;
                    hash ^= zobrist_table[5][index];
                    break;
                default:
                    std::cout << "PIECE NOT FOUND 2 1 index : " << index;
                    return;
            }
        } else {
            switch (piece_table[index]) {
                case Piece::Pawn:
                    black_pawns ^= bit;
                    empty_squares |= bit;
                    hash ^= zobrist_table[6][index];
                    break;
                case Piece::Rook:
                    black_rooks ^= bit;
                    empty_squares |= bit;
                    hash ^= zobrist_table[7][index];
                    break;
                case Piece::Knight:
                    black_knights ^= bit;
                    empty_squares |= bit;
                    hash ^= zobrist_table[8][index];
                    break;
                case Piece::Bishop:
                    black_bishops ^= bit;
                    empty_squares |= bit;
                    hash ^= zobrist_table[9][index];
                    break;
                case Piece::Queen:
                    black_queens ^= bit;
                    empty_squares |= bit;
                    hash ^= zobrist_table[10][index];
                    break;
                case Piece::King:
                    black_kings ^= bit;
                    empty_squares |= bit;
                    hash ^= zobrist_table[11][index];
                    break;
                default:
                    std::cout << "PIECE NOT FOUND 3 1 index:" << index;
                    return;
            }
        }
    }

    piece_table[index] = piece;
    color_table[index] = col;

    // innserting in Bitboards
    // Update individual Bitboards
    if (col == Color::White) {
        switch (piece) {
            case Piece::Pawn:
                white_pawns |= bit;
                empty_squares ^= bit;
                hash ^= zobrist_table[0][index];
                break;
            case Piece::Rook:
                white_rooks |= bit;
                empty_squares ^= bit;
                hash ^= zobrist_table[1][index];
                break;
            case Piece::Knight:
                white_knights |= bit;
                empty_squares ^= bit;
                hash ^= zobrist_table[2][index];
                break;
            case Piece::Bishop:
                white_bishops |= bit;
                empty_squares ^= bit;
                hash ^= zobrist_table[3][index];
                break;
            case Piece::Queen:
                white_queens |= bit;
                empty_squares ^= bit;
                hash ^= zobrist_table[4][index];
                break;
            case Piece::King:
                white_kings |= bit;
                empty_squares ^= bit;
                hash ^= zobrist_table[5][index];
                break;
            case Piece::Empty:
                break;
            default:
                std::cout << "PIECE NOT FOUND 2";
                return;
        }
    } else {
        switch (piece) {
            case Piece::Pawn:
                black_pawns |= bit;
                empty_squares ^= bit;
                hash ^= zobrist_table[6][index];
                break;
            case Piece::Rook:
                black_rooks |= bit;
                empty_squares ^= bit;
                hash ^= zobrist_table[7][index];
                break;
            case Piece::Knight:
                black_knights |= bit;
                empty_squares ^= bit;
                hash ^= zobrist_table[8][index];
                break;
            case Piece::Bishop:
                black_bishops |= bit;
                empty_squares ^= bit;
                hash ^= zobrist_table[9][index];
                break;
            case Piece::Queen:
                black_queens |= bit;
                empty_squares ^= bit;
                hash ^= zobrist_table[10][index];
                break;
            case Piece::King:
                black_kings |= bit;
                empty_squares ^= bit;
                hash ^= zobrist_table[11][index];
                break;
            case Piece::Empty:
                break;
            default:
                std::cout << "PIECE NOT FOUND 3";
                return;
        }
    }
}

void Position::make_move(Move m) {
    Piece moving_piece = piece_table[m.from];

    if (moving_piece == Piece::Empty) {
        print_full_board(*this);
        std::cout << m.from;
        std::cout << "moving an empty square";
        std::cout << m.to;
        std::cout << "\nwhite ks castle" << white_kingside_castling_right;
        std::cout << "\nwhite qs castle" << white_queenside_castling_right;
        std::cout << "\nblack ks castle" << black_kingside_castling_right;
        std::cout << "\nblack qs castle" << black_queenside_castling_right;
        for (auto moves : move_history)
            print_move(moves);
        exit(1);
    }

    // Castling right tracking
    if (moving_piece == Piece::King) {
        if (side_to_move == Color::White) {
            m.king_destroyed_short_castle = white_kingside_castling_right;
            m.king_destroyed_long_castle = white_queenside_castling_right;
            white_kingside_castling_right = false;
            white_queenside_castling_right = false;
        } else {
            m.king_destroyed_short_castle = black_kingside_castling_right;
            m.king_destroyed_long_castle = black_queenside_castling_right;
            black_kingside_castling_right = false;
            black_queenside_castling_right = false;
        }
    } else if (moving_piece == Piece::Rook) {
        if (side_to_move == Color::White) {
            if (white_kingside_castling_right) {
                if (m.from == 7) {
                    m.rook_destroyed_castle = true;
                    white_kingside_castling_right = false;
                }
            }
            if (white_queenside_castling_right) {
                if (m.from == 0) {
                    m.rook_destroyed_castle = true;
                    white_queenside_castling_right = false;
                }
            }
        } else {
            if (black_kingside_castling_right) {
                if (m.from == 63) {
                    m.rook_destroyed_castle = true;
                    black_kingside_castling_right = false;
                }
            }
            if (black_queenside_castling_right) {
                if (m.from == 56) {
                    m.rook_destroyed_castle = true;
                    black_queenside_castling_right = false;
                }
            }
        }
    }

    // En Passent
    if (is_en_passent(*this, m)) {
        int captured_pawn_index =
            side_to_move == Color::White ? m.to - 8 : m.to + 8;
        set_piece(Piece::Empty, captured_pawn_index, Color::Empty);
    }

    m.previous_en_passent_square = en_passent_square;
    if (moving_piece == Piece::Pawn) {
        if (std::abs(m.from - m.to) == 16) {
            en_passent_square = m.from < m.to ? m.to - 8 : m.to + 8;
        } else {
            en_passent_square = -1;
        }
    } else {
        en_passent_square = -1;
    }

    // delete from initial square
    set_piece(Piece::Empty, m.from, Color::Empty);

    m.previous_moves_since_pawnmove_or_capture = moves_since_panwmove_or_capture;

    m.captured_piece = piece_table[m.to];
    moves_since_panwmove_or_capture = m.captured_piece == Piece::Empty
        ? moves_since_panwmove_or_capture + 1
        : 0;
    // insert to new square
    if (m.promotion != Piece::Empty)
        moving_piece = m.promotion;
    set_piece(moving_piece, m.to, side_to_move);

    if (piece_table[m.to] == Piece::King) {
        if (m.from == 4) {
            if (m.to == 6) {
                set_piece(Piece::Empty, 7, Color::Empty);
                set_piece(Piece::Rook, 5, Color::White);
                m.castling = Castling::WhiteShort;
                white_kingside_castling_right = false;
            }
            if (m.to == 2) {
                set_piece(Piece::Empty, 0, Color::Empty);
                set_piece(Piece::Rook, 3, Color::White);
                m.castling = Castling::WhiteLong;
                white_queenside_castling_right = false;
            }
        } else if (m.from == 60) {
            if (m.to == 62) {
                set_piece(Piece::Empty, 63, Color::Empty);
                set_piece(Piece::Rook, 61, Color::Black);
                m.castling = Castling::BlackShort;
                black_kingside_castling_right = false;
            }
            if (m.to == 58) {
                set_piece(Piece::Empty, 56, Color::Empty);
                set_piece(Piece::Rook, 59, Color::Black);
                m.castling = Castling::BlackLong;
                black_queenside_castling_right = false;
            }
        }
    }

    if (side_to_move == Color::White) {
        side_to_move = Color::Black;
    } else {
        side_to_move = Color::White;
    }
    move_history.push_back(m);
    hash_history.push_back(hash);
}

void Position::unmake_move() {
    Move m = move_history.back();
    move_history.pop_back();
    hash_history.pop_back();

    moves_since_panwmove_or_capture = m.previous_moves_since_pawnmove_or_capture;

    // Important to note this happening here
    if (side_to_move == Color::White) {
        side_to_move = Color::Black;
    } else {
        side_to_move = Color::White;
    }

    if (m.king_destroyed_short_castle) {
        if (side_to_move == Color::White) {
            white_kingside_castling_right = true;
        } else {
            black_kingside_castling_right = true;
        }
    }
    if (m.king_destroyed_long_castle) {
        if (side_to_move == Color::White) {
            white_queenside_castling_right = true;
        } else {
            black_queenside_castling_right = true;
        }
    }
    if (m.rook_destroyed_castle) {
        if (side_to_move == Color::White) {
            if (m.from == 7) {
                white_kingside_castling_right = true;
            } else {
                white_queenside_castling_right = true;
            }
        } else {
            if (m.from == 63) {
                black_kingside_castling_right = true;
            } else {
                black_queenside_castling_right = true;
            }
        }
    }

    en_passent_square = m.previous_en_passent_square;

    if (m.castling == Castling::None) {

        if (en_passent_square == m.to && piece_table[m.to] == Piece::Pawn) {
            int captured_pawn_index =
                side_to_move == Color::White ? m.to - 8 : m.to + 8;
            set_piece(Piece::Pawn, captured_pawn_index,
                    side_to_move == Color::White ? Color::Black : Color::White);
        }

        Piece moved_piece = piece_table[m.to];
        if (m.promotion != Piece::Empty)
            moved_piece = Piece::Pawn;
        set_piece(moved_piece, m.from, side_to_move);
        set_piece(Piece::Empty, m.to, Color::Empty);
        if (m.captured_piece != Piece::Empty)
            set_piece(m.captured_piece, m.to,
                    side_to_move == Color::White ? Color::Black : Color::White);

    } else {
        switch (m.castling) {
            case Castling::WhiteShort:
                set_piece(Piece::Empty, 5, Color::Empty);
                set_piece(Piece::Empty, 6, Color::Empty);
                set_piece(Piece::King, 4, Color::White);
                set_piece(Piece::Rook, 7, Color::White);
                white_kingside_castling_right = true;
                break;
            case Castling::WhiteLong:
                set_piece(Piece::Empty, 2, Color::Empty);
                set_piece(Piece::Empty, 3, Color::Empty);
                set_piece(Piece::King, 4, Color::White);
                set_piece(Piece::Rook, 0, Color::White);
                white_queenside_castling_right = true;
                break;
            case Castling::BlackShort:
                set_piece(Piece::Empty, 61, Color::Empty);
                set_piece(Piece::Empty, 62, Color::Empty);
                set_piece(Piece::King, 60, Color::Black);
                set_piece(Piece::Rook, 63, Color::Black);
                black_kingside_castling_right = true;
                break;
            case Castling::BlackLong:
                set_piece(Piece::Empty, 58, Color::Empty);
                set_piece(Piece::Empty, 59, Color::Empty);
                set_piece(Piece::King, 60, Color::Black);
                set_piece(Piece::Rook, 56, Color::Black);
                black_queenside_castling_right = true;
                break;
            case Castling::None:
                break;
        }
    }
}

bool is_consistant(Position &p) {
    std::string bbs = "";
    for (int rank = 7; rank >= 0; --rank) {
        for (int file = 0; file < 8; ++file) {
            int square = rank * 8 + file;
            if (p.black_pawns & (1ULL << square)) {
                bbs += "p ";
            } else if (p.black_knights & (1ULL << square)) {
                bbs += "n ";
            } else if (p.black_bishops & (1ULL << square)) {
                bbs += "b ";
            } else if (p.black_rooks & (1ULL << square)) {
                bbs += "r ";
            } else if (p.black_queens & (1ULL << square)) {
                bbs += "q ";
            } else if (p.black_kings & (1ULL << square)) {
                bbs += "k ";
            } else if (p.white_pawns & (1ULL << square)) {
                bbs += "P ";
            } else if (p.white_knights & (1ULL << square)) {
                bbs += "N ";
            } else if (p.white_bishops & (1ULL << square)) {
                bbs += "B ";
            } else if (p.white_rooks & (1ULL << square)) {
                bbs += "R ";
            } else if (p.white_queens & (1ULL << square)) {
                bbs += "Q ";
            } else if (p.white_kings & (1ULL << square)) {
                bbs += "K ";
            } else {
                bbs += ". ";
            }
        }
        bbs += "\n";
    }

    std::string tbs = "";
    for (int rank = 8; rank >= 1; --rank) {
        for (char file = 'a'; file <= 'h'; ++file) {
            int index = get_index(file, rank);
            auto piece = p.piece_table[index];
            char res;
            switch (piece) {
                case Piece::Pawn:
                    res = 'p';
                    break;
                case Piece::Rook:
                    res = 'r';
                    break;
                case Piece::Knight:
                    res = 'n';
                    break;
                case Piece::Bishop:
                    res = 'b';
                    break;
                case Piece::Queen:
                    res = 'q';
                    break;
                case Piece::King:
                    res = 'k';
                    break;
                case Piece::Empty:
                    res = '.';
                    break;
                default:
                    std::cout << "PANIC, invalid piece in 8x8\n";
            }
            if (p.color_table[index] == Color::White) {
                res -= 32;
            }
            tbs += res;

            tbs += " ";
        }
        tbs += "\n";
    }
    bool result = bbs == tbs;
    if (!result) {
        std::cout << "bbs\n" << bbs;
        std::cout << "\ntbs\n" << tbs;
    }
    return result;
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

void print_full_board(Position &p) {
    std::cout << "\nBitboard-based\n";
    for (int rank = 7; rank >= 0; --rank) {
        for (int file = 0; file < 8; ++file) {
            int square = rank * 8 + file;
            if (p.black_pawns & (1ULL << square)) {
                std::cout << "p ";
            } else if (p.black_knights & (1ULL << square)) {
                std::cout << "n ";
            } else if (p.black_bishops & (1ULL << square)) {
                std::cout << "b ";
            } else if (p.black_rooks & (1ULL << square)) {
                std::cout << "r ";
            } else if (p.black_queens & (1ULL << square)) {
                std::cout << "q ";
            } else if (p.black_kings & (1ULL << square)) {
                std::cout << "k ";
            } else if (p.white_pawns & (1ULL << square)) {
                std::cout << "P ";
            } else if (p.white_knights & (1ULL << square)) {
                std::cout << "N ";
            } else if (p.white_bishops & (1ULL << square)) {
                std::cout << "B ";
            } else if (p.white_rooks & (1ULL << square)) {
                std::cout << "R ";
            } else if (p.white_queens & (1ULL << square)) {
                std::cout << "Q ";
            } else if (p.white_kings & (1ULL << square)) {
                std::cout << "K ";
            } else {
                std::cout << ". ";
            }
        }
        std::cout << std::endl;
    }
    std::cout << "\n8x8-based\n";
    for (int rank = 8; rank >= 1; --rank) {
        for (char file = 'a'; file <= 'h'; ++file) {
            int index = get_index(file, rank);
            auto piece = p.piece_table[index];
            char res;
            switch (piece) {
                case Piece::Pawn:
                    res = 'p';
                    break;
                case Piece::Rook:
                    res = 'r';
                    break;
                case Piece::Knight:
                    res = 'n';
                    break;
                case Piece::Bishop:
                    res = 'b';
                    break;
                case Piece::Queen:
                    res = 'q';
                    break;
                case Piece::King:
                    res = 'k';
                    break;
                case Piece::Empty:
                    res = '.';
                    break;
                default:
                    std::cout << "PANIC, invalid piece in 8x8\n";
                    return;
            }
            if (p.color_table[index] == Color::White) {
                res -= 32;
            }
            std::cout << res << " ";
        }
        std::cout << std::endl;
    }

    std::cout << "\n8x8-based color\n";
    for (int rank = 8; rank >= 1; --rank) {
        for (char file = 'a'; file <= 'h'; ++file) {
            int index = get_index(file, rank);
            auto color = p.color_table[index];
            char res = color == Color::White   ? 'W'
                : color == Color::Black ? 'B'
                : '.';
            std::cout << res << " ";
        }
        std::cout << std::endl;
    }
}

void cmdl_game_loop() {

    Position p("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    print_full_board(p);

    char from;
    int from_row;
    char to;
    int to_row;

    for (;;) {
        std::cin >> from >> from_row >> to >> to_row;
        if (from < 'a' || from > 'h')
            continue;
        if (to < 'a' || to > 'h')
            continue;
        if (from_row < 1 || from_row > 8)
            continue;
        if (to_row < 1 || to_row > 8)
            continue;

        int from_index = get_index(from, from_row);
        int to_index = get_index(to, to_row);
        Move m(from_index, to_index);
        p.make_move(m);
        print_full_board(p);
    }
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
    if (p.piece_table[m.to] != Piece::Empty)
        return true;
    return is_en_passent(p, m);
}

bool is_en_passent(Position &p, Move &m) {
    if (p.piece_table[m.from] != Piece::Pawn)
        return false;
    return p.en_passent_square == m.to;
}

bool is_castle(Position &p, Move &m) {
    if (m.from == 4 && p.piece_table[m.from] == Piece::King)
        return m.to == 6 || m.to == 2;
    if (m.from == 60 && p.piece_table[m.from] == Piece::King)
        return m.to == 62 || m.to == 58;
    return false;
}
