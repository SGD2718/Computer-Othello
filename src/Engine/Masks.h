//
// Created by Benjamin Lee on 10/11/23.
//

#include <cstdint>

#ifndef OTHELLO_MASKS_H
#define OTHELLO_MASKS_H

// masks for each square
constexpr uint64_t A1 = 1;
constexpr uint64_t A2 = A1 << 1;
constexpr uint64_t A3 = A1 << 2;
constexpr uint64_t A4 = A1 << 3;
constexpr uint64_t A5 = A1 << 4;
constexpr uint64_t A6 = A1 << 5;
constexpr uint64_t A7 = A1 << 6;
constexpr uint64_t A8 = A1 << 7;

constexpr uint64_t B1 = A1 << 8;
constexpr uint64_t B2 = A1 << 9;
constexpr uint64_t B3 = A1 << 10;
constexpr uint64_t B4 = A1 << 11;
constexpr uint64_t B5 = A1 << 12;
constexpr uint64_t B6 = A1 << 13;
constexpr uint64_t B7 = A1 << 14;
constexpr uint64_t B8 = A1 << 15;

constexpr uint64_t C1 = A1 << 16;
constexpr uint64_t C2 = A1 << 17;
constexpr uint64_t C3 = A1 << 18;
constexpr uint64_t C4 = A1 << 19;
constexpr uint64_t C5 = A1 << 20;
constexpr uint64_t C6 = A1 << 21;
constexpr uint64_t C7 = A1 << 22;
constexpr uint64_t C8 = A1 << 23;

constexpr uint64_t D1 = A1 << 24;
constexpr uint64_t D2 = A1 << 25;
constexpr uint64_t D3 = A1 << 26;
constexpr uint64_t D4 = A1 << 27;
constexpr uint64_t D5 = A1 << 28;
constexpr uint64_t D6 = A1 << 29;
constexpr uint64_t D7 = A1 << 30;
constexpr uint64_t D8 = A1 << 31;

constexpr uint64_t E1 = A1 << 32;
constexpr uint64_t E2 = A1 << 33;
constexpr uint64_t E3 = A1 << 34;
constexpr uint64_t E4 = A1 << 35;
constexpr uint64_t E5 = A1 << 36;
constexpr uint64_t E6 = A1 << 37;
constexpr uint64_t E7 = A1 << 38;
constexpr uint64_t E8 = A1 << 39;

constexpr uint64_t F1 = A1 << 40;
constexpr uint64_t F2 = A1 << 41;
constexpr uint64_t F3 = A1 << 42;
constexpr uint64_t F4 = A1 << 43;
constexpr uint64_t F5 = A1 << 44;
constexpr uint64_t F6 = A1 << 45;
constexpr uint64_t F7 = A1 << 46;
constexpr uint64_t F8 = A1 << 47;

constexpr uint64_t G1 = A1 << 48;
constexpr uint64_t G2 = A1 << 49;
constexpr uint64_t G3 = A1 << 50;
constexpr uint64_t G4 = A1 << 51;
constexpr uint64_t G5 = A1 << 52;
constexpr uint64_t G6 = A1 << 53;
constexpr uint64_t G7 = A1 << 54;
constexpr uint64_t G8 = A1 << 55;

constexpr uint64_t H1 = A1 << 56;
constexpr uint64_t H2 = A1 << 57;
constexpr uint64_t H3 = A1 << 58;
constexpr uint64_t H4 = A1 << 59;
//constexpr uint64_t H5 = A1 << 60;
constexpr uint64_t H6 = A1 << 61;
constexpr uint64_t H7 = A1 << 62;
constexpr uint64_t H8 = A1 << 63;

constexpr uint64_t FILE_MASK_A = 0x0101010101010101;
constexpr uint64_t FILE_MASK_B = FILE_MASK_A << 1;
constexpr uint64_t FILE_MASK_C = FILE_MASK_A << 2;
constexpr uint64_t FILE_MASK_D = FILE_MASK_A << 3;
constexpr uint64_t FILE_MASK_E = FILE_MASK_A << 4;
constexpr uint64_t FILE_MASK_F = FILE_MASK_A << 5;
constexpr uint64_t FILE_MASK_G = FILE_MASK_A << 6;
constexpr uint64_t FILE_MASK_H = FILE_MASK_A << 7;

constexpr uint64_t RANK_MASK_1 = 0x00000000000000FF;
constexpr uint64_t RANK_MASK_2 = RANK_MASK_1 << 8;
constexpr uint64_t RANK_MASK_3 = RANK_MASK_1 << 16;
constexpr uint64_t RANK_MASK_4 = RANK_MASK_1 << 24;
constexpr uint64_t RANK_MASK_5 = RANK_MASK_1 << 32;
constexpr uint64_t RANK_MASK_6 = RANK_MASK_1 << 40;
constexpr uint64_t RANK_MASK_7 = RANK_MASK_1 << 48;
constexpr uint64_t RANK_MASK_8 = RANK_MASK_1 << 56;

constexpr uint64_t HALF_MASK_U = 0x00000000FFFFFFFF;
constexpr uint64_t HALF_MASK_L = 0x0F0F0F0F0F0F0F0F;
constexpr uint64_t HALF_MASK_D = 0xFFFFFFFF00000000;
constexpr uint64_t HALF_MASK_R = 0xF0F0F0F0F0F0F0F0;

constexpr uint64_t CORNER_MASK = 0x8100000000000081;

// diagonal 7
constexpr uint64_t DIAGONAL_MASK_7 = A8 | B7 | C6 | D5 | E4 | F3 | G2 | H1;
constexpr uint64_t DIAGONAL_MASK_7L7 = DIAGONAL_MASK_7 << 8;
constexpr uint64_t DIAGONAL_MASK_7L6 = DIAGONAL_MASK_7 << 16;
constexpr uint64_t DIAGONAL_MASK_7L5 = DIAGONAL_MASK_7 << 24;
constexpr uint64_t DIAGONAL_MASK_7L4 = DIAGONAL_MASK_7 << 32;
constexpr uint64_t DIAGONAL_MASK_7L3 = DIAGONAL_MASK_7 << 40;
constexpr uint64_t DIAGONAL_MASK_7L2 = DIAGONAL_MASK_7 << 48;
constexpr uint64_t DIAGONAL_MASK_7L1 = DIAGONAL_MASK_7 << 56;
constexpr uint64_t DIAGONAL_MASK_7R7 = DIAGONAL_MASK_7 >> 8;
constexpr uint64_t DIAGONAL_MASK_7R6 = DIAGONAL_MASK_7 >> 16;
constexpr uint64_t DIAGONAL_MASK_7R5 = DIAGONAL_MASK_7 >> 24;
constexpr uint64_t DIAGONAL_MASK_7R4 = DIAGONAL_MASK_7 >> 32;
constexpr uint64_t DIAGONAL_MASK_7R3 = DIAGONAL_MASK_7 >> 40;
constexpr uint64_t DIAGONAL_MASK_7R2 = DIAGONAL_MASK_7 >> 48;
constexpr uint64_t DIAGONAL_MASK_7R1 = DIAGONAL_MASK_7 >> 56;

// diagonal 9
constexpr uint64_t DIAGONAL_MASK_9 = 0x8040201008040201;
constexpr uint64_t DIAGONAL_MASK_9L7 = DIAGONAL_MASK_9 << 8;
constexpr uint64_t DIAGONAL_MASK_9L6 = DIAGONAL_MASK_9 << 16;
constexpr uint64_t DIAGONAL_MASK_9L5 = DIAGONAL_MASK_9 << 24;
constexpr uint64_t DIAGONAL_MASK_9L4 = DIAGONAL_MASK_9 << 32;
constexpr uint64_t DIAGONAL_MASK_9L3 = DIAGONAL_MASK_9 << 40;
constexpr uint64_t DIAGONAL_MASK_9L2 = DIAGONAL_MASK_9 << 48;
constexpr uint64_t DIAGONAL_MASK_9L1 = DIAGONAL_MASK_9 << 56;
constexpr uint64_t DIAGONAL_MASK_9R7 = DIAGONAL_MASK_9 >> 8;
constexpr uint64_t DIAGONAL_MASK_9R6 = DIAGONAL_MASK_9 >> 16;
constexpr uint64_t DIAGONAL_MASK_9R5 = DIAGONAL_MASK_9 >> 24;
constexpr uint64_t DIAGONAL_MASK_9R4 = DIAGONAL_MASK_9 >> 32;
constexpr uint64_t DIAGONAL_MASK_9R3 = DIAGONAL_MASK_9 >> 40;
constexpr uint64_t DIAGONAL_MASK_9R2 = DIAGONAL_MASK_9 >> 48;
constexpr uint64_t DIAGONAL_MASK_9R1 = DIAGONAL_MASK_9 >> 56;

constexpr uint64_t INTERNAL_MASK = 0x007E7E7E7E7E7E00;
constexpr uint64_t EDGE_MASK = ~(INTERNAL_MASK | CORNER_MASK);
constexpr uint64_t NON_CORNER_MASK = ~CORNER_MASK;
constexpr uint64_t CENTER_MASK = 0x0000001818000000;

// indices for each square
constexpr int I_A1 = 0;
constexpr int I_A2 = 1;
constexpr int I_A3 = 2;
constexpr int I_A4 = 3;
constexpr int I_A5 = 4;
constexpr int I_A6 = 5;
constexpr int I_A7 = 6;
constexpr int I_A8 = 7;

constexpr int I_B1 = 8;
constexpr int I_B2 = 9;
constexpr int I_B3 = 10;
constexpr int I_B4 = 11;
constexpr int I_B5 = 12;
constexpr int I_B6 = 13;
constexpr int I_B7 = 14;
constexpr int I_B8 = 15;

constexpr int I_C1 = 16;
constexpr int I_C2 = 17;
constexpr int I_C3 = 18;
constexpr int I_C4 = 19;
constexpr int I_C5 = 20;
constexpr int I_C6 = 21;
constexpr int I_C7 = 22;
constexpr int I_C8 = 23;

constexpr int I_D1 = 24;
constexpr int I_D2 = 25;
constexpr int I_D3 = 26;
constexpr int I_D4 = 27;
constexpr int I_D5 = 28;
constexpr int I_D6 = 29;
constexpr int I_D7 = 30;
constexpr int I_D8 = 31;

constexpr int I_E1 = 32;
constexpr int I_E2 = 33;
constexpr int I_E3 = 34;
constexpr int I_E4 = 35;
constexpr int I_E5 = 36;
constexpr int I_E6 = 37;
constexpr int I_E7 = 38;
constexpr int I_E8 = 39;

constexpr int I_F1 = 40;
constexpr int I_F2 = 41;
constexpr int I_F3 = 42;
constexpr int I_F4 = 43;
constexpr int I_F5 = 44;
constexpr int I_F6 = 45;
constexpr int I_F7 = 46;
constexpr int I_F8 = 47;

constexpr int I_G1 = 48;
constexpr int I_G2 = 49;
constexpr int I_G3 = 50;
constexpr int I_G4 = 51;
constexpr int I_G5 = 52;
constexpr int I_G6 = 53;
constexpr int I_G7 = 54;
constexpr int I_G8 = 55;

constexpr int I_H1 = 56;
constexpr int I_H2 = 57;
constexpr int I_H3 = 58;
constexpr int I_H4 = 59;
constexpr int I_H5 = 60;
constexpr int I_H6 = 61;
constexpr int I_H7 = 62;
constexpr int I_H8 = 63;

#endif //OTHELLO_MASKS_H
