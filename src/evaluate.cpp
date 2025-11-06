#include "evaluate.h"
#include "board.h"
#include "utils.h"

int piece_value(Piece p) {
    switch (p) {
        case Piece::Pawn:
            return 100;
        case Piece::Knight:
            return 300;
        case Piece::Bishop:
            return 300;
        case Piece::Rook:
            return 500;
        case Piece::Queen:
            return 900;
        default:
            return 0;
    }
}

int evaluate_material(Position &p) {
    int res = 0;

    for (int i = 0; i < 64; i++) {

        if (p.color_table[i] == Color::Empty)
            continue;

        if (p.color_table[i] == Color::White) {
            res += piece_value(p.piece_table[i]);
        } else {
            res -= piece_value(p.piece_table[i]);
        }
    }

    return res;
}

int evaluate(Position &p) {
    int repetitions = 0;
    for (auto current_hash : p.hash_history) {
        if (current_hash == p.hash_history.back())
            repetitions++;
        if (repetitions >= 3)
            return -1;
    }

    if (p.moves_since_panwmove_or_capture >= 100)
        return 0;

    int res = 0;

    if (p.side_to_move == Color::White)
        res += evaluate_material(p);
    else
        res -= evaluate_material(p);

    return res;
}
