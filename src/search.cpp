#include "search.h"
#include "board.h"
#include "evaluate.h"
#include "movegen.h"
#include "utils.h"
#include <algorithm>
#include <complex>
#include <optional>

std::vector<Move> order_moves(Position &p, std::vector<Move> moves) {
    std::vector<Move> checks;
    std::vector<Move> captures;
    std::vector<Move> no_captures;
    for (auto &m : moves) {
        p.make_move(m);
        if (p.is_check()) {
            checks.push_back(m);
            p.unmake_move();
            continue;
        }
        p.unmake_move();
        if (p.piece_table[m.to] != Piece::Empty)
            captures.push_back(m);
        else
            no_captures.push_back(m);
    }

    captures.insert(captures.end(), no_captures.begin(), no_captures.end());
    checks.insert(checks.end(), captures.begin(), captures.end());
    return checks;
}

std::optional<int> search_captures(
        Position &p, int alpha, int beta,
        std::chrono::time_point<std::chrono::high_resolution_clock> deadline) {

    if (std::chrono::high_resolution_clock::now() > deadline) {
        return std::nullopt;
    }

    int evaluation = evaluate(p);
    if (evaluation >= beta)
        return beta;
    alpha = std::max(evaluation, alpha);

    // generate captures
    std::vector<Move> moves = generate_captures(p);
    if (moves.size() == 0) {
        // basicly checks for mate
        if (p.position_is_legal())
            return -2000000000;
        return 0;
    }
    for (auto &m : moves) {
        p.make_move(m);
        auto search_res = search_captures(p, -beta, -alpha, deadline);
        if (!search_res)
            return std::nullopt;
        evaluation = -*search_res;
        p.unmake_move();

        if (evaluation >= beta)
            break;

        alpha = std::max(evaluation, alpha);
    }
    return alpha;
}

std::optional<int>
minimax(Position &p, int depth, Move &best_move, int starting_depth, int alpha,
        int beta,
        std::chrono::time_point<std::chrono::high_resolution_clock> deadline) {

    if (std::chrono::high_resolution_clock::now() > deadline) {
        return std::nullopt;
    }

    if (depth == 0)
        return search_captures(p, alpha, beta, deadline);

    int max = alpha;

    std::vector<Move> moves = generate_moves(p);
    moves = order_moves(p, moves);
    if (moves.size() == 0) {
        // basicly checks for mate
        if (p.position_is_legal())
            return -2000000000;
        return 0;
    }
    for (auto &m : moves) {
        p.make_move(m);
        auto minimax_res =
            minimax(p, depth - 1, best_move, starting_depth, -beta, -max, deadline);

        if (!minimax_res)
            return std::nullopt;
        int score = -*minimax_res;
        p.unmake_move();

        if (score > max) {
            max = score;
            if (depth == starting_depth) {
                best_move.from = m.from;
                best_move.to = m.to;
                best_move.promotion = m.promotion;
            }
            if (max >= beta)
                break;
        }
    }
    return max;
}

std::optional<Move>
search(Position &p, int depth,
        std::chrono::time_point<std::chrono::high_resolution_clock> deadline) {
    Move best_move(0, 0);
    auto minimax_res =
        minimax(p, depth, best_move, depth, -2000000001, 2000000001, deadline);
    if (!minimax_res)
        return std::nullopt;

    std::cout << "info depth " << depth << " score " << *minimax_res << std::endl;
    return best_move;
}
