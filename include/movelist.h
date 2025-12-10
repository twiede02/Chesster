#pragma once

#include "move.h"
#include "board.h"

class Movelist {
   public:
    Movelist(Position* p) : p_(p) {}

    Move* begin() { return &moves_[0]; }
    const Move* begin() const { return &moves_[0]; }

    Move* end() { return &moves_[size_]; }
    const Move* end() const { return &moves_[size_]; }

    void add (Move& m) {
        if (!is_move_valid(m, *p_))
            return;
        moves_[size_] = m;
        size_++;
    }

    int size() const { return size_; }

    void clear() { size_ = 0; }

   private:
    Move moves_[256];
    int size_ = 0;
    Position* p_;
};


