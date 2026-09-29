#pragma once

#include "move.h"
#include "board.h"

class Movelist {
   public:
    Movelist() {}

    Move* begin() { return &moves_[0]; }
    const Move* begin() const { return &moves_[0]; }

    Move* end() { return &moves_[size_]; }
    const Move* end() const { return &moves_[size_]; }

    void add (Move m) {
        assert_throw(size_ < 256);
        moves_[size_++] = m;
    }

    size_t size() const { return size_; }

    void clear() { size_ = 0; }

   private:
    Move moves_[256];
    size_t size_ = 0;
};


