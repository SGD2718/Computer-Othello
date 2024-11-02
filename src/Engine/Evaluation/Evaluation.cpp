//
// Created by Benjamin Lee on 3/8/24.
//

#include "Evaluation.h"
#include "../Search/SearchStructs.h"
#include "EvalBuilder.h"
#include <fstream>

namespace engine::eval {
    short EvaluationFeatures::PATTERN_WEIGHTS[2][NUM_PHASES][NUM_PATTERNS][POW3[MAX_FEATURE_SIZE]];     // [reversed][phase][pattern][feature]
    short EvaluationFeatures::PATTERN_WEIGHTS_END[2][NUM_PATTERNS_END][POW3[MAX_FEATURE_SIZE]];         // [reversed][phase][pattern][feature]
    short EvaluationFeatures::SURROUND_WEIGHTS[NUM_PHASES][MAX_SURROUND][MAX_SURROUND];                 // [phase][player_surround][opp_surround]
    short EvaluationFeatures::SCORE_WEIGHTS[NUM_PHASES][SCORE_RANGE][SCORE_RANGE];                      // [phase][player_discs][opp_discs]

    void EvaluationFeatures::eval_init(const std::string& filepath, const std::string& filepathEnd) {
        std::ifstream file(filepath, std::ios::binary);

        if (!file.is_open()) {
            std::cout << "Error opening file: " << filepath << std::endl;
            exit(1);
        }

        std::string weight;
        int patternIdx;
        int numPatternDiscs;
        int numPatternPermutations;
        short w;

        for (auto phase = 0; phase < NUM_PHASES; ++phase) {
            for (auto pattern = 0; pattern < NUM_PATTERNS; ++pattern) {
                numPatternDiscs = PATTERNS[pattern].size;
                numPatternPermutations = POW3[numPatternDiscs];
                for (auto i = 0; i < numPatternPermutations; ++i) {
                    file.read(reinterpret_cast<char *>(&w), sizeof(short));
                    PATTERN_WEIGHTS[0][phase][pattern][i] = w;
                    PATTERN_WEIGHTS[1][phase][pattern][get_reversed_index(i, numPatternDiscs)] = w;
                }
            }
            for (int p = 0; p < SCORE_RANGE; ++p) {
                for (int o = 0; o < SCORE_RANGE; ++o) {
                    file.read(reinterpret_cast<char *>(&SCORE_WEIGHTS[phase][p][o]), sizeof(short));
                }
            }
            for (int p = 0; p < MAX_SURROUND; ++p) {
                for (int o = 0; o < MAX_SURROUND; ++o) {
                    file.read(reinterpret_cast<char *>(&SURROUND_WEIGHTS[phase][p][o]), sizeof(short));
                }
            }
        }

        file.close();
        file.open(filepathEnd, std::ios::binary);

        if (!file.is_open()) {
            std::cout << "Error opening file: " << filepathEnd << std::endl;
            exit(1);
        }

        for (patternIdx = 0; patternIdx < NUM_PATTERNS_END; ++patternIdx) {
            numPatternDiscs = PATTERNS[patternIdx].size;
            numPatternPermutations = POW3[numPatternDiscs];
            for (auto i = 0; i < numPatternPermutations; ++i) {
                file.read(reinterpret_cast<char *>(&w), sizeof(short));
                PATTERN_WEIGHTS_END[0][patternIdx][i] = w;
                PATTERN_WEIGHTS_END[1][patternIdx][get_reversed_index(i, numPatternDiscs)] = w;
            }
        }

        file.close();
    }

    EvaluationFeatures::EvaluationFeatures(const Board *board) :
            reversed(false) {
        this->calc_features(board);
    }

    EvaluationFeatures::EvaluationFeatures(SearchNode *node) :
            reversed(false) {
        this->calc_features(&node->board);
    }

    int EvaluationFeatures::mid_evaluate(const SearchNode *node) {
        auto phase = get_phase(node->discCount);
        auto surroundP = get_potential_mobility(node->board.P, node->board.O);
        auto surroundO = get_potential_mobility(node->board.O, node->board.P);
        auto scoreP = __builtin_popcountll(node->board.P);
        auto scoreO = node->discCount - scoreP;

        int value = pattern_evaluate(phase) +
                SURROUND_WEIGHTS[phase][surroundP][surroundO] +
                SCORE_WEIGHTS[phase][scoreP][scoreO];

        value += value >= 0 ? HALF_EVAL_SCALE : -HALF_EVAL_SCALE;
        value >>= EVAL_SCALE_LOG_2;

        return value;
    }

    int EvaluationFeatures::end_evaluate_move_ordering(SearchNode *node) {
        int value = pattern_evaluate_end();
        // round and scale
        value += value >= 0 ? HALF_EVAL_SCALE : -HALF_EVAL_SCALE;
        value >>= EVAL_SCALE_LOG_2;
        return value;
    }

    Board EvaluationFeatures::to_board() const {
        Board board(0, 0), tmp;
        for (int i = 0; i < NUM_PATTERN_SYMMETRIES; i++) {
            tmp = EvalBuilder::feature_to_board(&FEATURES[i], features[i]);
            board.P |= tmp.P;
            board.O |= tmp.O;
        }
        return board;
    }
} // engine::eval