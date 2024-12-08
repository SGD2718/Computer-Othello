//
// Created by Benjamin Lee on 10/3/23.
//

#ifndef OTHELLO_CONST_H
#define OTHELLO_CONST_H

#include <iostream>
#include "Engine/Masks.h"

#define RUN_TRAINING_MODE false
#define TUNE_MODE_MIDGAME true
#define TUNE_PROBCUT false
#define USE_SIMD true
#define USE_MPC true
#define USE_ETC true
#define LOCK_TT false

constexpr int ETC_DEPTH = 14;
constexpr int MPC_DEPTH = 20;

constexpr int MAX_MPC_LEVEL = 5;

constexpr int MPC_LEVEL_74 = 0;
constexpr int MPC_LEVEL_88 = 1;
constexpr int MPC_LEVEL_93 = 2;
constexpr int MPC_LEVEL_98 = 3;
constexpr int MPC_LEVEL_99 = 4;
constexpr int MPC_LEVEL_100 = 5;

#if TUNE_MODE_MIDGAME
    #define MODEL_NAME "mid eval"
#else
    #define MODEL_NAME "end ordering"
#endif

constexpr int WIN = 64;
constexpr int LOSS = -64;

constexpr int SCORE_MAX = 64;
constexpr int SCORE_UNDEFINED = -127;

constexpr int NUM_TRAINING_GAMES = 121123;
constexpr int NUM_STAGE_FEATURES = 167267;
constexpr int NUM_STAGES = 13;
constexpr int NUM_EVAL_WEIGHTS = NUM_STAGE_FEATURES * NUM_STAGES;

constexpr int EVAL_SCALE_LOG_2 = 7;
constexpr int HALF_EVAL_SCALE = 1 << (EVAL_SCALE_LOG_2 - 1);
constexpr int EVAL_SCALE = 1 << EVAL_SCALE_LOG_2;
constexpr double EVAL_WEIGHT_TO_INT = (double)(1ULL << EVAL_SCALE_LOG_2);
constexpr double EVAL_TO_DOUBLE = 1 / (double)(1ULL << EVAL_SCALE_LOG_2);

constexpr int HASH_BITS = 28;

constexpr int MID_TO_END_DEPTH = 13;
constexpr int END_SEARCH_DEPTH = 20;
constexpr int PERFECT_SEARCH_DEPTH = 16;
constexpr int END_FAST_DEPTH = 0;

constexpr int HASH_MOVE_VALUE = 1000000;
constexpr int WIPEOUT_SCORE = 10000000;
constexpr int FIRST_HASH_MOVE_SCORE = 10000000;
constexpr int SECOND_HASH_MOVE_SCORE = 1000000;

constexpr uint8_t MAX_DEPTH = 64;

// pass move coordinates. This move should never be
// legal since the center 4 squares start occupied.
constexpr uint8_t I_PASS = 27;

constexpr uint_fast8_t PLAYER = 0;
constexpr uint_fast8_t OPPONENT = 1;

// A placeholder for a legal move bitboard that has not been calculated yet.
// Since the center 4 squares start occupied, this should be an impossible legal move bitboard.
constexpr uint64_t LEGAL_UNDEFINED = 1ULL << 28;

// A bitboard of squares that can be legal moves (every square but the middle 4)
constexpr uint64_t POSSIBLE_LEGALS = 0xffffffe7e7ffffff;

#define MPC_DATA_FILEPATH "/Users/benjaminlee/Desktop/Othello/assets/ProbCut/mpc.txt"
#define LOGBOOK_FILEPATH "/Users/benjaminlee/Desktop/Othello/assets/Evaluation/logbook.gam"

#define TORCH_MODEL_DIRECTORY "/Users/benjaminlee/Desktop/Othello/assets/Evaluation/Torch Models/"

#define WEIGHT_FILEPATH "/Users/benjaminlee/Desktop/Othello/assets/Evaluation/mid eval.bin"
#define RAW_WEIGHT_FILEPATH "/Users/benjaminlee/Desktop/Othello/assets/Evaluation/end ordering raw.bin"
#define WEIGHT_FILEPATH_END "/Users/benjaminlee/Desktop/Othello/assets/Evaluation/end ordering.bin"
#define RAW_WEIGHT_FILEPATH_END "/Users/benjaminlee/Desktop/Othello/assets/Evaluation/end ordering raw.bin"

#define WEIGHT_DIRECTORY "/Users/benjaminlee/Desktop/Othello/assets/Evaluation/"
#define TRANSCRIPT_DIRECTORY "/Users/benjaminlee/Desktop/Othello/assets/Evaluation/Transcripts/"
#define BINARY_DATASET_DIRECTORY "/Users/benjaminlee/Desktop/Othello/assets/Evaluation/Binary Datasets/"
#define COMBINED_DATASET_DIRECTORY "/Users/benjaminlee/Desktop/Othello/assets/Evaluation/Binary Datasets New/"
#define LOSS_DIRECTORY "/Users/benjaminlee/Desktop/Othello/assets/Evaluation/Losses/"
#define HASH_FILE "/Users/benjaminlee/Desktop/Othello/assets/Hash/hash.txt"


#endif //OTHELLO_CONST_H
