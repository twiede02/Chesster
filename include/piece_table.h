#pragma once

#include <cassert>
#include "square.h"

template <typename T>
class PieceTable {
public:
    PieceTable() : data_{} {}

    T* begin() { return &data_[0]; }
    const T* begin() const { return &data_[0]; }

    T* end() { return &data_[Capacity]; }
    const T* end() const { return &data_[Capacity]; }

    T& operator[](Square sq) { assert(sq); return data_[sq.value()]; }
    const T& operator[](Square sq) const { assert(sq); return data_[sq.value()]; }

    constexpr static int Capacity = 64;
private:
    T data_[Capacity];
};


