#include <array>
#include <cstdint>

#include "attack_masks.h"
#include "board.h"
#include "magics.h"
#include "movegen.h"
#include "move.h"
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
    int n = res.size();
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
            Square from = promotable_pawns.msb_pop();
            Square to = from;
            to.shift_rank_up();

            if (p.color_table[to] == Color::Empty) {
                Move m;
                m = Move(from, to, MoveType::Promotion, Piece::Knight);
                res.add(m);
                m = Move(from, to, MoveType::Promotion, Piece::Rook);
                res.add(m);
                m = Move(from, to, MoveType::Promotion, Piece::Bishop);
                res.add(m);
                m = Move(from, to, MoveType::Promotion, Piece::Queen);
                res.add(m);
            }

            if (from.file() > 0) {
                Square left_to = to;
                left_to.shift_file_left();

                Move m;
                m = Move(from, left_to, MoveType::Promotion, Piece::Knight);
                res.add(m);
                m = Move(from, left_to, MoveType::Promotion, Piece::Rook);
                res.add(m);
                m = Move(from, left_to, MoveType::Promotion, Piece::Bishop);
                res.add(m);
                m = Move(from, left_to, MoveType::Promotion, Piece::Queen);
                res.add(m);
            }

            if (from.file() < 7) {
                Square right_to = to;
                right_to.shift_file_right();

                Move m;
                m = Move(from, right_to, MoveType::Promotion, Piece::Knight);
                res.add(m);
                m = Move(from, right_to, MoveType::Promotion, Piece::Rook);
                res.add(m);
                m = Move(from, right_to, MoveType::Promotion, Piece::Bishop);
                res.add(m);
                m = Move(from, right_to, MoveType::Promotion, Piece::Queen);
                res.add(m);
            }
        }

        while (left_capuring_pawns) {
            Square to = left_capuring_pawns.msb_pop();
            if (p.color_table[to] != p.side_to_move) {
                Square from = to;
                from.shift_rank_down().shift_file_right();
                Move m(from, to);
                res.add(m);
            }
        }

        while (right_capuring_pawns) {
            Square to = right_capuring_pawns.msb_pop();
            if (p.color_table[to] != p.side_to_move) {
                Square from = to;
                from.shift_rank_down().shift_file_left();
                Move m(from, to);
                res.add(m);
            }
        }

        while (double_pushed_pawns) {
            Square to = double_pushed_pawns.msb_pop();
            Square from = to;
            from.shift_rank_down().shift_rank_down();
            Move m(from, to);
            res.add(m);
        }

        while (pushed_pawns) {
            Square to = pushed_pawns.msb_pop();
            Square from = to;
            from.shift_rank_down();
            Move m(from, to);
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
            Square from = promotable_pawns.msb_pop();
            Square to = from;
            to.shift_rank_up();

            if (p.color_table[to] == Color::Empty) {
                Move m;
                m = Move(from, to, MoveType::Promotion, Piece::Knight);
                res.add(m);
                m = Move(from, to, MoveType::Promotion, Piece::Rook);
                res.add(m);
                m = Move(from, to, MoveType::Promotion, Piece::Bishop);
                res.add(m);
                m = Move(from, to, MoveType::Promotion, Piece::Queen);
                res.add(m);

            }

            if (from.file() > 0) {
                Square left_to = to;
                left_to.shift_file_left();

                Move m;
                m = Move(from, to, MoveType::Promotion, Piece::Knight);
                res.add(m);
                m = Move(from, to, MoveType::Promotion, Piece::Rook);
                res.add(m);
                m = Move(from, to, MoveType::Promotion, Piece::Bishop);
                res.add(m);
                m = Move(from, to, MoveType::Promotion, Piece::Queen);
                res.add(m);
            }

            if (from.file() < 7) {
                Square right_to = to;
                right_to.shift_file_right();

                Move m;
                m = Move(from, right_to, MoveType::Promotion, Piece::Knight);
                res.add(m);
                m = Move(from, right_to, MoveType::Promotion, Piece::Rook);
                res.add(m);
                m = Move(from, right_to, MoveType::Promotion, Piece::Bishop);
                res.add(m);
                m = Move(from, right_to, MoveType::Promotion, Piece::Queen);
                res.add(m);
            }
        }

        while (left_capuring_pawns) {
            Square to = left_capuring_pawns.msb_pop();
            if (p.color_table[to] != p.side_to_move) {
                Square from = to;
                from.shift_rank_up().shift_file_right();
                Move m(from, to);
                res.add(m);
            }
        }

        while (right_capuring_pawns) {
            Square to = right_capuring_pawns.msb_pop();
            if (p.color_table[to] != p.side_to_move) {
                Square from = to;
                from.shift_rank_up().shift_file_left();
                Move m(from, to);
                res.add(m);
            }
        }

        while (double_pushed_pawns) {
            Square to = double_pushed_pawns.msb_pop();
            Square from = to;
            from.shift_rank_up().shift_rank_up();
            Move m(from, to);
            res.add(m);
        }
        
        while (pushed_pawns) {
            Square to = pushed_pawns.msb_pop();
            Square from = to;
            from.shift_rank_up();
            Move m(from, to);
            res.add(m);
        }
    }

    std::cout << "pawn moves: " << res.size() - n << std::endl;

    // TODO: EN PASSENT
}

void generate_knight_moves(Movelist &res, Position &p) {
    Bitboard knights =
        p.side_to_move == Color::White ? Bitboard(p.white_knights) : Bitboard(p.black_knights);

    while (knights) {
        Square from = knights.msb_pop();

        Bitboard possible_squares = Bitboard(knight_masks[from.value()]);
        possible_squares.masked_by(p.friendly_pieces());

        while (possible_squares) {
            Square to = possible_squares.msb_pop();
            Move m(from, to);
            res.add(m);
        }
    }
}

void generate_rook_moves(Movelist &res, Position &p) {
    Bitboard rooks =
        p.side_to_move == Color::White ? Bitboard(p.white_rooks): Bitboard(p.black_rooks);

    while (rooks) {
        Square from = rooks.msb_pop();
        
        Bitboard blockers = Bitboard(p.occupied_squares)
            .masked_by(rook_attack_masks[from.value()]);

        Bitboard possible_squares = lookup_rook_move(from, blockers);
        possible_squares.masked_by(p.friendly_pieces());

        while (possible_squares) {
            Square to = possible_squares.msb_pop();
            Move m(from, to);
            res.add(m);
        }
    }
}

void generate_bishop_moves(Movelist &res, Position &p) {
    Bitboard bishops =
        p.side_to_move == Color::White ? p.white_bishops : p.black_bishops;

    while (bishops) {
        Square from = bishops.msb_pop();
        
        Bitboard blockers = Bitboard(p.occupied_squares)
            .masked_by(bishop_attack_masks[from.value()]);

        Bitboard possible_squares = lookup_bishop_move(from, blockers);
        possible_squares.masked_by(p.friendly_pieces());

        while (possible_squares) {
            Square to = possible_squares.msb_pop();
            Move m(from, to);
            res.add(m);
        }
    }
}

void generate_queen_moves(Movelist &res, Position &p) {
    Bitboard queens =
        p.side_to_move == Color::White ? p.white_queens : p.black_queens;

    while (queens) {
        Square from = queens.msb_pop();
        
        Bitboard rook_blockers = Bitboard(p.occupied_squares)
            .masked_by(rook_attack_masks[from.value()]);

        Bitboard possible_rook_squares = lookup_rook_move(from, rook_blockers);
        possible_rook_squares.masked_by(p.friendly_pieces());

        while (possible_rook_squares) {
            Square to = possible_rook_squares.msb_pop();
            Move m(from, to);
            res.add(m);
        }

        Bitboard bishop_blockers = Bitboard(p.occupied_squares)
            .masked_by(bishop_attack_masks[from.value()]);

        Bitboard possible_bishop_squares = lookup_bishop_move(from, bishop_blockers);
        possible_bishop_squares.masked_by(p.friendly_pieces());

        while (possible_bishop_squares) {
            Square to = possible_bishop_squares.msb_pop();
            Move m(from, to);
            res.add(m);
        }
    }
}

void generate_king_moves(Movelist &res, Position &p) {

    Bitboard king =
        p.side_to_move == Color::White ? Bitboard(p.white_kings) : Bitboard(p.black_kings);

    Square from = king.msb();
    Bitboard possible_squares = Bitboard(king_masks[from.value()]);
    possible_squares.masked_by(p.friendly_pieces());

    while (possible_squares) {
        Square to = possible_squares.msb_pop();
        Move m(from, to);
        res.add(m);
    }

    // TODO: Castling

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
        if (p.piece_table[m.to()] != Piece::Empty)
            res.add(m);

    return res;
}
