//
// Created by Benjamin Lee on 1/18/24.
//

#pragma once

#ifndef OTHELLO_MOVE_H
#define OTHELLO_MOVE_H

#include <cstdint>
#include "../Const.h"

class Board;

struct Move {
    Move() = default;
    Move(const Board& board, uint_fast8_t x);
    Move(const Board& board, uint_fast8_t col, uint_fast8_t row);
    Move(const Board& board, const std::string& coordinates);
    Move(uint_fast8_t x, uint64_t flip);
    Move(uint_fast8_t col, uint_fast8_t row, uint64_t flip);
    Move(const std::string& coordinates, uint64_t flip);

    inline void init(uint_fast8_t position, uint64_t flipped) {
        this->x = position;
        this->flip = flipped;
    }

    [[nodiscard]] inline bool is_pass() const {
        return this->flip == 0;
    }

    inline bool operator==(const Move& move) const {
        return this->x == move.x && this->flip == move.flip;
    }

    inline bool operator!=(const Move& move) const {
        return this->x != move.x || this->flip != move.flip;
    }

    [[nodiscard]] inline std::string to_string() const {
        return this->x == 64 ? "pass" : std::string(1, (char)( 'a' + (this->x & (char)7) )) + std::to_string(((int)this->x >> 3) + 1);
    }
    
    static inline bool compute_flip(Move& move, const Board& board, uint_fast8_t x) {
        move = Move(board, x);
        return move.flip != 0;
    }
    
    uint_fast8_t x = 64;
    uint64_t flip = 0;

};

const Move PASS = Move(64, 0);
const Move MOVE_UNDEFINED = Move(127, 0);

std::ostream& operator<<(std::ostream& os, const Move& move);

#endif //OTHELLO_MOVE_H
