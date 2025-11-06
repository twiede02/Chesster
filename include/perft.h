#pragma once

#include <cstdint>
#include <iostream>

#include "board.h"

struct Move;
struct Position;

struct PerftResults {
    uint64_t number_of_nodes = 0;
    uint64_t number_of_captures = 0;
    uint64_t number_of_checks = 0;
    uint64_t number_of_checkmates = 0;
    uint64_t number_of_en_passent = 0;
    uint64_t number_of_promotions = 0;
    uint64_t number_of_castles = 0;

    bool operator==(const PerftResults &other) const {
        return (number_of_nodes == other.number_of_nodes &&
                number_of_captures == other.number_of_captures &&
                number_of_checks == other.number_of_checks &&
                number_of_checkmates == other.number_of_checkmates &&
                number_of_en_passent == other.number_of_en_passent &&
                number_of_promotions == other.number_of_promotions &&
                number_of_castles == other.number_of_castles);
    }

    void print() const {
        std::cout << "\nNodes: " << number_of_nodes;
        std::cout << "\nCaptures: " << number_of_captures;
        std::cout << "\nEn Passent: " << number_of_en_passent;
        std::cout << "\nCastles: " << number_of_castles;
        std::cout << "\nPromotions: " << number_of_promotions;
        std::cout << "\nChecks: " << number_of_checks;
        std::cout << "\nCheckmates: " << number_of_checkmates;
        std::cout << "\n";
    }
};

class Perft {
    public:
        Perft(Position pos);

        Position p;
        PerftResults res;

        uint64_t run_fast(int depth);

        uint64_t run_debug(int depth);

        PerftResults run(int depth);

        uint64_t run_wrapped(int depth);

        void run_up_to(int depth, Position p);
};
