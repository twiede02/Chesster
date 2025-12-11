#pragma once

#include <cstdint>
#include <string>

class Square {
public:
    enum class Value : uint8_t {
        A1, B1, C1, D1, E1, F1, G1, H1,
        A2, B2, C2, D2, E2, F2, G2, H2,
        A3, B3, C3, D3, E3, F3, G3, H3,
        A4, B4, C4, D4, E4, F4, G4, H4,
        A5, B5, C5, D5, E5, F5, G5, H5,
        A6, B6, C6, D6, E6, F6, G6, H6,
        A7, B7, C7, D7, E7, F7, G7, H7,
        A8, B8, C8, D8, E8, F8, G8, H8,
        NO_SQR
    };

    Square() : val_(Value::NO_SQR) {}
    Square(Value val) : val_(val) {}
    explicit Square(int sq) : val_(static_cast<Value>(sq)) {}
    explicit Square(int file, int rank) : val_(static_cast<Value>(rank * 8 + file)) {}

    constexpr int file() const { return static_cast<int>(val_) % 8; }
    constexpr int rank() const { return static_cast<int>(val_) / 8; }

    constexpr int value() const { return static_cast<int>(val_); }

    Square& shift_rank_up() { val_ = static_cast<Value>(static_cast<int>(val_) + 8); return *this; }
    Square& shift_rank_down() { val_ = static_cast<Value>(static_cast<int>(val_) - 8); return *this; }

    Square& shift_file_left() { val_ = static_cast<Value>(static_cast<int>(val_) + 1); return *this; }
    Square& shift_file_right() { val_ = static_cast<Value>(static_cast<int>(val_) - 1); return *this; }

    constexpr bool operator==(const Square& rhs) const { return val_ == rhs.val_; }
    constexpr bool operator!=(const Square& rhs) const { return val_ != rhs.val_; }

    constexpr explicit operator bool() const { return val_ != Value::NO_SQR; }

    std::string to_string() const {
        if (val_ == Value::NO_SQR) return "NO_SQR";
        char f = static_cast<char>('a' + file());
        char r = static_cast<char>('1' + rank());
        return std::string{f, r};
    }

private:
    Value val_;
};


