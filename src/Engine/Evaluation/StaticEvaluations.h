//
// Created by Benjamin Lee on 1/26/24.
//
#pragma once
#ifndef OTHELLO_STATICEVALUATIONS_H
#define OTHELLO_STATICEVALUATIONS_H

#include "../../Bit.h"

namespace engine::eval {
    /* Compute the potential mobility of a position.
     * @param P: current player bitboard
     * @param O: opponent's bitboard
     * @return the number of empty squares that surround the opponent's discs.
     *
     * Code borrowed from Nyanyan's Egaroucid.
     */
    inline int get_potential_mobility(uint64_t P, uint64_t O) {
        const auto hMask = O & 0x7E7E7E7E7E7E7E7EULL;
        const auto vMask = O & 0x00FFFFFFFFFFFF00ULL;
        const auto hvMask = O & 0x007E7E7E7E7E7E00ULL;
        const auto adjMask =
                (hMask << 1) | (hMask >> 1) |
                (vMask << 8) | (vMask >> 8) |
                (hvMask << 7) | (hvMask >> 7) |
                (hvMask << 9) | (hvMask >> 9);
        return __builtin_popcountll(~(O | P) & adjMask);
    }

    /* Compute the number of legal corner moves.
     * @param P: current player bitboard
     * @param O: opponent's bitboard
     * @return the number of legal corner moves in a given position
     *
     * Code borrowed from Nyanyan's Egaroucid.
     */
    inline int get_corner_mobility(uint64_t legalMask) {
        int res = (int) ((legalMask & 0b10000001ULL) + ((legalMask >> 56) & 0b10000001ULL));
        return (res & 0b11) + (res >> 7);
    }

    /* Compute the weighted mobility of a position. Corner moves are weighted more heavily than the rest.
     * @param P: current player bitboard
     * @param O: opponent's bitboard
     * @return the position's weighted mobility.
     *
     * Code borrowed from Nyanyan's Egaroucid.
     */
    inline int get_weighted_mobility(uint64_t legalMask) {
        return __builtin_popcountll(legalMask) * 2 + get_corner_mobility(legalMask);
    }



    const constexpr int CELL_WEIGHTS[64] = {
            18, 4, 16, 12, 12, 16, 4, 18,
            4, 2, 6, 8, 8, 6, 2, 4,
            16, 6, 14, 10, 10, 14, 6, 16,
            12, 8, 10, 0, 0, 10, 8, 12,
            12, 8, 10, 0, 0, 10, 8, 12,
            16, 6, 14, 10, 10, 14, 6, 16,
            4, 2, 6, 8, 8, 6, 2, 4,
            18, 4, 16, 12, 12, 16, 4, 18
    };

    const constexpr uint64_t PARITY_BITS[64] = {
            1, 1, 1, 1, 2, 2, 2, 2,
            1, 1, 1, 1, 2, 2, 2, 2,
            1, 1, 1, 1, 2, 2, 2, 2,
            1, 1, 1, 1, 2, 2, 2, 2,
            4, 4, 4, 4, 8, 8, 8, 8,
            4, 4, 4, 4, 8, 8, 8, 8,
            4, 4, 4, 4, 8, 8, 8, 8,
            4, 4, 4, 4, 8, 8, 8, 8
    };

    const constexpr int PARITY_INDICES[64] = {
            1, 1, 1, 1, 2, 2, 2, 2,
            1, 1, 1, 1, 2, 2, 2, 2,
            1, 1, 1, 1, 2, 2, 2, 2,
            1, 1, 1, 1, 2, 2, 2, 2,
            3, 3, 3, 3, 4, 4, 4, 4,
            3, 3, 3, 3, 4, 4, 4, 4,
            3, 3, 3, 3, 4, 4, 4, 4,
            3, 3, 3, 3, 4, 4, 4, 4
    };

    /*  0, TL, TR,   T,
     * BL, BR,  L, ~BL,
     * BR,
     *  */
    const constexpr uint64_t PARITY_REGIONS[16] = {
            0x0000000000000000ULL, 0x000000000F0F0F0FULL, 0x00000000F0F0F0F0ULL, 0x00000000FFFFFFFFULL,
            0x0F0F0F0F00000000ULL, 0x0F0F0F0F0F0F0F0FULL, 0x0F0F0F0FF0F0F0F0ULL, 0x0F0F0F0FFFFFFFFFULL,
            0xF0F0F0F000000000ULL, 0xF0F0F0F00F0F0F0FULL, 0xF0F0F0F0F0F0F0F0ULL, 0xF0F0F0F0FFFFFFFFULL,
            0xFFFFFFFF00000000ULL, 0xFFFFFFFF0F0F0F0FULL, 0xFFFFFFFFF0F0F0F0ULL, 0xFFFFFFFFFFFFFFFFULL
    };

    const constexpr uint64_t SURROUND_MASKS[64] = {
            0xE0A0E0ULL << 9, 0xE0A0E0ULL << 8, 0xE0A0E0ULL << 7, 0xE0A0E0ULL << 6, 0xE0A0E0ULL << 5, 0xE0A0E0ULL << 4, 0xE0A0E0ULL << 3, 0xE0A0E0ULL << 2,
            0xE0A0E0ULL << 1, 0xE0A0E0ULL, 0xE0A0E0ULL >> 1, 0xE0A0E0ULL >> 2, 0xE0A0E0ULL >> 3, 0xE0A0E0ULL >> 4, 0xE0A0E0ULL >> 5, 0xE0A0E0ULL >> 6,
            0xE0A0E0ULL >> 7, 0xE0A0E0ULL >> 8, 0xE0A0E0ULL >> 9, 0xE0A0E0ULL >> 10, 0xE0A0E0ULL >> 11, 0xE0A0E0ULL >> 12, 0xE0A0E0ULL >> 13, 0xE0A0E0ULL >> 14,
            0xE0A0E0ULL >> 15, 0xE0A0E0ULL >> 16, 0xE0A0E0ULL >> 17, 0xE0A0E0ULL >> 18, 0xE0A0E0ULL >> 19, 0xE0A0E0ULL >> 20, 0xE0A0E0ULL >> 21, 0xE0A0E0ULL >> 22,
            0xE0A0E0ULL >> 23, 0xE0A0E0ULL >> 24, 0xE0A0E0ULL >> 25, 0xE0A0E0ULL >> 26, 0xE0A0E0ULL >> 27, 0xE0A0E0ULL >> 28, 0xE0A0E0ULL >> 29, 0xE0A0E0ULL >> 30,
            0xE0A0E0ULL >> 31, 0xE0A0E0ULL >> 32, 0xE0A0E0ULL >> 33, 0xE0A0E0ULL >> 34, 0xE0A0E0ULL >> 35, 0xE0A0E0ULL >> 36, 0xE0A0E0ULL >> 37, 0xE0A0E0ULL >> 38,
            0xE0A0E0ULL >> 39, 0xE0A0E0ULL >> 40, 0xE0A0E0ULL >> 41, 0xE0A0E0ULL >> 42, 0xE0A0E0ULL >> 43, 0xE0A0E0ULL >> 44, 0xE0A0E0ULL >> 45, 0xE0A0E0ULL >> 46,
            0xE0A0E0ULL >> 47, 0xE0A0E0ULL >> 48, 0xE0A0E0ULL >> 49, 0xE0A0E0ULL >> 50, 0xE0A0E0ULL >> 51, 0xE0A0E0ULL >> 52, 0xE0A0E0ULL >> 53, 0xE0A0E0ULL >> 54,
    };


}

#endif //OTHELLO_STATICEVALUATIONS_H
