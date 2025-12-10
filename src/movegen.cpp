#include <array>
#include <cstdint>

#include "attack_masks.h"
#include "board.h"
#include "movegen.h"
#include "square.h"
#include "utils.h"
#include "bitboard.h"

static Bitboard RANK_1(0xff);
static Bitboard RANK_2(0xff00);
static Bitboard RANK_3(0xff0000);
static Bitboard RANK_4(0xff000000);
static Bitboard RANK_5(0xff00000000);
static Bitboard RANK_6(0xff0000000000);
static Bitboard RANK_7(0xff000000000000);
static Bitboard RANK_8(0xff00000000000000);

static Bitboard A_FILE(0x101010101010101);
static Bitboard B_FILE(0x202020202020202);
static Bitboard C_FILE(0x404040404040404);
static Bitboard D_FILE(0x808080808080808);
static Bitboard E_FILE(0x1010101010101010);
static Bitboard F_FILE(0x2020202020202020);
static Bitboard G_FILE(0x4040404040404040);
static Bitboard H_FILE(0x8080808080808080);

static constexpr int one_row_offset = 8;
static constexpr int two_row_offset = 16;
static Bitboard ABCDEFG_FILE(0x7f7f7f7f7f7f7f7f);
static Bitboard BCDEFGH_FILE(0xfefefefefefefefe);
static Bitboard RANK_2_TO_6(0xffffffffff00);
static Bitboard RANK_3_TO_7(0xffffffffff0000);

void generate_pawn_moves(Movelist &res, Position &p) {
    Bitboard pawns;
    Bitboard promotable_pawns;
    Bitboard pushed_pawns;
    Bitboard double_pushed_pawns;
    Bitboard left_capuring_pawns;
    Bitboard right_capuring_pawns;

    if (p.side_to_move == Color::White) {
        pawns = Bitboard(p.white_pawns);
        promotable_pawns = Bitboard(p.white_pawns).masked_by(RANK_7);
        pushed_pawns = Bitboard(p.white_pawns).masked_by(RANK_2_TO_6);
        double_pushed_pawns = Bitboard(p.white_pawns).masked_by(RANK_2_TO_6);
        left_capuring_pawns = Bitboard(p.white_pawns).masked_by(RANK_2_TO_6);
        right_capuring_pawns = Bitboard(p.white_pawns).masked_by(RANK_2_TO_6);

        pushed_pawns
            .shift_rank_up()
            .masked_by(Bitboard(p.empty_squares));

        double_pushed_pawns
            .masked_by(RANK_2)
            .shift_rank_up()
            .masked_by(Bitboard(p.empty_squares))
            .shift_rank_up()
            .masked_by(Bitboard(p.empty_squares));

        left_capuring_pawns
            .masked_by(BCDEFGH_FILE)
            .shift_rank_up()
            .shift_left()
            .masked_by(Bitboard(p.occupied_squares));

        right_capuring_pawns
            .masked_by(ABCDEFG_FILE)
            .shift_rank_up()
            .shift_right()
            .masked_by(Bitboard(p.occupied_squares));

        while (promotable_pawns) {
            Square msb = promotable_pawns.msb_pop();
            Square to = msb;
            to.shift_rank_up();

            if (p.color_table[to.value()] == Color::Empty) {
                Move m(msb, to);
                m.promotion = Piece::Knight;
                res.add(m);
                m.promotion = Piece::Rook;
                res.add(m);
                m.promotion = Piece::Bishop;
                res.add(m);
                m.promotion = Piece::Queen;
                res.add(m);
            }

            if (msb.file() > 0) {
                Square left_to = to;
                left_to.shift_file_left();

                Move m(msb, left_to);
                m.promotion = Piece::Knight;
                res.add(m);
                m.promotion = Piece::Rook;
                res.add(m);
                m.promotion = Piece::Bishop;
                res.add(m);
                m.promotion = Piece::Queen;
                res.add(m);
            }

            if (msb.file() < 7) {
                Square right_to = to;
                right_to.shift_file_right();

                Move m(msb, right_to);
                m.promotion = Piece::Knight;
                res.add(m);
                m.promotion = Piece::Rook;
                res.add(m);
                m.promotion = Piece::Bishop;
                res.add(m);
                m.promotion = Piece::Queen;
                res.add(m);
            }
        }

        while (left_capuring_pawns) {
            Square msb = left_capuring_pawns.msb_pop();
            if (p.color_table[msb.value()] != p.side_to_move) {
                Square from = msb;
                from.shift_rank_down().shift_file_right();
                Move m(from, msb);
                res.add(m);
            }
        }

        while (right_capuring_pawns) {
            Square msb = right_capuring_pawns.msb_pop();
            if (p.color_table[msb.value()] != p.side_to_move) {
                Square from = msb;
                from.shift_rank_down().shift_file_left();
                Move m(from, msb);
                res.add(m);
            }
        }

        while (double_pushed_pawns) {
            Square msb = double_pushed_pawns.msb_pop();
            Square from = msb;
            from.shift_rank_down().shift_rank_down();
            Move m(from, msb);
            res.add(m);
        }

        while (pushed_pawns) {
            Square msb = pushed_pawns.msb_pop();
            Square from = msb;
            from.shift_rank_down();
            Move m(from, msb);
            res.add(m);
        }

    } else {
        pawns = Bitboard(p.black_pawns);
        promotable_pawns = Bitboard(p.black_pawns).masked_by(RANK_2);
        pushed_pawns = Bitboard(p.black_pawns).masked_by(RANK_3_TO_7);
        double_pushed_pawns = Bitboard(p.black_pawns).masked_by(RANK_3_TO_7);
        left_capuring_pawns = Bitboard(p.black_pawns).masked_by(RANK_3_TO_7);
        right_capuring_pawns = Bitboard(p.black_pawns).masked_by(RANK_3_TO_7);

        pushed_pawns
            .shift_rank_down()
            .masked_by(Bitboard(p.empty_squares));

        double_pushed_pawns
            .masked_by(RANK_7)
            .shift_rank_down()
            .masked_by(Bitboard(p.empty_squares))
            .shift_rank_down()
            .masked_by(Bitboard(p.empty_squares));

        left_capuring_pawns
            .masked_by(BCDEFGH_FILE)
            .shift_rank_down()
            .shift_left()
            .masked_by(Bitboard(p.occupied_squares));

        right_capuring_pawns
            .masked_by(ABCDEFG_FILE)
            .shift_rank_down()
            .shift_right()
            .masked_by(Bitboard(p.occupied_squares));

        while (promotable_pawns) {
            Square msb = promotable_pawns.msb_pop();
            Square to = msb;
            to.shift_rank_up();

            if (p.color_table[to.value()] == Color::Empty) {
                Move m(msb, to);
                m.promotion = Piece::Knight;
                res.add(m);
                m.promotion = Piece::Rook;
                res.add(m);
                m.promotion = Piece::Bishop;
                res.add(m);
                m.promotion = Piece::Queen;
                res.add(m);
            }

            if (msb.file() > 0) {
                Square left_to = to;
                left_to.shift_file_left();

                Move m(msb, left_to);
                m.promotion = Piece::Knight;
                res.add(m);
                m.promotion = Piece::Rook;
                res.add(m);
                m.promotion = Piece::Bishop;
                res.add(m);
                m.promotion = Piece::Queen;
                res.add(m);
            }

            if (msb.file() < 7) {
                Square right_to = to;
                right_to.shift_file_right();

                Move m(msb, right_to);
                m.promotion = Piece::Knight;
                res.add(m);
                m.promotion = Piece::Rook;
                res.add(m);
                m.promotion = Piece::Bishop;
                res.add(m);
                m.promotion = Piece::Queen;
                res.add(m);
            }
        }

        while (left_capuring_pawns) {
            Square msb = left_capuring_pawns.msb_pop();
            if (p.color_table[msb.value()] != p.side_to_move) {
                Square from = msb;
                from.shift_rank_up().shift_file_right();
                Move m(from, msb);
                res.add(m);
            }
        }

        while (right_capuring_pawns) {
            Square msb = right_capuring_pawns.msb_pop();
            if (p.color_table[msb.value()] != p.side_to_move) {
                Square from = msb;
                from.shift_rank_up().shift_file_left();
                Move m(from, msb);
                res.add(m);
            }
        }

        while (double_pushed_pawns) {
            Square msb = double_pushed_pawns.msb_pop();
            Square from = msb;
            from.shift_rank_up().shift_rank_up();
            Move m(from, msb);
            res.add(m);
        }
        
        while (pushed_pawns) {
            Square msb = pushed_pawns.msb_pop();
            Square from = msb;
            from.shift_rank_up();
            Move m(from, msb);
            res.add(m);
        }
    }
}

void generate_knight_moves(Movelist &res, Position &p) {
    Bitboard knights =
        p.side_to_move == Color::White ? Bitboard(p.white_knights) : Bitboard(p.black_knights);

    while (knights) {
        Square from = knights.msb_pop();

        Bitboard possible_squares = Bitboard(knight_masks[from.value()]);

        while (possible_squares) {
            Square to = possible_squares.msb_pop();

            if (p.color_table[to.value()] != p.side_to_move) {
                Move m(from, to);
                res.add(m);
            }
        }
    }
}

void generate_rook_moves(Movelist &res, Position &p) {
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
                res.add(m);
        }

        for (int offset = 1; offset <= right_steps; offset++) {
            current_index = index + offset;
            Move m(index, current_index);
            if (p.color_table[current_index] != Color::Empty) {
                offset = right_steps + 1;
            }
            if (is_move_valid(m, p))
                res.add(m);
        }

        for (int offset = 1; offset <= up_steps; offset++) {
            current_index = index + offset * 8;
            Move m(index, current_index);
            if (p.color_table[current_index] != Color::Empty) {
                offset = up_steps + 1;
            }
            if (is_move_valid(m, p))
                res.add(m);
        }

        for (int offset = 1; offset <= down_steps; offset++) {
            current_index = index - offset * 8;
            Move m(index, current_index);
            if (p.color_table[current_index] != Color::Empty) {
                offset = down_steps + 1;
            }
            if (is_move_valid(m, p))
                res.add(m);
        }

        rooks ^= 1ULL << index;
    }
}

void generate_bishop_moves(Movelist &res, Position &p) {
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
                res.add(m);
        }

        for (int offset = 1; offset <= left_up_steps; offset++) {
            current_index = index + offset * 7;
            Move m(index, current_index);
            if (p.color_table[current_index] != Color::Empty) {
                offset = left_up_steps + 1;
            }
            if (is_move_valid(m, p))
                res.add(m);
        }

        for (int offset = 1; offset <= right_down_steps; offset++) {
            current_index = index - offset * 7;
            Move m(index, current_index);
            if (p.color_table[current_index] != Color::Empty) {
                offset = right_down_steps + 1;
            }
            if (is_move_valid(m, p))
                res.add(m);
        }

        for (int offset = 1; offset <= right_up_steps; offset++) {
            current_index = index + offset * 9;
            Move m(index, current_index);
            if (p.color_table[current_index] != Color::Empty) {
                offset = right_up_steps + 1;
            }
            if (is_move_valid(m, p))
                res.add(m);
        }

        bishops ^= 1ULL << index;
    }
}

void generate_queen_moves(Movelist &res, Position &p) {
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
                res.add(m);
        }

        for (int offset = 1; offset <= right_steps; offset++) {
            current_index = index + offset;
            Move m(index, current_index);
            if (p.color_table[current_index] != Color::Empty) {
                offset = right_steps + 1;
            }
            if (is_move_valid(m, p))
                res.add(m);
        }

        for (int offset = 1; offset <= up_steps; offset++) {
            current_index = index + offset * 8;
            Move m(index, current_index);
            if (p.color_table[current_index] != Color::Empty) {
                offset = up_steps + 1;
            }
            if (is_move_valid(m, p))
                res.add(m);
        }

        for (int offset = 1; offset <= down_steps; offset++) {
            current_index = index - offset * 8;
            Move m(index, current_index);
            if (p.color_table[current_index] != Color::Empty) {
                offset = down_steps + 1;
            }
            if (is_move_valid(m, p))
                res.add(m);
        }

        for (int offset = 1; offset <= left_down_steps; offset++) {
            current_index = index - offset * 9;
            Move m(index, current_index);
            if (p.color_table[current_index] != Color::Empty) {
                offset = left_down_steps + 1;
            }
            if (is_move_valid(m, p))
                res.add(m);
        }

        for (int offset = 1; offset <= left_up_steps; offset++) {
            current_index = index + offset * 7;
            Move m(index, current_index);
            if (p.color_table[current_index] != Color::Empty) {
                offset = left_up_steps + 1;
            }
            if (is_move_valid(m, p))
                res.add(m);
        }

        for (int offset = 1; offset <= right_down_steps; offset++) {
            current_index = index - offset * 7;
            Move m(index, current_index);
            if (p.color_table[current_index] != Color::Empty) {
                offset = right_down_steps + 1;
            }
            if (is_move_valid(m, p))
                res.add(m);
        }

        for (int offset = 1; offset <= right_up_steps; offset++) {
            current_index = index + offset * 9;
            Move m(index, current_index);
            if (p.color_table[current_index] != Color::Empty) {
                offset = right_up_steps + 1;
            }
            if (is_move_valid(m, p))
                res.add(m);
        }

        queens ^= 1ULL << index;
    }
}

void generate_king_moves(Movelist &res, Position &p) {

    Bitboard king =
        p.side_to_move == Color::White ? Bitboard(p.white_kings) : Bitboard(p.black_kings);

    Square from = king.msb();
    Bitboard possible_squares = Bitboard(king_masks[from.value()]);

    while (possible_squares) {
        Square to = possible_squares.msb_pop();

        if (p.color_table[to.value()] != p.side_to_move) {
            Move m(from, to);
            res.add(m);
        }
    }

    if (p.side_to_move == Color::White) {
        if (from != Square::Value::E1)
            return;
        if (p.white_kingside_castling_right) {
            bool can_castle_kingside = p.piece_table[5] == Piece::Empty &&
                p.piece_table[6] == Piece::Empty &&
                p.piece_table[7] == Piece::Rook &&
                p.color_table[7] == p.side_to_move;

            if (can_castle_kingside) {
                Move m(4, 5);
                auto log = p.make_move(m);
                can_castle_kingside &= p.position_is_legal();
                p.unmake_move(log);

                m.to = 6;
                log = p.make_move(m);
                can_castle_kingside &= p.position_is_legal();
                p.unmake_move(log);
            }

            if (can_castle_kingside) {
                Move m(4, 6);
                if (is_move_valid(m, p))
                    res.add(m);
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
                auto log = p.make_move(m);
                can_castle_queenside &= p.position_is_legal();
                p.unmake_move(log);

                m.to = 2;
                log = p.make_move(m);
                can_castle_queenside &= p.position_is_legal();
                p.unmake_move(log);
            }

            if (can_castle_queenside) {
                Move m(4, 2);
                if (is_move_valid(m, p))
                    res.add(m);
            }
        }
    } else {
        if (p.piece_table[60] != Piece::King)
            return;
        if (p.black_kingside_castling_right) {
            bool can_castle_kingside = p.piece_table[61] == Piece::Empty &&
                p.piece_table[62] == Piece::Empty &&
                p.piece_table[63] == Piece::Rook &&
                p.color_table[63] == p.side_to_move;

            if (can_castle_kingside) {
                Move m(60, 61);
                auto log = p.make_move(m);
                can_castle_kingside &= p.position_is_legal();
                p.unmake_move(log);

                m.to = 62;
                log = p.make_move(m);
                can_castle_kingside &= p.position_is_legal();
                p.unmake_move(log);
            }

            if (can_castle_kingside) {
                Move m(60, 62);
                if (is_move_valid(m, p))
                    res.add(m);
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
                auto log = p.make_move(m);
                can_castle_queenside &= p.position_is_legal();
                p.unmake_move(log);

                m.to = 58;
                log = p.make_move(m);
                can_castle_queenside &= p.position_is_legal();
                p.unmake_move(log);
            }

            if (can_castle_queenside) {
                Move m(60, 58);
                if (is_move_valid(m, p))
                    res.add(m);
            }
        }
    }
}

Movelist generate_moves(Position &p) {
    Movelist res(&p);

    generate_pawn_moves(res, p);
    generate_knight_moves(res, p);
    generate_rook_moves(res, p);
    generate_bishop_moves(res, p);
    generate_queen_moves(res, p);
    generate_king_moves(res, p);

    return res;
}

Movelist generate_captures(Position &p) {
    Movelist moves = generate_moves(p);
    Movelist res(&p);

    for (auto &m : moves)
        if (p.piece_table[m.to] != Piece::Empty)
            res.add(m);

    return res;
}
