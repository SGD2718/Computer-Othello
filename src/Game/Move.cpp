//
// Created by Benjamin Lee on 1/18/24.
//

#include "Move.h"
#include "Board.h"

Move::Move(const Board& board, uint8_t x) :
        x(x),
        flip(board.get_flipped(x)) {}

Move::Move(const Board& board, uint8_t col, uint8_t row) :
        x(col + (row << 3)),
        flip(board.get_flipped(col, row)) {}

Move::Move(const Board& board, const std::string& coordinates) :
        x(std::tolower(coordinates[0]) - 'a' + ((coordinates[1] - '1') << 3)),
        flip(board.get_flipped(coordinates)) {}

Move::Move(uint8_t x, uint64_t flip) :
    x(x),
    flip(flip) {}

Move::Move(uint8_t col, uint8_t row, uint64_t flip) :
    x(col + (row << 3)),
    flip(flip) {}

Move::Move(const std::string& coordinates, uint64_t flip) :
    x(std::tolower(coordinates[0]) - 'a' + ((coordinates[1] - '1') << 3)),
    flip(flip) {}


std::ostream& operator<<(std::ostream& os, const Move& move) {
    if (move.is_pass()) {
        os << "PASS";
        return os;
    }
    os << (char)((move.x & (uint8_t)7) + 'a') << (char)((move.x >> 3) + '1');
    return os;
}