//
// Created by Benjamin Lee on 10/2/23.
//

#include "Board.h"
#include "arm_neon.h"
#include <array>
#include <iostream>
#include <random>

uint_fast8_t Board::FLIP[256][256][8] {0};
int Board::N_FLIPPED[256][8] {0};
auto init = Board::init_flip(); // initialize the flip tables

int Board::init_flip() {
    for (int x = 0; x < 8; ++x) {
        // outflank
        for (uint O = 0; O < 256; ++O) {
            uint8_t X, L, R;
            X = 1 << x;
            L = X, R = X;

            while (O & (L <<= 1));
            while (O & (R >>= 1));

            const uint8_t outflank = L | R;
            for (uint P = 0; P < 256; ++P) {
                if (P & O || (P | O) & X) continue;
                uint8_t maskedOutflank = outflank & P;

                uint8_t remove, flip;

                flip = maskedOutflank | X;

                remove = flip | ( (flip & -flip) - 1 );
                flip |= flip >> 1;
                flip |= flip >> 2;
                flip |= flip >> 4;
                flip &= ~remove;
                Board::FLIP[P][O][x] = flip;

                // check if X is the only empty square
                if (P | O | X == 0xFF)
                    Board::N_FLIPPED[P][x] = __builtin_popcount(flip);
            }
        }
    }
    return 0;
}

uint64_t Board::get_legal_moves() const {
    if (!this->P) return 0ULL;

    #if (USE_SIMD)
        // since ARM doesn't support 256 bit registers, we will do left and right shifts in parallel
        int64x2_t shift, shift2;
        uint64x2_t pre, flip, moves, PP, OO, mOO;

        PP = vdupq_n_u64(P);
        OO = vdupq_n_u64(O);
        mOO = vandq_u64(OO, vdupq_n_u64(0x7e7e7e7e7e7e7e7eULL));

        // shift 1
        shift = {1, -1};
        flip = vandq_u64(mOO, vshlq_u64(PP, shift));
        flip = vorrq_u64(flip, vandq_u64(mOO, vshlq_u64(flip, shift)));
        pre = vandq_u64(mOO, vshlq_u64(mOO, shift));
        shift2 = {2, -2};
        flip = vorrq_u64(flip, vandq_u64(pre, vshlq_u64(flip, shift2)));
        flip = vorrq_u64(flip, vandq_u64(pre, vshlq_u64(flip, shift2)));
        moves = vshlq_u64(flip, shift);

        // shift 7
        shift = {7, -7};
        flip = vandq_u64(mOO, vshlq_u64(PP, shift));
        flip = vorrq_u64(flip, vandq_u64(mOO, vshlq_u64(flip, shift)));
        pre = vandq_u64(mOO, vshlq_u64(mOO, shift));
        shift2 = {14, -14};
        flip = vorrq_u64(flip, vandq_u64(pre, vshlq_u64(flip, shift2)));
        flip = vorrq_u64(flip, vandq_u64(pre, vshlq_u64(flip, shift2)));
        moves = vorrq_u64(moves, vshlq_u64(flip, shift));

        // shift 8
        shift = {8, -8};
        flip = vandq_u64(OO, vshlq_u64(PP, shift));
        flip = vorrq_u64(flip, vandq_u64(OO, vshlq_u64(flip, shift)));
        pre = vandq_u64(OO, vshlq_u64(OO, shift));
        shift2 = {16, -16};
        flip = vorrq_u64(flip, vandq_u64(pre, vshlq_u64(flip, shift2)));
        flip = vorrq_u64(flip, vandq_u64(pre, vshlq_u64(flip, shift2)));
        moves = vorrq_u64(moves, vshlq_u64(flip, shift));

        // shift 9
        shift = {9, -9};
        flip = vandq_u64(mOO, vshlq_u64(PP, shift));
        flip = vorrq_u64(flip, vandq_u64(mOO, vshlq_u64(flip, shift)));
        pre = vandq_u64(mOO, vshlq_u64(mOO, shift));
        shift2 = {18, -18};
        flip = vorrq_u64(flip, vandq_u64(pre, vshlq_u64(flip, shift2)));
        flip = vorrq_u64(flip, vandq_u64(pre, vshlq_u64(flip, shift2)));
        moves = vorrq_u64(moves, vshlq_u64(flip, shift));

        return (vgetq_lane_u64(moves, 0) | vgetq_lane_u64(moves, 1)) & ~(P | O);

    #else
        // original code from https://github.com/Nyanyan/Egaroucid/blob/main/src/engine/mobility_generic.hpp
        uint64_t flip, pre, mO = O & 0x7e7e7e7e7e7e7e7eULL;
        auto legal = 0ULL;

        // shift left 1
        flip = mO & (P << 1);       // get current player's pieces next to opponent pieces
        flip |= mO & (flip << 1);   // get current player's pieces next to second consecutive opponent piece
        pre = mO & (mO << 1);       // find where two consecutive opponent pieces are next to each other
        flip |= pre & (flip << 2);  // extend again (3-4 in a row)
        flip |= pre & (flip << 2);  // extend again (5-6 in a row)
        legal |= flip << 1;         // turn potential flips into legal moves

        // shift right 1
        flip = mO & (P >> 1);
        flip |= mO & (flip >> 1);
        pre >>= 1;
        flip |= pre & (flip >> 2);
        flip |= pre & (flip >> 2);
        legal |= flip >> 1;

        // shift 7
        flip = mO & (P << 7);
        flip |= mO & (flip << 7);
        pre = mO & (mO << 7);
        flip |= pre & (flip << 14);
        flip |= pre & (flip << 14);
        legal |= flip << 7;

        flip = mO & (P >> 7);
        flip |= mO & (flip >> 7);
        pre >>= 7;
        flip |= pre & (flip >> 14);
        flip |= pre & (flip >> 14);
        legal |= flip >> 7;

        // shift 8
        flip = mO & (P << 8);
        flip |= mO & (flip << 8);
        pre = mO & (mO << 8);
        flip |= pre & (flip << 16);
        flip |= pre & (flip << 16);
        legal |= flip << 8;

        flip = mO & (P >> 8);
        flip |= mO & (flip >> 8);
        pre >>= 8;
        flip |= pre & (flip >> 16);
        flip |= pre & (flip >> 16);
        legal |= flip >> 8;

        // shift 9
        flip = mO & (P << 9);
        flip |= mO & (flip << 9);
        pre = mO & (mO << 9);
        flip |= pre & (flip << 18);
        flip |= pre & (flip << 18);
        legal |= flip << 7;

        flip = mO & (P >> 9);
        flip |= mO & (flip >> 9);
        pre >>= 9;
        flip |= pre & (flip >> 18);
        flip |= pre & (flip >> 18);
        legal |= flip >> 9;

        // remove moves on occupied squares
        return legal & ~(P | O);
    #endif
}

void Board::print(bool isBlackPlayer) const {
    std::cout << this->to_string(isBlackPlayer);
}

void Board::print(bool isBlackPlayer, uint64_t highlight) const {
    std::cout << this->to_string(isBlackPlayer, highlight);
}

std::string Board::to_string(bool isBlackPlayer) const {
    return this->to_string(isBlackPlayer, this->get_legal_moves());
}

std::string Board::to_string(bool isBlackPlayer, uint64_t highlight) const {
    std::string str = "\n";

    const std::string BG1 = "\033[48;2;46;174;82m";
    const std::string BG2 = "\033[48;2;40;165;75m";
    const std::string HIGHLIGHT = "\033[48;2;71;199;107m";
    const std::string BLACK_COLOR = "\033[38;2;20;20;20m";
    const std::string WHITE_COLOR = "\033[38;2;255;255;255m";
    const std::string RESET = "\033[0m\033";
    const std::string TEXT = isBlackPlayer ? WHITE_COLOR : BLACK_COLOR;
    const std::string BG = isBlackPlayer ? "\033[48;2;75;75;75m" : "\033[48;2;192;192;192m";
    const std::string DEFAULT = BG + TEXT;
    const std::string newline = DEFAULT + "  " + RESET + '\n' + DEFAULT + ' ';
    str += DEFAULT + "    A  B  C  D  E  F  G  H ";
    int rank = 1;
    bool bg = false;

    auto black = isBlackPlayer ? this->P : this->O;
    auto white = isBlackPlayer ? this->O : this->P;
    auto blackScore = isBlackPlayer ? this->get_player_score() : this->get_opponent_score();
    auto whiteScore = isBlackPlayer ? this->get_opponent_score() : this->get_player_score();

    for (auto mask = 1ULL; mask; mask <<= 1) {
        if (mask & 0x0101010101010101ULL) {
            str += newline + std::to_string(rank++) + ' ';
            bg = !bg;
        }

        str += (mask & highlight) ? HIGHLIGHT : bg ? BG1 : BG2;
        if (mask & black)
            str += BLACK_COLOR + " ● ";
        else if (mask & white)
            str += WHITE_COLOR + " ● ";
        else str += "   ";

        bg = !bg;
    }
    str += newline + (isBlackPlayer ? "        Black's Turn      " : "        White's Turn      ");
    str += newline + "    " + BLACK_COLOR + (blackScore < 10 ? " ●  " : " ● ") + std::to_string(blackScore);
    str += "         " + WHITE_COLOR + (whiteScore < 10 ? " ●  " : " ● ") + std::to_string(whiteScore) + "   ";
    return str + newline + "                            " + RESET + '\n';
}