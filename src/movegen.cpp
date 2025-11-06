#include <csignal>
#include <vector>

#include "attack_masks.h"
#include "board.h"
#include "movegen.h"
#include "utils.h"

bool is_move_valid(Move &m, Position &p) {
    if (p.side_to_move == p.color_table[m.to])
        return false;
    bool res = false;
    p.make_move(m);
    if (p.position_is_legal())
        res = true;
    p.unmake_move();
    return res;
}

std::vector<Move> &generate_pawn_moves(std::vector<Move> &res, Position &p) {
    uint64_t white_pawns_starting_mask = 0x000000000000FF00ULL;
    uint64_t black_pawns_starting_mask = 0x00FF000000000000ULL;

    uint64_t pawns;
    uint64_t pushed_pawns;
    uint64_t pushable_pawns;
    uint64_t double_pushable_pawns;

    if (p.side_to_move == Color::White) {
        pawns = p.white_pawns;
        pushed_pawns = p.white_pawns << 8;
        pushable_pawns = (pushed_pawns & p.empty_squares) >> 8;
        double_pushable_pawns =
            (((pushable_pawns & white_pawns_starting_mask) << 16) &
             p.empty_squares) >>
            16;
    } else {
        pawns = p.black_pawns;
        pushed_pawns = p.black_pawns >> 8;
        pushable_pawns = (pushed_pawns & p.empty_squares) << 8;
        double_pushable_pawns =
            (((pushable_pawns & black_pawns_starting_mask) >> 16) & p.empty_squares)
            << 16;
    }

    while (pushable_pawns) {
        int index = fast_log_2(pushable_pawns);

        Move m(index, index);
        m.to = p.side_to_move == Color::White ? index + 8 : index - 8;
        if (m.to < 56 && m.to > 7) {
            if (is_move_valid(m, p))
                res.push_back(m);
        } else {
            m.promotion = Piece::Queen;
            if (is_move_valid(m, p))
                res.push_back(m);
            m.promotion = Piece::Knight;
            if (is_move_valid(m, p))
                res.push_back(m);
            m.promotion = Piece::Rook;
            if (is_move_valid(m, p))
                res.push_back(m);
            m.promotion = Piece::Bishop;
            if (is_move_valid(m, p))
                res.push_back(m);
        }

        pushable_pawns ^= 1ULL << index;
    }

    while (double_pushable_pawns) {
        int index = fast_log_2(double_pushable_pawns);

        Move m(index, index);
        m.to = p.side_to_move == Color::White ? index + 16 : index - 16;
        if (is_move_valid(m, p))
            res.push_back(m);

        double_pushable_pawns ^= 1ULL << index;
    }

    // captures
    while (pawns) {
        int index = fast_log_2(pawns);

        if (index % 8 < 7) {
            // rightwards (from white's perspective)
            int attacked_index =
                p.side_to_move == Color::White ? index + 9 : index - 7;
            if (p.piece_table[attacked_index] != Piece::Empty &&
                    p.color_table[attacked_index] != p.side_to_move) {
                Move m(index, attacked_index);
                m.captured_piece = p.piece_table[attacked_index];
                if (attacked_index < 56 && attacked_index > 7) {
                    if (is_move_valid(m, p))
                        res.push_back(m);
                } else {
                    m.promotion = Piece::Queen;
                    if (is_move_valid(m, p))
                        res.push_back(m);
                    m.promotion = Piece::Knight;
                    if (is_move_valid(m, p))
                        res.push_back(m);
                    m.promotion = Piece::Rook;
                    if (is_move_valid(m, p))
                        res.push_back(m);
                    m.promotion = Piece::Bishop;
                    if (is_move_valid(m, p))
                        res.push_back(m);
                }
            }
        }

        if (index % 8 > 0) {
            // leftwards (from white's perspective)
            int attacked_index =
                p.side_to_move == Color::White ? index + 7 : index - 9;
            if (p.piece_table[attacked_index] != Piece::Empty &&
                    p.color_table[attacked_index] != p.side_to_move) {
                Move m(index, attacked_index);
                m.captured_piece = p.piece_table[attacked_index];
                if (attacked_index < 56 && attacked_index > 7) {
                    if (is_move_valid(m, p))
                        res.push_back(m);
                } else {
                    m.promotion = Piece::Queen;
                    if (is_move_valid(m, p))
                        res.push_back(m);
                    m.promotion = Piece::Knight;
                    if (is_move_valid(m, p))
                        res.push_back(m);
                    m.promotion = Piece::Rook;
                    if (is_move_valid(m, p))
                        res.push_back(m);
                    m.promotion = Piece::Bishop;
                    if (is_move_valid(m, p))
                        res.push_back(m);
                }
            }
        }

        pawns ^= 1ULL << index;
    }

    // En Passent

    if (p.en_passent_square != -1) {
        if (p.en_passent_square % 8 < 7) {
            int from_index = p.side_to_move == Color::White ? p.en_passent_square - 7
                : p.en_passent_square + 9;
            if (p.piece_table[from_index] == Piece::Pawn &&
                    p.color_table[from_index] == p.side_to_move) {
                Move m(from_index, p.en_passent_square);
                if (is_move_valid(m, p))
                    res.push_back(m);
            }
        }
        if (p.en_passent_square % 8 > 0) {
            int from_index = p.side_to_move == Color::White ? p.en_passent_square - 9
                : p.en_passent_square + 7;
            if (p.piece_table[from_index] == Piece::Pawn &&
                    p.color_table[from_index] == p.side_to_move) {
                Move m(from_index, p.en_passent_square);
                if (is_move_valid(m, p))
                    res.push_back(m);
            }
        }
    }

    return res;
}

std::vector<Move> &generate_knight_moves(std::vector<Move> &res, Position &p) {
    uint64_t knights =
        p.side_to_move == Color::White ? p.white_knights : p.black_knights;

    while (knights) {
        int index = fast_log_2(knights);
        if (p.piece_table[index] != Piece::Knight) {
            std::cout << to_string(p.piece_table[index]);
            std::cout << to_string(p.color_table[index]);
            std::cout << to_string(p.side_to_move);
        }
        uint64_t possible_squares = knight_masks[index];

        while (possible_squares) {
            int current_index = fast_log_2(possible_squares);

            if (p.piece_table[current_index] == Piece::Empty) {
                Move m(index, current_index);
                if (is_move_valid(m, p))
                    res.push_back(m);
            } else if (p.color_table[current_index] != p.side_to_move) {
                // capture
                Move m(index, current_index);
                if (p.color_table[current_index] != Color::Empty) {
                    m.captured_piece = p.piece_table[current_index];
                }
                if (is_move_valid(m, p))
                    res.push_back(m);
            }
            possible_squares ^= 1ULL << current_index;
        }

        knights ^= 1ULL << index;
    }

    return res;
}

std::vector<Move> &generate_rook_moves(std::vector<Move> &res, Position &p) {
    uint64_t rooks =
        p.side_to_move == Color::White ? p.white_rooks : p.black_rooks;

    while (rooks != 0) {
        int index = fast_log_2(rooks);

        int down_steps = index / 8;
        int up_steps = 7 - down_steps;
        int left_steps = index % 8;
        int right_steps = 7 - left_steps;

        int current_index;

        for (int offset = 1; offset <= left_steps; offset++) {
            current_index = index - offset;
            Move m(index, current_index);
            if (p.color_table[current_index] != Color::Empty) {
                offset = left_steps + 1;
            }
            if (is_move_valid(m, p))
                res.push_back(m);
        }

        for (int offset = 1; offset <= right_steps; offset++) {
            current_index = index + offset;
            Move m(index, current_index);
            if (p.color_table[current_index] != Color::Empty) {
                offset = right_steps + 1;
            }
            if (is_move_valid(m, p))
                res.push_back(m);
        }

        for (int offset = 1; offset <= up_steps; offset++) {
            current_index = index + offset * 8;
            Move m(index, current_index);
            if (p.color_table[current_index] != Color::Empty) {
                offset = up_steps + 1;
            }
            if (is_move_valid(m, p))
                res.push_back(m);
        }

        for (int offset = 1; offset <= down_steps; offset++) {
            current_index = index - offset * 8;
            Move m(index, current_index);
            if (p.color_table[current_index] != Color::Empty) {
                offset = down_steps + 1;
            }
            if (is_move_valid(m, p))
                res.push_back(m);
        }

        rooks ^= 1ULL << index;
    }

    return res;
}

std::vector<Move> &generate_bishop_moves(std::vector<Move> &res, Position &p) {
    uint64_t bishops =
        p.side_to_move == Color::White ? p.white_bishops : p.black_bishops;

    while (bishops != 0) {
        int index = fast_log_2(bishops);

        int left_down_steps = std::min(index / 8, index % 8);
        int left_up_steps = std::min(7 - (index / 8), index % 8);
        int right_down_steps = std::min(index / 8, 7 - (index % 8));
        int right_up_steps = std::min(7 - (index / 8), 7 - (index % 8));

        int current_index;

        for (int offset = 1; offset <= left_down_steps; offset++) {
            current_index = index - offset * 9;
            Move m(index, current_index);
            if (p.color_table[current_index] != Color::Empty) {
                offset = left_down_steps + 1;
            }
            if (is_move_valid(m, p))
                res.push_back(m);
        }

        for (int offset = 1; offset <= left_up_steps; offset++) {
            current_index = index + offset * 7;
            Move m(index, current_index);
            if (p.color_table[current_index] != Color::Empty) {
                offset = left_up_steps + 1;
            }
            if (is_move_valid(m, p))
                res.push_back(m);
        }

        for (int offset = 1; offset <= right_down_steps; offset++) {
            current_index = index - offset * 7;
            Move m(index, current_index);
            if (p.color_table[current_index] != Color::Empty) {
                offset = right_down_steps + 1;
            }
            if (is_move_valid(m, p))
                res.push_back(m);
        }

        for (int offset = 1; offset <= right_up_steps; offset++) {
            current_index = index + offset * 9;
            Move m(index, current_index);
            if (p.color_table[current_index] != Color::Empty) {
                offset = right_up_steps + 1;
            }
            if (is_move_valid(m, p))
                res.push_back(m);
        }

        bishops ^= 1ULL << index;
    }

    return res;
}

std::vector<Move> &generate_queen_moves(std::vector<Move> &res, Position &p) {
    uint64_t queens =
        p.side_to_move == Color::White ? p.white_queens : p.black_queens;

    // Bishop pattern first, then rook pattern
    while (queens != 0) {
        int index = fast_log_2(queens);

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
            Move m(index, current_index);
            if (p.color_table[current_index] != Color::Empty) {
                offset = left_steps + 1;
            }
            if (is_move_valid(m, p))
                res.push_back(m);
        }

        for (int offset = 1; offset <= right_steps; offset++) {
            current_index = index + offset;
            Move m(index, current_index);
            if (p.color_table[current_index] != Color::Empty) {
                offset = right_steps + 1;
            }
            if (is_move_valid(m, p))
                res.push_back(m);
        }

        for (int offset = 1; offset <= up_steps; offset++) {
            current_index = index + offset * 8;
            Move m(index, current_index);
            if (p.color_table[current_index] != Color::Empty) {
                offset = up_steps + 1;
            }
            if (is_move_valid(m, p))
                res.push_back(m);
        }

        for (int offset = 1; offset <= down_steps; offset++) {
            current_index = index - offset * 8;
            Move m(index, current_index);
            if (p.color_table[current_index] != Color::Empty) {
                offset = down_steps + 1;
            }
            if (is_move_valid(m, p))
                res.push_back(m);
        }

        for (int offset = 1; offset <= left_down_steps; offset++) {
            current_index = index - offset * 9;
            Move m(index, current_index);
            if (p.color_table[current_index] != Color::Empty) {
                offset = left_down_steps + 1;
            }
            if (is_move_valid(m, p))
                res.push_back(m);
        }

        for (int offset = 1; offset <= left_up_steps; offset++) {
            current_index = index + offset * 7;
            Move m(index, current_index);
            if (p.color_table[current_index] != Color::Empty) {
                offset = left_up_steps + 1;
            }
            if (is_move_valid(m, p))
                res.push_back(m);
        }

        for (int offset = 1; offset <= right_down_steps; offset++) {
            current_index = index - offset * 7;
            Move m(index, current_index);
            if (p.color_table[current_index] != Color::Empty) {
                offset = right_down_steps + 1;
            }
            if (is_move_valid(m, p))
                res.push_back(m);
        }

        for (int offset = 1; offset <= right_up_steps; offset++) {
            current_index = index + offset * 9;
            Move m(index, current_index);
            if (p.color_table[current_index] != Color::Empty) {
                offset = right_up_steps + 1;
            }
            if (is_move_valid(m, p))
                res.push_back(m);
        }

        queens ^= 1ULL << index;
    }
    return res;
}

std::vector<Move> &generate_king_moves(std::vector<Move> &res, Position &p) {
    uint64_t king =
        p.side_to_move == Color::White ? p.white_kings : p.black_kings;
    int index = fast_log_2(king);

    uint64_t moves = king_masks[index];

    while (moves != 0ULL) {
        int current_index = fast_log_2(moves);
        if (p.piece_table[current_index] == Piece::Empty) {
            Move m(index, current_index);

            if (is_move_valid(m, p))
                res.push_back(m);
        } else if (p.color_table[current_index] != p.side_to_move &&
                p.color_table[current_index] != Color::Empty) {
            // capture
            Move m(index, current_index);
            m.captured_piece = p.piece_table[current_index];

            if (is_move_valid(m, p))
                res.push_back(m);
        }
        moves ^= 1ULL << current_index;
    }

    // castling
    // filtered for legality already

    if (p.is_check())
        return res;

    if (p.side_to_move == Color::White) {
        if (p.piece_table[4] != Piece::King)
            return res;
        if (p.white_kingside_castling_right) {
            bool can_castle_kingside = p.piece_table[5] == Piece::Empty &&
                p.piece_table[6] == Piece::Empty &&
                p.piece_table[7] == Piece::Rook &&
                p.color_table[7] == p.side_to_move;

            if (can_castle_kingside) {
                Move m(4, 5);
                p.make_move(m);
                can_castle_kingside &= p.position_is_legal();
                p.unmake_move();

                m.to = 6;
                p.make_move(m);
                can_castle_kingside &= p.position_is_legal();
                p.unmake_move();
            }

            if (can_castle_kingside) {
                Move m(4, 6);
                if (is_move_valid(m, p))
                    res.push_back(m);
            }
        }

        if (p.white_queenside_castling_right) {
            bool can_castle_queenside = p.piece_table[3] == Piece::Empty &&
                p.piece_table[2] == Piece::Empty &&
                p.piece_table[1] == Piece::Empty &&
                p.piece_table[0] == Piece::Rook &&
                p.color_table[0] == p.side_to_move;

            if (can_castle_queenside) {
                Move m(4, 3);
                p.make_move(m);
                can_castle_queenside &= p.position_is_legal();
                p.unmake_move();

                m.to = 2;
                p.make_move(m);
                can_castle_queenside &= p.position_is_legal();
                p.unmake_move();
            }

            if (can_castle_queenside) {
                Move m(4, 2);
                if (is_move_valid(m, p))
                    res.push_back(m);
            }
        }
    } else {
        if (p.piece_table[60] != Piece::King)
            return res;
        if (p.black_kingside_castling_right) {
            bool can_castle_kingside = p.piece_table[61] == Piece::Empty &&
                p.piece_table[62] == Piece::Empty &&
                p.piece_table[63] == Piece::Rook &&
                p.color_table[63] == p.side_to_move;

            if (can_castle_kingside) {
                Move m(60, 61);
                p.make_move(m);
                can_castle_kingside &= p.position_is_legal();
                p.unmake_move();

                m.to = 62;
                p.make_move(m);
                can_castle_kingside &= p.position_is_legal();
                p.unmake_move();
            }

            if (can_castle_kingside) {
                Move m(60, 62);
                if (is_move_valid(m, p))
                    res.push_back(m);
            }
        }

        if (p.black_queenside_castling_right) {
            bool can_castle_queenside = p.piece_table[59] == Piece::Empty &&
                p.piece_table[58] == Piece::Empty &&
                p.piece_table[57] == Piece::Empty &&
                p.piece_table[56] == Piece::Rook &&
                p.color_table[56] == p.side_to_move;

            if (can_castle_queenside) {
                Move m(60, 59);
                p.make_move(m);
                can_castle_queenside &= p.position_is_legal();
                p.unmake_move();

                m.to = 58;
                p.make_move(m);
                can_castle_queenside &= p.position_is_legal();
                p.unmake_move();
            }

            if (can_castle_queenside) {
                Move m(60, 58);
                if (is_move_valid(m, p))
                    res.push_back(m);
            }
        }
    }

    return res;
}

std::vector<Move> generate_moves(Position &p) {
    std::vector<Move> res;

    res = generate_pawn_moves(res, p);
    res = generate_knight_moves(res, p);
    res = generate_rook_moves(res, p);
    res = generate_bishop_moves(res, p);
    res = generate_queen_moves(res, p);
    res = generate_king_moves(res, p);

    return res;
}

std::vector<Move> generate_captures(Position &p) {
    std::vector<Move> moves = generate_moves(p);
    std::vector<Move> res;

    for (auto &m : moves)
        if (p.piece_table[m.to] != Piece::Empty)
            res.push_back(m);

    return res;
}
