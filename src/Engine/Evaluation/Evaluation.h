//
// Created by Benjamin Lee on 3/8/24.
//

#ifndef OTHELLO_EVALUATION_H
#define OTHELLO_EVALUATION_H

#include "TernaryIndices.h"
#include "StaticEvaluations.h"
#include "../../Game/Board.h"
#include "../../Const.h"
#include "../Masks.h"
#include <fstream>

namespace engine {
    class SearchNode;

    namespace eval {
        constexpr int POW3[] = {
                1, 3, 9, 27, 81, 243, 729, 2187, 6561,
                19683, 59049, 177147, 531441, 1594323, 4782969,
                14348907, 43046721, 129140163, 387420489, 1162261467
        };

    #if TUNE_MODE_MIDGAME || !RUN_TRAINING_MODE
        constexpr int NUM_PATTERN_SYMMETRIES = 58;
        constexpr int NUM_PATTERNS = 14;
        constexpr int MAX_COORD_FEATURES = 11;

        constexpr int NUM_PATTERN_SYMMETRIES_END = 16;
        constexpr int NUM_PATTERNS_END = 4;
        constexpr int MAX_COORD_FEATURES_END = 6;

        constexpr int MAX_FEATURE_SIZE = 10;

        constexpr int NUM_PATTERN_PERMUTATIONS = 403380;
        constexpr int NUM_PHASES = 15;
        constexpr int NUM_PHASE_DISCS = (120 + NUM_PHASES) / (2 * NUM_PHASES);
        constexpr int MAX_SURROUND = 64;
        constexpr int SCORE_RANGE = 65;

        constexpr int NUM_PHASE_PARAMS = NUM_PATTERN_PERMUTATIONS + MAX_SURROUND*MAX_SURROUND + SCORE_RANGE*SCORE_RANGE;
        constexpr int NUM_FEATURES = NUM_PATTERN_SYMMETRIES + 2;
        constexpr int NUM_EVAL_PARAMS = NUM_PHASE_PARAMS * NUM_PHASES;
    #else // TUNE_ENDGAME
        constexpr int NUM_PATTERN_SYMMETRIES = 16;
        constexpr int NUM_PATTERNS = 4;
        constexpr int MAX_COORD_FEATURES = 12;

        constexpr int NUM_PATTERN_SYMMETRIES_END = 16;
        constexpr int NUM_PATTERNS_END = 4;
        constexpr int MAX_COORD_FEATURES_END = 6;

        constexpr int MAX_FEATURE_SIZE = 10;

        constexpr int NUM_PATTERN_PERMUTATIONS = 236196;
        constexpr int NUM_PHASES = 1;
        constexpr int NUM_PHASE_DISCS = END_SEARCH_DEPTH;
        constexpr int MAX_SURROUND = 64;
        constexpr int SCORE_RANGE = 65;

        constexpr int NUM_PHASE_PARAMS = NUM_PATTERN_PERMUTATIONS;
        constexpr int NUM_FEATURES = NUM_PATTERN_SYMMETRIES;
        constexpr int NUM_EVAL_PARAMS = NUM_PHASE_PARAMS * NUM_PHASES;
    #endif

        /**
        * @brief A pattern for the evaluation function
        *
        * @param size The size of the pattern
        * @param cells The cells of the pattern
        */
        struct Feature {
            uint_fast8_t size;
            uint_fast8_t cells[MAX_FEATURE_SIZE];
        };

        /**
        * @brief a feature that a coordinate is apart of
        *
        * @param feature The index of the feature that the coordinate is apart of
        * @param offset The index offset of the coordinate in the feature
        */
        struct CoordFeature {
            uint_fast8_t feature = 255; // 255 is an invalid feature
            uint_fast16_t offset = 0;
        };

        /**
        * @brief Which features the coordinate is apart of
        *
        * @param numFeatures The number of features the coordinate is apart of
        * @param features The features the coordinate is apart of
        */
        struct CoordFeatures {
            uint_fast8_t numFeatures = 0;
            CoordFeature features[MAX_COORD_FEATURES] = {0};
        };

        constexpr const char *PATTERN_NAMES[NUM_FEATURES] = {
                "edge + 2X",
                "corner + block",
                "triangle",
                "thick diagonal",
                "stealth bomber",
                "corner 2x5",
                "corner 3x3",
                "hv2",
                "hv3",
                "hv4",
                "d8",
                "d7",
                "d6",
                "d5",
                "disc count",
                "surround"
        };

        constexpr Feature PATTERNS[NUM_PATTERNS] = {
                {10, {T_B2, T_A1, T_A2, T_A3, T_A4, T_A5, T_A6, T_A7, T_A8, T_B7}},
                {10, {T_A8, T_A6, T_A5, T_A4, T_A3, T_A1, T_B6, T_B5, T_B4, T_B3}},
                {10, {T_A1, T_B1, T_C1, T_D1, T_A2, T_B2, T_C2, T_A3, T_B3, T_A4}},
                {10, {T_A1, T_B2, T_C3, T_D4, T_B1, T_C2, T_D3, T_A2, T_B3, T_C4}},
                {10, {T_A1, T_B1, T_C1, T_D1, T_E1, T_A2, T_B2, T_A3, T_A4, T_A5}},
                {10, {T_A1, T_A2, T_A3, T_A4, T_A5, T_B1, T_B2, T_B3, T_B4, T_B5}},
                {9,  {T_A1, T_A2, T_A3, T_B1, T_B2, T_B3, T_C1, T_C2, T_C3}},
                {8,  {T_B1, T_B2, T_B3, T_B4, T_B5, T_B6, T_B7, T_B8}},
                {8,  {T_C1, T_C2, T_C3, T_C4, T_C5, T_C6, T_C7, T_C8}},
                {8,  {T_D1, T_D2, T_D3, T_D4, T_D5, T_D6, T_D7, T_D8}},
                {8,  {T_H1, T_G2, T_F3, T_E4, T_D5, T_C6, T_B7, T_A8}},
                {7,  {T_G1, T_F2, T_E3, T_D4, T_C5, T_B6, T_A7}},
                {6,  {T_F1, T_E2, T_D3, T_C4, T_B5, T_A6}},
                {5,  {T_E1, T_D2, T_C3, T_B4, T_A5}}
        };

        constexpr Feature FEATURES[NUM_PATTERN_SYMMETRIES] = {
                // 0 edge + 2X
                {10, {T_B2, T_A1, T_A2, T_A3, T_A4, T_A5, T_A6, T_A7, T_A8, T_B7}},
                {10, {T_B7, T_A8, T_B8, T_C8, T_D8, T_E8, T_F8, T_G8, T_H8, T_G7}},
                {10, {T_G7, T_H8, T_H7, T_H6, T_H5, T_H4, T_H3, T_H2, T_H1, T_G2}},
                {10, {T_G2, T_H1, T_G1, T_F1, T_E1, T_D1, T_C1, T_B1, T_A1, T_B2}},
                // 1 corner + block
                {10, {T_A8, T_A6, T_A5, T_A4, T_A3, T_A1, T_B6, T_B5, T_B4, T_B3}},
                {10, {T_H8, T_F8, T_E8, T_D8, T_C8, T_A8, T_F7, T_E7, T_D7, T_C7}},
                {10, {T_H1, T_H3, T_H4, T_H5, T_H6, T_H8, T_G3, T_G4, T_G5, T_G6}},
                {10, {T_A1, T_C1, T_D1, T_E1, T_F1, T_H1, T_C2, T_D2, T_E2, T_F2}},
                // 2 triangle
                {10, {T_A1, T_B1, T_C1, T_D1, T_A2, T_B2, T_C2, T_A3, T_B3, T_A4}},
                {10, {T_A8, T_A7, T_A6, T_A5, T_B8, T_B7, T_B6, T_C8, T_C7, T_D8}},
                {10, {T_H8, T_G8, T_F8, T_E8, T_H7, T_G7, T_F7, T_H6, T_G6, T_H5}},
                {10, {T_H1, T_H2, T_H3, T_H4, T_G1, T_G2, T_G3, T_F1, T_F2, T_E1}},
                // 3 thick diagonal
                {10, {T_A1, T_B2, T_C3, T_D4, T_B1, T_C2, T_D3, T_A2, T_B3, T_C4}},
                {10, {T_A8, T_B7, T_C6, T_D5, T_A7, T_B6, T_C5, T_B8, T_C7, T_D6}},
                {10, {T_H8, T_G7, T_F6, T_E5, T_G8, T_F7, T_E6, T_H7, T_G6, T_F5}},
                {10, {T_H1, T_G2, T_F3, T_E4, T_H2, T_G3, T_F4, T_G1, T_F2, T_E3}},
                // 4 stealth bomber
                {10, {T_A1, T_B1, T_C1, T_D1, T_E1, T_A2, T_B2, T_A3, T_A4, T_A5}},
                {10, {T_A8, T_A7, T_A6, T_A5, T_A4, T_B8, T_B7, T_C8, T_D8, T_E8}},
                {10, {T_H8, T_G8, T_F8, T_E8, T_D8, T_H7, T_G7, T_H6, T_H5, T_H4}},
                {10, {T_H1, T_H2, T_H3, T_H4, T_H5, T_G1, T_G2, T_F1, T_E1, T_D1}},
                // 5 corner 2x5
                {10, {T_A1, T_A2, T_A3, T_A4, T_A5, T_B1, T_B2, T_B3, T_B4, T_B5}},
                {10, {T_A8, T_A7, T_A6, T_A5, T_A4, T_B8, T_B7, T_B6, T_B5, T_B4}},
                {10, {T_A8, T_B8, T_C8, T_D8, T_E8, T_A7, T_B7, T_C7, T_D7, T_E7}},
                {10, {T_H8, T_G8, T_F8, T_E8, T_D8, T_H7, T_G7, T_F7, T_E7, T_D7}},
                {10, {T_H8, T_H7, T_H6, T_H5, T_H4, T_G8, T_G7, T_G6, T_G5, T_G4}},
                {10, {T_H1, T_H2, T_H3, T_H4, T_H5, T_G1, T_G2, T_G3, T_G4, T_G5}},
                {10, {T_H1, T_G1, T_F1, T_E1, T_D1, T_H2, T_G2, T_F2, T_E2, T_D2}},
                {10, {T_A1, T_B1, T_C1, T_D1, T_E1, T_A2, T_B2, T_C2, T_D2, T_E2}},
                // 6 corner 3x3
                {9, {T_A1, T_A2, T_A3, T_B1, T_B2, T_B3, T_C1, T_C2, T_C3}},
                {9, {T_A8, T_B8, T_C8, T_A7, T_B7, T_C7, T_A6, T_B6, T_C6}},
                {9, {T_H8, T_H7, T_H6, T_G8, T_G7, T_G6, T_F8, T_F7, T_F6}},
                {9, {T_H1, T_G1, T_F1, T_H2, T_G2, T_F2, T_H3, T_G3, T_F3}},
                // 7 hv2
                {8, {T_B1, T_B2, T_B3, T_B4, T_B5, T_B6, T_B7, T_B8}},
                {8, {T_A7, T_B7, T_C7, T_D7, T_E7, T_F7, T_G7, T_H7}},
                {8, {T_G8, T_G7, T_G6, T_G5, T_G4, T_G3, T_G2, T_G1}},
                {8, {T_H2, T_G2, T_F2, T_E2, T_D2, T_C2, T_B2, T_A2}},
                // 8 hv3
                {8, {T_C1, T_C2, T_C3, T_C4, T_C5, T_C6, T_C7, T_C8}},
                {8, {T_A6, T_B6, T_C6, T_D6, T_E6, T_F6, T_G6, T_H6}},
                {8, {T_F8, T_F7, T_F6, T_F5, T_F4, T_F3, T_F2, T_F1}},
                {8, {T_H3, T_G3, T_F3, T_E3, T_D3, T_C3, T_B3, T_A3}},
                // 9 hv4
                {8, {T_D1, T_D2, T_D3, T_D4, T_D5, T_D6, T_D7, T_D8}},
                {8, {T_A5, T_B5, T_C5, T_D5, T_E5, T_F5, T_G5, T_H5}},
                {8, {T_E8, T_E7, T_E6, T_E5, T_E4, T_E3, T_E2, T_E1}},
                {8, {T_H4, T_G4, T_F4, T_E4, T_D4, T_C4, T_B4, T_A4}},
                // 10 d8
                {8, {T_H1, T_G2, T_F3, T_E4, T_D5, T_C6, T_B7, T_A8}},
                {8, {T_A1, T_B2, T_C3, T_D4, T_E5, T_F6, T_G7, T_H8}},
                // 11 d7
                {7, {T_G1, T_F2, T_E3, T_D4, T_C5, T_B6, T_A7}},
                {7, {T_A2, T_B3, T_C4, T_D5, T_E6, T_F7, T_G8}},
                {7, {T_B8, T_C7, T_D6, T_E5, T_F4, T_G3, T_H2}},
                {7, {T_H7, T_G6, T_F5, T_E4, T_D3, T_C2, T_B1}},
                // 12 d6
                {6, {T_F1, T_E2, T_D3, T_C4, T_B5, T_A6}},
                {6, {T_A3, T_B4, T_C5, T_D6, T_E7, T_F8}},
                {6, {T_C8, T_D7, T_E6, T_F5, T_G4, T_H3}},
                {6, {T_H6, T_G5, T_F4, T_E3, T_D2, T_C1}},
                // 13 d5
                {5, {T_E1, T_D2, T_C3, T_B4, T_A5}},
                {5, {T_A4, T_B5, T_C6, T_D7, T_E8}},
                {5, {T_D8, T_E7, T_F6, T_G5, T_H4}},
                {5, {T_H5, T_G4, T_F3, T_E2, T_D1}}
        };


        constexpr CoordFeatures COORD_FEATURES[64] = {
                {11, {{0, 6561}, {3, 3}, {4, 81}, {7, 19683}, {8, 19683}, {12, 19683}, {16, 19683}, {20, 19683}, {27, 19683}, {28, 6561}, {45, 2187}}},
                {9, {{0, 2187}, {8, 243}, {12, 9}, {16, 81}, {20, 6561}, {27, 81}, {28, 2187}, {35, 1}, {47, 729}}},
                {8, {{0, 729}, {4, 243}, {8, 9}, {16, 9}, {20, 2187}, {28, 729}, {39, 1}, {51, 243}}},
                {9, {{0, 243}, {4, 729}, {8, 1}, {16, 3}, {17, 243}, {20, 729}, {21, 243}, {43, 1}, {55, 81}}},
                {9, {{0, 81}, {4, 2187}, {9, 729}, {16, 1}, {17, 729}, {20, 243}, {21, 729}, {41, 2187}, {54, 1}}},
                {8, {{0, 27}, {4, 6561}, {9, 2187}, {17, 2187}, {21, 2187}, {29, 9}, {37, 2187}, {50, 1}}},
                {9, {{0, 9}, {9, 6561}, {13, 243}, {17, 6561}, {21, 6561}, {22, 81}, {29, 243}, {33, 2187}, {46, 1}}},
                {11, {{0, 3}, {1, 6561}, {4, 19683}, {5, 81}, {9, 19683}, {13, 19683}, {17, 19683}, {21, 19683}, {22, 19683}, {29, 6561}, {44, 1}}},
                {9, {{3, 9}, {8, 6561}, {12, 243}, {16, 6561}, {20, 81}, {27, 6561}, {28, 243}, {32, 2187}, {49, 1}}},
                {11, {{0, 19683}, {3, 1}, {8, 81}, {12, 6561}, {16, 27}, {20, 27}, {27, 27}, {28, 81}, {32, 729}, {35, 3}, {45, 729}}},
                {8, {{4, 1}, {8, 3}, {12, 3}, {20, 9}, {28, 27}, {32, 243}, {39, 3}, {47, 243}}},
                {7, {{4, 3}, {20, 3}, {21, 1}, {32, 81}, {43, 3}, {51, 81}, {54, 3}}},
                {7, {{4, 9}, {20, 1}, {21, 3}, {32, 27}, {41, 729}, {50, 3}, {55, 27}}},
                {8, {{4, 27}, {9, 27}, {13, 81}, {21, 9}, {29, 3}, {32, 9}, {37, 729}, {46, 3}}},
                {11, {{0, 1}, {1, 19683}, {9, 81}, {13, 6561}, {17, 27}, {21, 27}, {22, 27}, {29, 81}, {32, 3}, {33, 729}, {44, 3}}},
                {9, {{1, 2187}, {9, 243}, {13, 9}, {17, 81}, {21, 81}, {22, 6561}, {29, 2187}, {32, 1}, {48, 729}}},
                {8, {{3, 27}, {7, 6561}, {8, 2187}, {16, 2187}, {27, 2187}, {28, 9}, {36, 2187}, {53, 1}}},
                {8, {{7, 27}, {8, 27}, {12, 81}, {27, 9}, {28, 3}, {35, 9}, {36, 729}, {49, 3}}},
                {6, {{12, 2187}, {28, 1}, {36, 243}, {39, 9}, {45, 243}, {54, 9}}},
                {5, {{12, 1}, {36, 81}, {43, 9}, {47, 81}, {50, 9}}},
                {5, {{13, 27}, {36, 27}, {41, 243}, {46, 9}, {51, 27}}},
                {6, {{13, 2187}, {29, 1}, {36, 9}, {37, 243}, {44, 9}, {55, 9}}},
                {8, {{5, 1}, {9, 3}, {13, 3}, {22, 9}, {29, 27}, {33, 243}, {36, 3}, {48, 243}}},
                {8, {{1, 729}, {5, 243}, {9, 9}, {17, 9}, {22, 2187}, {29, 729}, {36, 1}, {52, 243}}},
                {9, {{3, 81}, {7, 2187}, {8, 729}, {16, 729}, {19, 1}, {26, 243}, {27, 729}, {40, 2187}, {57, 1}}},
                {7, {{7, 9}, {26, 1}, {27, 3}, {35, 27}, {40, 729}, {53, 3}, {54, 27}}},
                {5, {{12, 27}, {39, 27}, {40, 243}, {49, 9}, {50, 27}}},
                {5, {{12, 729}, {40, 81}, {43, 27}, {45, 81}, {46, 27}}},
                {5, {{13, 729}, {40, 27}, {41, 81}, {44, 27}, {47, 27}}},
                {5, {{13, 1}, {37, 81}, {40, 9}, {48, 81}, {51, 9}}},
                {7, {{5, 3}, {22, 3}, {23, 1}, {33, 81}, {40, 3}, {52, 81}, {55, 3}}},
                {9, {{1, 243}, {5, 729}, {9, 1}, {17, 3}, {18, 243}, {22, 729}, {23, 243}, {40, 1}, {56, 81}}},
                {9, {{3, 243}, {7, 729}, {11, 1}, {16, 243}, {19, 3}, {26, 729}, {27, 243}, {42, 1}, {54, 81}}},
                {7, {{7, 3}, {26, 3}, {27, 1}, {35, 81}, {42, 3}, {50, 81}, {57, 3}}},
                {5, {{15, 1}, {39, 81}, {42, 9}, {46, 81}, {53, 9}}},
                {5, {{15, 729}, {42, 27}, {43, 81}, {44, 81}, {49, 27}}},
                {5, {{14, 729}, {41, 27}, {42, 81}, {45, 27}, {48, 27}}},
                {5, {{14, 27}, {37, 27}, {42, 243}, {47, 9}, {52, 27}}},
                {7, {{5, 9}, {22, 1}, {23, 3}, {33, 27}, {42, 729}, {51, 3}, {56, 27}}},
                {9, {{1, 81}, {5, 2187}, {10, 729}, {17, 1}, {18, 729}, {22, 243}, {23, 729}, {42, 2187}, {55, 1}}},
                {8, {{3, 729}, {7, 243}, {11, 9}, {19, 9}, {26, 2187}, {31, 729}, {38, 1}, {50, 243}}},
                {8, {{7, 1}, {11, 3}, {15, 3}, {26, 9}, {31, 27}, {35, 243}, {38, 3}, {46, 243}}},
                {6, {{15, 2187}, {31, 1}, {38, 9}, {39, 243}, {44, 243}, {57, 9}}},
                {5, {{15, 27}, {38, 27}, {43, 243}, {48, 9}, {53, 27}}},
                {5, {{14, 1}, {38, 81}, {41, 9}, {49, 81}, {52, 9}}},
                {6, {{14, 2187}, {30, 1}, {37, 9}, {38, 243}, {45, 9}, {56, 9}}},
                {8, {{5, 27}, {10, 27}, {14, 81}, {23, 9}, {30, 3}, {33, 9}, {38, 729}, {47, 3}}},
                {8, {{1, 27}, {5, 6561}, {10, 2187}, {18, 2187}, {23, 2187}, {30, 9}, {38, 2187}, {51, 1}}},
                {9, {{3, 2187}, {11, 243}, {15, 9}, {19, 81}, {25, 81}, {26, 6561}, {31, 2187}, {34, 1}, {46, 729}}},
                {11, {{2, 1}, {3, 19683}, {11, 81}, {15, 6561}, {19, 27}, {25, 27}, {26, 27}, {31, 81}, {34, 3}, {35, 729}, {44, 729}}},
                {8, {{6, 27}, {11, 27}, {15, 81}, {25, 9}, {31, 3}, {34, 9}, {39, 729}, {48, 3}}},
                {7, {{6, 9}, {24, 1}, {25, 3}, {34, 27}, {43, 729}, {52, 3}, {57, 27}}},
                {7, {{6, 3}, {24, 3}, {25, 1}, {34, 81}, {41, 3}, {53, 81}, {56, 3}}},
                {8, {{6, 1}, {10, 3}, {14, 3}, {24, 9}, {30, 27}, {34, 243}, {37, 3}, {49, 243}}},
                {11, {{1, 1}, {2, 19683}, {10, 81}, {14, 6561}, {18, 27}, {23, 27}, {24, 27}, {30, 81}, {33, 3}, {34, 729}, {45, 3}}},
                {9, {{1, 9}, {10, 6561}, {14, 243}, {18, 6561}, {23, 6561}, {24, 81}, {30, 243}, {34, 2187}, {47, 1}}},
                {11, {{2, 3}, {3, 6561}, {6, 19683}, {7, 81}, {11, 19683}, {15, 19683}, {19, 19683}, {25, 19683}, {26, 19683}, {31, 6561}, {44, 2187}}},
                {9, {{2, 9}, {11, 6561}, {15, 243}, {19, 6561}, {25, 6561}, {26, 81}, {31, 243}, {35, 2187}, {48, 1}}},
                {8, {{2, 27}, {6, 6561}, {11, 2187}, {19, 2187}, {25, 2187}, {31, 9}, {39, 2187}, {52, 1}}},
                {9, {{2, 81}, {6, 2187}, {11, 729}, {18, 1}, {19, 729}, {24, 243}, {25, 729}, {43, 2187}, {56, 1}}},
                {9, {{2, 243}, {6, 729}, {10, 1}, {18, 3}, {19, 243}, {24, 729}, {25, 243}, {41, 1}, {57, 81}}},
                {8, {{2, 729}, {6, 243}, {10, 9}, {18, 9}, {24, 2187}, {30, 729}, {37, 1}, {53, 243}}},
                {9, {{2, 2187}, {10, 243}, {14, 9}, {18, 81}, {23, 81}, {24, 6561}, {30, 2187}, {33, 1}, {49, 729}}},
                {11, {{1, 3}, {2, 6561}, {5, 19683}, {6, 81}, {10, 19683}, {14, 19683}, {18, 19683}, {23, 19683}, {24, 19683}, {30, 6561}, {45, 1}}}
        };

        constexpr CoordFeatures COORD_FEATURES_END[64] = {
                {6, {{0, 6561}, {3, 3}, {4, 81}, {7, 19683}, {8, 19683}, {12, 19683}}},
                {3, {{0, 2187}, {8, 243}, {12, 9}}},
                {3, {{0, 729}, {4, 243}, {8, 9}}},
                {3, {{0, 243}, {4, 729}, {8, 1}}},
                {3, {{0, 81}, {4, 2187}, {9, 729}}},
                {3, {{0, 27}, {4, 6561}, {9, 2187}}},
                {3, {{0, 9}, {9, 6561}, {13, 243}}},
                {6, {{0, 3}, {1, 6561}, {4, 19683}, {5, 81}, {9, 19683}, {13, 19683}}},
                {3, {{3, 9}, {8, 6561}, {12, 243}}},
                {4, {{0, 19683}, {3, 1}, {8, 81}, {12, 6561}}},
                {3, {{4, 1}, {8, 3}, {12, 3}}},
                {1, {{4, 3}}},
                {1, {{4, 9}}},
                {3, {{4, 27}, {9, 27}, {13, 81}}},
                {4, {{0, 1}, {1, 19683}, {9, 81}, {13, 6561}}},
                {3, {{1, 2187}, {9, 243}, {13, 9}}},
                {3, {{3, 27}, {7, 6561}, {8, 2187}}},
                {3, {{7, 27}, {8, 27}, {12, 81}}},
                {1, {{12, 2187}}},
                {1, {{12, 1}}},
                {1, {{13, 27}}},
                {1, {{13, 2187}}},
                {3, {{5, 1}, {9, 3}, {13, 3}}},
                {3, {{1, 729}, {5, 243}, {9, 9}}},
                {3, {{3, 81}, {7, 2187}, {8, 729}}},
                {1, {{7, 9}}},
                {1, {{12, 27}}},
                {1, {{12, 729}}},
                {1, {{13, 729}}},
                {1, {{13, 1}}},
                {1, {{5, 3}}},
                {3, {{1, 243}, {5, 729}, {9, 1}}},
                {3, {{3, 243}, {7, 729}, {11, 1}}},
                {1, {{7, 3}}},
                {1, {{15, 1}}},
                {1, {{15, 729}}},
                {1, {{14, 729}}},
                {1, {{14, 27}}},
                {1, {{5, 9}}},
                {3, {{1, 81}, {5, 2187}, {10, 729}}},
                {3, {{3, 729}, {7, 243}, {11, 9}}},
                {3, {{7, 1}, {11, 3}, {15, 3}}},
                {1, {{15, 2187}}},
                {1, {{15, 27}}},
                {1, {{14, 1}}},
                {1, {{14, 2187}}},
                {3, {{5, 27}, {10, 27}, {14, 81}}},
                {3, {{1, 27}, {5, 6561}, {10, 2187}}},
                {3, {{3, 2187}, {11, 243}, {15, 9}}},
                {4, {{2, 1}, {3, 19683}, {11, 81}, {15, 6561}}},
                {3, {{6, 27}, {11, 27}, {15, 81}}},
                {1, {{6, 9}}},
                {1, {{6, 3}}},
                {3, {{6, 1}, {10, 3}, {14, 3}}},
                {4, {{1, 1}, {2, 19683}, {10, 81}, {14, 6561}}},
                {3, {{1, 9}, {10, 6561}, {14, 243}}},
                {6, {{2, 3}, {3, 6561}, {6, 19683}, {7, 81}, {11, 19683}, {15, 19683}}},
                {3, {{2, 9}, {11, 6561}, {15, 243}}},
                {3, {{2, 27}, {6, 6561}, {11, 2187}}},
                {3, {{2, 81}, {6, 2187}, {11, 729}}},
                {3, {{2, 243}, {6, 729}, {10, 1}}},
                {3, {{2, 729}, {6, 243}, {10, 9}}},
                {3, {{2, 2187}, {10, 243}, {14, 9}}},
                {6, {{1, 3}, {2, 6561}, {5, 19683}, {6, 81}, {10, 19683}, {14, 19683}}}
        };

        constexpr int FEATURE_TO_PATTERN[NUM_FEATURES] = {
                0, 0, 0, 0,
                1, 1, 1, 1,
                2, 2, 2, 2,
                3, 3, 3, 3,
                4, 4, 4, 4,
                5, 5, 5, 5, 5, 5, 5, 5,
                6, 6, 6, 6,
                7, 7, 7, 7,
                8, 8, 8, 8,
                9, 9, 9, 9,
                10, 10,
                11, 11, 11, 11,
                12, 12, 12, 12,
                13, 13, 13, 13,
                14,
                15
        };

        #if TUNE_MODE_MIDGAME || !RUN_TRAINING_MODE
            /**
             * @brief get phase index of the game
             * @param numDiscs number of discs on the board
             * @return phase index
             */
            inline int get_phase(int numDiscs) {
                return ((numDiscs - 5) / NUM_PHASE_DISCS) & -(int) (numDiscs >= 5);
            }
        #else
            /**
             * @brief get phase index of the game
             * @param numDiscs number of discs on the board
             * @return 0 (endgame does not have phases)
             */
            inline int get_phase(int numDiscs) {
                return 0;
            }
        #endif

        class EvaluationFeatures {
        public:
            EvaluationFeatures() = default;
            explicit EvaluationFeatures(const Board *board);
            explicit EvaluationFeatures(SearchNode *node);

            static inline uint16_t get_feature_index(const uint_fast8_t boardArray[], const Feature *f) {
                uint16_t feature = 0;
                #pragma omp simd
                for (int i = 0; i < f->size; i++) {
                    feature *= 3;
                    feature += boardArray[f->cells[i]];
                }
                return feature;
            }

            inline void calc_features(const Board *b) {
                uint_fast8_t boardArray[64];
                b->to_array(boardArray);
                #pragma omp simd
                for (int i = 0; i < NUM_PATTERN_SYMMETRIES; i++) {
                    features[i] = get_feature_index(boardArray, &FEATURES[i]);
                }
            }

            inline void play_move(const Move *move) {
                int i;
                if (reversed) {
                    // add the new disc
                    #pragma omp simd
                    //for (i = 0; i < MAX_COORD_FEATURES && COORD_FEATURES[move->x].features[i].feature != NO_FEATURE; i++)
                    for (i = 0; i < COORD_FEATURES[move->x].numFeatures; i++)
                        // from E to O => 2 to 1
                        features[COORD_FEATURES[move->x].features[i].feature] -= COORD_FEATURES[move->x].features[i].offset;
                    // flip the discs
                    auto flip = move->flip;
                    #pragma omp simd
                    for (auto x = bit::first_set_idx(flip); flip; x = bit::next_set_idx(flip))
                        for (i = 0; i < COORD_FEATURES[x].numFeatures; i++)
                            // from P to O => 0 to 1
                            features[COORD_FEATURES[x].features[i].feature] += COORD_FEATURES[x].features[i].offset;
                } else {
                    // add the new disc
                    #pragma omp simd
                    for (i = 0; i < COORD_FEATURES[move->x].numFeatures; i++)
                        // from E to P => 2 to 0
                        features[COORD_FEATURES[move->x].features[i].feature] -=
                                COORD_FEATURES[move->x].features[i].offset << 1;
                    // flip the discs
                    auto flip = move->flip;
                    #pragma omp simd
                    for (auto x = bit::first_set_idx(flip); flip; x = bit::next_set_idx(flip))
                        for (i = 0; i < COORD_FEATURES[x].numFeatures; i++)
                            // from O to P => 1 to 0
                            features[COORD_FEATURES[x].features[i].feature] -= COORD_FEATURES[x].features[i].offset;
                }
                reversed ^= 1;
            }

            inline void undo_move(const Move *move) {
                reversed ^= 1;
                int i;
                if (reversed) {
                    // add the new disc
                    #pragma omp simd
                    for (i = 0; i < COORD_FEATURES[move->x].numFeatures; i++)
                        // from O to E => 1 to 2
                        this->features[COORD_FEATURES[move->x].features[i].feature] += COORD_FEATURES[move->x].features[i].offset;
                    // flip the discs
                    auto flip = move->flip;
                    #pragma omp simd
                    for (auto x = bit::first_set_idx(flip); flip; x = bit::next_set_idx(flip))
                        for (i = 0; i < COORD_FEATURES[x].numFeatures; i++)
                            // from O to P => 1 to 0
                            this->features[COORD_FEATURES[x].features[i].feature] -= COORD_FEATURES[x].features[i].offset;
                } else {
                    // add the new disc
                    #pragma omp simd
                    for (i = 0; i < COORD_FEATURES[move->x].numFeatures; i++)
                        // from P to E => 0 to 2
                        this->features[COORD_FEATURES[move->x].features[i].feature] +=
                                COORD_FEATURES[move->x].features[i].offset << 1;
                    // flip the discs
                    auto flip = move->flip;
                    #pragma omp simd
                    for (auto x = bit::first_set_idx(flip); flip; x = bit::next_set_idx(flip))
                        for (i = 0; i < COORD_FEATURES[x].numFeatures; i++)
                            // from P to O => 0 to 1
                            this->features[COORD_FEATURES[x].features[i].feature] += COORD_FEATURES[x].features[i].offset;
                }
            }

            inline void play_move_end(const Move *move) {
                int i;
                if (reversed) {
                    // add the new disc
                    #pragma omp simd
                    for (i = 0; i < COORD_FEATURES_END[move->x].numFeatures; i++)
                        // from E to O => 2 to 1
                        this->features[COORD_FEATURES_END[move->x].features[i].feature] -= COORD_FEATURES_END[move->x].features[i].offset;
                    // flip the discs
                    auto flip = move->flip;
                    #pragma omp simd
                    for (auto x = bit::first_set_idx(flip); flip; x = bit::next_set_idx(flip))
                        for (i = 0; i < COORD_FEATURES_END[x].numFeatures; i++)
                            // from P to O => 0 to 1
                            this->features[COORD_FEATURES_END[x].features[i].feature] += COORD_FEATURES_END[x].features[i].offset;
                } else {
                    // add the new disc
                    #pragma omp simd
                    for (i = 0; i < COORD_FEATURES_END[move->x].numFeatures; i++)
                        // from E to P => 2 to 0
                        this->features[COORD_FEATURES_END[move->x].features[i].feature] -=
                                COORD_FEATURES_END[move->x].features[i].offset << 1;
                    // flip the discs
                    auto flip = move->flip;
                    #pragma omp simd
                    for (auto x = bit::first_set_idx(flip); flip; x = bit::next_set_idx(flip))
                        for (i = 0; i < COORD_FEATURES_END[x].numFeatures; i++)
                            // from O to P => 1 to 0
                            this->features[COORD_FEATURES_END[x].features[i].feature] -= COORD_FEATURES_END[x].features[i].offset;
                }
                reversed ^= 1;
            }

            inline void undo_move_end(const Move *move) {
                reversed ^= 1;
                int i;
                if (reversed) {
                    // add the new disc
                    #pragma omp simd
                    for (i = 0; i < COORD_FEATURES_END[move->x].numFeatures; i++)
                        // from O to E => 1 to 2
                        this->features[COORD_FEATURES_END[move->x].features[i].feature] += COORD_FEATURES_END[move->x].features[i].offset;
                    // flip the discs
                    auto flip = move->flip;
                    #pragma omp simd
                    for (auto x = bit::first_set_idx(flip); flip; x = bit::next_set_idx(flip))
                        for (i = 0; i < COORD_FEATURES_END[x].numFeatures; i++)
                            // from O to P => 1 to 0
                            this->features[COORD_FEATURES_END[x].features[i].feature] -= COORD_FEATURES_END[x].features[i].offset;
                } else {
                    // add the new disc
                    #pragma omp simd
                    for (i = 0; i < COORD_FEATURES_END[move->x].numFeatures; i++)
                        // from P to E => 0 to 2
                        this->features[COORD_FEATURES_END[move->x].features[i].feature] +=
                                COORD_FEATURES_END[move->x].features[i].offset << 1;
                    // flip the discs
                    auto flip = move->flip;
                    #pragma omp simd
                    for (auto x = bit::first_set_idx(flip); flip; x = bit::next_set_idx(flip))
                        for (i = 0; i < COORD_FEATURES_END[x].numFeatures; i++)
                            // from P to O => 0 to 1
                            this->features[COORD_FEATURES_END[x].features[i].feature] += COORD_FEATURES_END[x].features[i].offset;
                }
            }

            inline void pass() {
                reversed ^= 1;
            }

            [[nodiscard]] inline int pattern_evaluate(int phase) {
                int score = 0;

                auto phaseWeights = PATTERN_WEIGHTS[ reversed ][ phase ];

                #pragma omp simd
                for (int i = 0; i < NUM_PATTERN_SYMMETRIES; ++i)
                    score += phaseWeights[ FEATURE_TO_PATTERN[i] ][ this->features[i] ];

                return score;
            }

            [[nodiscard]] inline int pattern_evaluate_end() {
                int score = 0;
                #pragma omp simd
                for (int i = 0; i < NUM_PATTERN_SYMMETRIES_END; i++) {
                    score += PATTERN_WEIGHTS_END[reversed][FEATURE_TO_PATTERN[i]][features[i]];
                }
                return score;
            }

            /**
             * @brief Get the index for the reversed index
             *
             * @param index the index corresponding to the permutation of the pattern
             * @param size number of discs in the pattern
             * @return the reversed index
             */
            static inline int get_reversed_index(int index, int size) {
                int reversedIndex = index;
                int remainder;
                for (int i = 0; i < size; ++i) {
                    remainder = index % 3;
                    if (remainder == 0)
                        reversedIndex += POW3[i];
                    else if (remainder == 1)
                        reversedIndex -= POW3[i];
                    index /= 3;
                }
                return reversedIndex;
            }

            [[nodiscard]] Board to_board() const;

            [[nodiscard]] int mid_evaluate(const SearchNode *node);
            [[nodiscard]] int end_evaluate_move_ordering(SearchNode *node);

            static void eval_init(const std::string &filepath = WEIGHT_FILEPATH, const std::string &filepathEnd = WEIGHT_FILEPATH_END);

            static short PATTERN_WEIGHTS[2][NUM_PHASES][NUM_PATTERNS][POW3[MAX_FEATURE_SIZE]];     // [reversed][phase][pattern][feature]
            static short PATTERN_WEIGHTS_END[2][NUM_PATTERNS_END][POW3[MAX_FEATURE_SIZE]];         // [reversed][phase][pattern][feature]
            static short SURROUND_WEIGHTS[NUM_PHASES][MAX_SURROUND][MAX_SURROUND];                 // [phase][player_surround][opp_surround]
            static short SCORE_WEIGHTS[NUM_PHASES][SCORE_RANGE][SCORE_RANGE];                      // [phase][player_discs][opp_discs]

            private:
                int features[NUM_PATTERN_SYMMETRIES]{};
                bool reversed = false;
        };
    } //eval
} // engine

#endif //OTHELLO_EVALUATION_H
