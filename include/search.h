#pragma once

#include "board.h"
#include "movelist.h"
#include <chrono>
#include <optional>

void order_moves(Position &p, Movelist& moves);

std::optional<Move>
search(Position &p, int depth,
        std::chrono::time_point<std::chrono::high_resolution_clock> deadline);
