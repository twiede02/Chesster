#include <chrono>
#include <iostream>

#include "board.h"
#include "movegen.h"
#include "perft.h"
#include "utils.h"

Perft::Perft(Position pos) { p = pos; }

uint64_t Perft::run_fast(int depth) {
    if (depth == 0) {
        return 1ULL;
    }

    std::vector<Move> move_list = generate_moves(p);
    if (depth == 1)
        return move_list.size();

    uint64_t nodes = 0;

    for (auto m : move_list) {
        p.make_move(m);
        nodes += run_fast(depth - 1);
        p.unmake_move();
    }

    return nodes;
}

uint64_t Perft::run_debug(int depth) {
    std::vector<Move> move_list = generate_moves(p);

    uint64_t nodes = 0;
    for (auto m : move_list) {
        p.make_move(m);
        uint64_t local_nodes = run_fast(depth - 1);
        nodes += local_nodes;
        print_move_compact(m);
        std::cout << ": " << local_nodes << "\n";
        p.unmake_move();
    }
    return nodes;
}

PerftResults Perft::run(int depth) {

    res = PerftResults();

    res.number_of_nodes = run_wrapped(depth);

    return res;
}

uint64_t Perft::run_wrapped(int depth) {
    std::vector<Move> move_list = generate_moves(p);

    if (depth == 0) {
        if (p.is_check()) {
            res.number_of_checks++;
        }
        if (move_list.size() == 0) {
            res.number_of_checkmates++;
        }
        return 1ULL;
    }

    uint64_t nodes = 0ULL;

    for (auto &m : move_list) {
        if (depth == 1) {
            res.number_of_captures += is_capture(p, m);

            res.number_of_en_passent += is_en_passent(p, m);

            res.number_of_promotions += m.promotion != Piece::Empty;

            res.number_of_castles += is_castle(p, m);
        }
        p.make_move(m);
        nodes += run_wrapped(depth - 1);
        p.unmake_move();
    }

    return nodes;
}

void Perft::run_up_to(int depth, Position p) {
    for (int i = 1; i <= depth; i++) {

        res = PerftResults();

        auto beg = std::chrono::high_resolution_clock::now();
        res.number_of_nodes = run_wrapped(i);
        auto end = std::chrono::high_resolution_clock::now();

        auto duration =
            std::chrono::duration_cast<std::chrono::microseconds>(end - beg);

        std::cout << "\n";
        std::cout << "Perft on depth " << i << " took " << duration.count()
            << " microseconds\n";
        std::cout << "Nodes: " << res.number_of_nodes << "\n";
        std::cout << "captures: " << res.number_of_captures << "\n";
        std::cout << "promotions: " << res.number_of_promotions << "\n";
        std::cout << "en passents: " << res.number_of_en_passent << "\n";
        std::cout << "castles: " << res.number_of_castles << "\n";
        std::cout << "checks: " << res.number_of_checks << "\n";
        std::cout << "checkmates: " << res.number_of_checkmates << "\n";
        std::cout << "\n";
    }
}
