//
// Created by Benjamin Lee on 10/13/23.
//

#pragma once

#ifndef OTHELLO_BIT_H
#define OTHELLO_BIT_H

#pragma omp declare simd

#include <cstdint>
#include <array>
#include "Engine/Masks.h"
#include "Const.h"

namespace bit {
    // helper arrays for diagonal manipulation

    constexpr uint64_t D7_TO_H_MASKS[15] = {
            0ULL, 0ULL, 0x0000000000010204ULL, 0x0000000001020408ULL,
            0x0000000102040810ULL, 0x0000010204081020ULL, 0x0001020408102040ULL, 0x0102040810204080ULL,
            0x0204081020408000ULL, 0x0408102040800000ULL, 0x0810204080000000ULL, 0x1020408000000000ULL,
            0x2040800000000000ULL, 0ULL, 0ULL
    };

    constexpr uint8_t D7_TO_H_LEFTSHIFT[15] = {
            0, 0, 5, 4,
            3, 2, 1, 0,
            0, 0, 0, 0,
            0, 0, 0
    };

    constexpr uint8_t D7_TO_H_RIGHTSHIFT[15] = {
            0, 0, 0, 0,
            0, 0, 0, 0,
            8, 16, 24, 32,
            40, 0, 0
    };

    constexpr uint64_t D9_TO_H_MASKS[15] = {
            0ULL, 0ULL, 0x0402010000000000ULL, 0x0804020100000000ULL,
            0x1008040201000000ULL, 0x2010080402010000ULL, 0x4020100804020100ULL, 0x8040201008040201ULL,
            0x0080402010080402ULL, 0x0000804020100804ULL, 0x0000008040201008ULL, 0x0000000080402010ULL,
            0x0000000000804020ULL, 0ULL, 0ULL
    };

    constexpr uint8_t D9_TO_H_RIGHTSHIFT[15] = {
            0, 0, 40, 32,
            24, 16, 8, 0,
            1, 2, 3, 4,
            5, 0, 0
    };

    /* Reflect a bitboard across the diagonal-7 line.
     *
     * @param x: a bitboard
     *
     * Borrowed from Nyanyan's othello engine "Egaroucid"
     * https://github.com/Nyanyan/Egaroucid/blob/main/src/engine/bit_generic.hpp
     * Original code by Nyanyan, modified by Benjamin Lee
     */
    inline uint64_t mirror_d7(uint64_t x){
        uint64_t t;
        t = 0x0f0f0f0f00000000 & (x ^ (x << 28));
        x ^=       t ^ (t >> 28) ;
        t  = 0x3333000033330000 & (x ^ (x << 14));
        x ^=       t ^ (t >> 14) ;
        t  = 0x5500550055005500 & (x ^ (x <<  7));
        x ^=       t ^ (t >>  7) ;
        return x;
    }

    /* Reflect a bitboard across the diagonal-9 line.
     *
     * @param x: a bitboard
     */
    inline uint64_t mirror_d9(uint64_t x){
        uint64_t t;
        t  = x ^ (x << 36) ;
        x ^= 0xf0f0f0f00f0f0f0f & (t ^ (x >> 36));
        t  = 0xcccc0000cccc0000 & (x ^ (x << 18));
        x ^= t ^ (t >> 18) ;
        t  = 0xaa00aa00aa00aa00 & (x ^ (x <<  9));
        x ^= t ^ (t >>  9) ;
        return x;
    }

    /* Mirror a bitboard horizontally
     *
     * @param x: a bitboard
     *
     * Code adapted from chess programming wiki
     * https://www.chessprogramming.org/Flipping_Mirroring_and_Rotating
     */
    inline uint64_t mirror_horizontal(uint64_t x) {
        x = ((x >> 1) & 0x5555555555555555ULL) | ((x & 0x5555555555555555ULL) << 1);
        x = ((x >> 2) & 0x3333333333333333ULL) | ((x & 0x3333333333333333ULL) << 2);
        x = ((x >> 4) & 0x0f0f0f0f0f0f0f0fULL) | ((x & 0x0f0f0f0f0f0f0f0fULL) << 4);
        return x;
    }

    /* Mirror a bitboard vertically
     *
     * @param x: a bitboard
     *
     * Code adapted from chess programming wiki
     * https://www.chessprogramming.org/Flipping_Mirroring_and_Rotating
     */
    inline uint64_t mirror_vertical(uint64_t x) {
        x = ((x >>  8) & 0x00FF00FF00FF00FFULL) | ((x & 0x00FF00FF00FF00FFULL) <<  8);
        x = ((x >> 16) & 0x0000FFFF0000FFFFULL) | ((x & 0x0000FFFF0000FFFFULL) << 16);
        x = ( x >> 32)                          | ( x                          << 32);
        return x;
    }

    /* rotate bitboard by 180 degrees.
     *
     * @param x: a bitboard
     *
     * Code adapted from chess programming wiki
     * https://www.chessprogramming.org/Flipping_Mirroring_and_Rotating
     */
    inline uint64_t rotate_180(uint64_t x) {
        x = ((x >>  1) & 0x5555555555555555ULL) | ((x & 0x5555555555555555ULL) <<  1);
        x = ((x >>  2) & 0x3333333333333333ULL) | ((x & 0x3333333333333333ULL) <<  2);
        x = ((x >>  4) & 0x0F0F0F0F0F0F0F0FULL) | ((x & 0x0F0F0F0F0F0F0F0FULL) <<  4);
        x = ((x >>  8) & 0x00FF00FF00FF00FFULL) | ((x & 0x00FF00FF00FF00FFULL) <<  8);
        x = ((x >> 16) & 0x0000FFFF0000FFFFULL) | ((x & 0x0000FFFF0000FFFFULL) << 16);
        x = ( x >> 32)                          | ( x                          << 32);
        return x;
    }

    /* rotate bitboard by 90 degrees clockwise.
     *
     * @param x: a bitboard
     */
    inline uint64_t rotate_90(uint64_t x) {
        return mirror_d7(mirror_vertical(x));
    }

    /* rotate bitboard by 270 degrees clockwise.
     *
     * @param x: a bitboard
     */
    inline uint64_t rotate_270(uint64_t x) {
        return mirror_vertical(mirror_d7(x));
    }

    /* Shift an 8-bit horizontal bitboard to a given row.
     *
     * @param x: a bitboard.
     * @param r: the row
     *
     * Borrowed from Nyanyan's othello engine "Egaroucid"
     * https://github.com/Nyanyan/Egaroucid/blob/main/src/engine/bit_generic.hpp
     * Original code by Nyanyan, modified by Benjamin Lee
     */
    inline uint64_t h8_to_h(uint8_t x, int r) {
        return (uint64_t)x << (r << 3);
    }

    /* Shift a 64-bit horizontal bitboard of a given row to an 8-bit horizontal bitboard
     *
     * @param x: a bitboard.
     * @param r: the row
     *
     * Borrowed from Nyanyan's othello engine "Egaroucid"
     * https://github.com/Nyanyan/Egaroucid/blob/main/src/engine/bit_generic.hpp
     * Original code by Nyanyan, modified by Benjamin Lee
     */
    inline uint8_t h_to_h8(uint64_t x, int r) {
        return (uint8_t)(x >> (r << 3));
    }

    /* Map an 8-bit horizontal bitboard to a vertical bitboard on a given column.
     *
     * @param x: a bitboard.
     * @param c: the column
     *
     * Borrowed from Nyanyan's othello engine "Egaroucid"
     * https://github.com/Nyanyan/Egaroucid/blob/main/src/engine/bit_generic.hpp
     * Original code by Nyanyan, modified by Benjamin Lee
     */
    inline uint64_t h8_to_v(uint8_t x, int c) {
        uint64_t res = ((uint64_t)x * 0x0002040810204081ULL) & 0x0101010101010101ULL;
        return res << c;
    }

    /* Map a vertical bitboard on a given column to an 8-bit horizontal bitboard.
     *
     * @param x: a bitboard.
     * @param c: the column
     *
     * Borrowed from Nyanyan's othello engine "Egaroucid"
     * https://github.com/Nyanyan/Egaroucid/blob/main/src/engine/bit_generic.hpp
     * Original code by Nyanyan, modified by Benjamin Lee
     */
    inline uint8_t v_to_h8(uint64_t x, int c) {
        x = (x >> c) & 0x0101010101010101ULL;
        return (x * 0x0102040810204080ULL) >> 56;
    }

    /* Map a diagonal-7 bitboard to an 8-bit horizontal bitboard.
     *
     * @param x: a bitboard.
     * @param t: the diagonal that the bitboard lies on
     *
     * Borrowed from Nyanyan's othello engine "Egaroucid"
     * https://github.com/Nyanyan/Egaroucid/blob/main/src/engine/bit_generic.hpp
     * Original code by Nyanyan, modified by Benjamin Lee
     */
    inline uint8_t d7_to_h8(uint64_t x, int t) {
        x = (x & D7_TO_H_MASKS[t]);
        x <<= D7_TO_H_LEFTSHIFT[t];
        x >>= D7_TO_H_RIGHTSHIFT[t];
        uint64_t res = ((x * 0x0002082080000000ULL) & 0x0F00000000000000ULL) | ((x * 0x0000000002082080ULL) & 0xF000000000000000ULL);
        return res >> 56;
    }

    /* Map an 8-bit horizontal bitboard to a diagonal-7 bitboard.
     *
     * @param x: a bitboard.
     * @param t: the diagonal that the bitboard lies on
     *
     * Borrowed from Nyanyan's othello engine "Egaroucid"
     * https://github.com/Nyanyan/Egaroucid/blob/main/src/engine/bit_generic.hpp
     * Original code by Nyanyan, modified by Benjamin Lee
     */
    inline uint64_t h8_to_d7(uint8_t x, int t) {
        uint64_t res = ((uint64_t)(x & 0b00001111) * 0x0000000002082080ULL) & 0x0000000010204080ULL;
        res |= ((uint64_t)(x & 0b11110000) * 0x0002082080000000ULL) & 0x0102040800000000ULL;
        res >>= D7_TO_H_LEFTSHIFT[t];
        res <<= D7_TO_H_RIGHTSHIFT[t];
        return res;
    }


    /* Map a diagonal-9 bitboard to an 8-bit horizontal bitboard.
     *
     * @param x: a bitboard.
     * @param t: the diagonal that the bitboard lies on
     *
     * Borrowed from Nyanyan's othello engine "Egaroucid"
     * https://github.com/Nyanyan/Egaroucid/blob/main/src/engine/bit_generic.hpp
     * Original code by Nyanyan, modified by Benjamin Lee
     */
    inline uint8_t d9_to_h8(uint64_t x, int t) {
        x = x & D9_TO_H_MASKS[t];
        x >>= D9_TO_H_RIGHTSHIFT[t];
        return (x * 0x0101010101010101ULL) >> 56;
    }

    /* Map an 8-bit horizontal bitboard to a diagonal-9 bitboard.
     *
     * @param x: a bitboard.
     * @param t: the diagonal that the bitboard lies on
     *
     * Borrowed from Nyanyan's othello engine "Egaroucid"
     * https://github.com/Nyanyan/Egaroucid/blob/main/src/engine/bit_generic.hpp
     * Original code by Nyanyan, modified by Benjamin Lee
     */
    inline uint64_t h8_to_d9(uint8_t x, int t) {
        uint64_t res = ((uint64_t)x * 0x0101010101010101ULL) & 0x8040201008040201ULL;
        res <<= D9_TO_H_RIGHTSHIFT[t];
        return res;
    }

    /* Convert a coordinate to a bitboard.
     *
     * @param coord: a coordinate from 0-63
     */
    inline uint64_t coord_to_bitboard(uint8_t coord) {
        return 1ULL << coord;
    }

    /* Convert a bitboard to a coordinate
     *
     * @param x: a bitboard
     */
    inline uint8_t bitboard_to_coord(uint64_t x) {
        return (uint8_t)__builtin_ctzll(x);
    }

    /* Get the least significant bit (LSB) in a bitboard.
     *
     * @param x: a bitboard
     */
    inline uint64_t lsb(uint64_t x) {
        return x & -x;
    }

    /* Unset the LSB of a bitboard and return the modified bitboard's new LSB.
     *
     * @param x: a bitboard
     */
    inline uint64_t next_set_bit(uint64_t& x) {
        x &= x - 1;
        return x & -x;
    }

    /* Unset the LSB of a bitboard and return the modified bitboard's new LSB.
     *
     * @param x: a bitboard
     */
    inline uint64_t next_set_bit(uint64_t* x) {
        *x &= *x - 1;
        return *x & -*x;
    }

    /* Get the index of the least significant bit (LSB) in a bitboard.
     *
     * @param x: a bitboard
     */
    inline uint64_t first_set_idx(uint64_t x) {
        return __builtin_ctzll(x);
    }


    /* Unset the LSB of a bitboard and return the index of the modified bitboard's new LSB.
     *
     * @param x: a bitboard
     */
    inline uint64_t next_set_idx(uint64_t& x) {
        x &= x - 1;
        return __builtin_ctzll(x);
    }

    /* Unset the LSB of a bitboard and return the index of the modified bitboard's new LSB.
     *
     * @param x: a bitboard
     */
    inline uint64_t next_set_idx(uint64_t* x) {
        *x &= *x - 1;
        return __builtin_ctzll(*x);
    }


}

#endif //OTHELLO_BIT_H
