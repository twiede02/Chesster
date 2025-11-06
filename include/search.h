#pragma once

#include "board.h"
#include <chrono>
#include <optional>
#include <vector>

std::vector<Move> order_moves(Position &p, std::vector<Move> moves);

std::optional<Move>
search(Position &p, int depth,
        std::chrono::time_point<std::chrono::high_resolution_clock> deadline);
