#pragma once

#include "board.h"
#include "movelist.h"

void generate_pawn_moves(Movelist &res, Position &p);

void generate_knight_moves(Movelist &res, Position &p);

void generate_rook_moves(Movelist &res, Position &p);

void generate_bishop_moves(Movelist &res, Position &p);

void generate_queen_moves(Movelist &res, Position &p);

void generate_king_moves(Movelist &res, Position &p);

Movelist generate_moves(Position &p);

Movelist generate_captures(Position &p);

