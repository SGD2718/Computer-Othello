//
// Created by Benjamin Lee on 10/12/23.
//

#include "../Engine.h"
#include <iostream>
#include <fstream>
#include <thread>

namespace engine {
    SearchResult
    Engine::iterative_deepening_search(SearchNode *node, int maxDepth, bool pass, bool useVerbose, bool *running,
                                       bool *complete) {
        // check for game over
        if (node->board.is_terminal()) {
            node->value = node->board.get_disc_difference();
            node->move = PASS;
            return SearchResult(node);
        }
        if (maxDepth + node->discCount > 64)
            maxDepth = 64 - node->discCount;
        auto legalMask = node->board.get_legal_moves();

        assert(*running);
        std::pair<int, int> res;
        int prevValue = 0;

        auto numEmpty = 64 - node->discCount;
        std::cout << "\033[1mSearch with: " << numEmpty << " empties remaining.\033[0m" << std::endl;
        // iterate until to maximum depth
        node->selectivity = MPC_LEVEL_74;
        for (int depth = 1; *running && depth <= maxDepth; depth++) {
            if (numEmpty <= PERFECT_SEARCH_DEPTH - 2) {
                if (depth < numEmpty)
                    node->selectivity = MPC_LEVEL_99;
                else
                    node->selectivity = MPC_LEVEL_98;
            } else if (numEmpty <= PERFECT_SEARCH_DEPTH) {
                if (depth < numEmpty)
                    node->selectivity = MPC_LEVEL_93;
                else
                    node->selectivity = MPC_LEVEL_88;
            } else if (numEmpty <= PERFECT_SEARCH_DEPTH + 2 || depth < 10) {
                node->selectivity = MPC_LEVEL_88;
            } else {
                node->selectivity = MPC_LEVEL_74;
            }

            auto tmpRes = first_pv_search(node, depth, LOSS, WIN, pass, legalMask, depth == numEmpty, running);
            if (tmpRes.first != SCORE_UNDEFINED) {
                res = tmpRes;
                res.first = std::clamp(res.first, -SCORE_MAX, SCORE_MAX);
                node->value = (int)(0.1 * prevValue + 0.9 * res.first);
                prevValue = res.first;
            }

            // verbose
            if (useVerbose) {
                std::cout << "Depth " << depth << ", selectivity level " << (int)node->selectivity << ": " << node->value << std::endl;
            }

            node->stop();
        }

        node->move = Move(node->board, (uint_fast8_t)res.second);
        return SearchResult(node);
    }

    std::pair<int, int> Engine::first_pv_search(SearchNode *node, int depth, int alpha, int beta, bool pass, uint64_t legalMask, bool isEndSearch,
                                bool *running) {
        if (!*running)
            return {SCORE_UNDEFINED, I_PASS};

        // check if the game is over
        if (node->discCount == 64) {
            ++node->numNodes;
            node->depth = 0;
            return {node->board.get_end_value(64), I_PASS};
        }

        ++node->numNodes;

        if (legalMask == LEGAL_UNDEFINED)
            legalMask = node->board.get_legal_moves();

        // pass if no legal moveList
        if (legalMask == 0) {
            if (pass) { // second pass in a row means the game is over
                node->depth = 0;
                return {node->board.get_end_value(node->discCount), I_PASS};
            }
            node->pass();
            auto value = -pv_search(node, depth, -beta, -alpha, true, LEGAL_UNDEFINED, isEndSearch, running);
            node->pass(); // undo pass with another pass
            node->depth = depth;
            return {value, I_PASS};
        }

        // check if we have reached the maximum depth
        if (depth == 0)
            std::cerr << "cannot perform a search of depth 0";

        auto hash = TranspositionTable::get_hash(&node->board);
        int lower = -SCORE_MAX;
        int upper = SCORE_MAX;
        uint_fast8_t hashMoves[2] = {I_PASS, I_PASS};
        this->transpositionTable.load(node, hash, depth, &lower, &upper, hashMoves);

        int originalAlpha = alpha;

        // init move list
        std::vector<MoveEval> moveList(__builtin_popcountll(legalMask));

        int idx = 0;
        for (auto mask = bit::lsb(legalMask); legalMask; mask = bit::next_set_bit(legalMask)) {
            auto x = bit::bitboard_to_coord(mask);
            moveList[idx].move.init(x, node->board.get_flipped(x));
            if (moveList[idx].move.flip == node->board.O) {
                node->value = node->discCount + 1;
                node->depth = 1;
                return {node->value, moveList[idx].move.x};
            }
            ++idx;
        }

        this->evaluate_move_list(node, depth, alpha, beta, moveList, hashMoves, running);

        // search
        int bestValue = SCORE_UNDEFINED;
        auto bestMove = PASS;
        int value;

        for (int i = 0; i < moveList.size(); ++i) {
            swap_next_best_move(moveList, i);

            node->play_move(moveList[i].move);
            if (bestValue == SCORE_UNDEFINED) {
                value = -pv_search(node, depth - 1, -beta, -alpha, false, moveList[i].legalMask, isEndSearch, running);
            } else {
                value = -null_window_search(node, depth - 1, -alpha - 1, false, moveList[i].legalMask, isEndSearch, running);
                if (alpha < value && value < beta)
                    value = -pv_search(node, depth - 1, -beta, -alpha, false, moveList[i].legalMask, isEndSearch, running);
            }
            node->undo_move(moveList[i].move);

            if (value > bestValue) {
                bestValue = value;
                bestMove = moveList[i].move;

                if (value > alpha) {
                    if (value >= beta)
                        break;
                    alpha = value;
                }
            }
        }

        if (*running) {
            this->transpositionTable.store(node, hash, depth, originalAlpha, beta, bestValue, bestMove.x);
            node->depth = depth;
            return {bestValue, bestMove.x};
        }
        return {SCORE_UNDEFINED, I_PASS};
    }

    int
    Engine::pv_search(SearchNode *node, int depth, int alpha, int beta, bool pass, uint64_t legalMask, bool isEndSearch, bool *running) {
        if (!*running)
            return SCORE_UNDEFINED;

        if (!isEndSearch) {
            if (depth == 1)
                return alpha_beta1(node, alpha, beta, pass, legalMask);
            if (depth == 0) {
                ++node->numNodes;
                return node->evalFeatures.mid_evaluate(node);
            }
        }
        if (beta - alpha == 1)
            return null_window_search(node, depth, alpha, pass, legalMask, isEndSearch, running);

        ++node->numNodes;

        if (legalMask == LEGAL_UNDEFINED)
            legalMask = node->board.get_legal_moves();

        if (legalMask == 0) {
            if (pass)
                return node->board.get_end_value(node->discCount);
            node->pass();
            auto value = -pv_search(node, depth, -beta, -alpha, true, LEGAL_UNDEFINED, isEndSearch, running);
            node->pass();
            return value;
        }

        auto hash = TranspositionTable::get_hash(&node->board);
        int lower = -SCORE_MAX;
        int upper = SCORE_MAX;
        uint_fast8_t hashMoves[2] = {I_PASS, I_PASS};
        this->transpositionTable.load(node, hash, depth, &lower, &upper, hashMoves);

        if (lower == upper) return lower;
        if (lower >= beta) return lower;
        if (upper <= alpha) return upper;
        if (lower > alpha) alpha = lower;
        if (upper < beta) beta = upper;
        int originalAlpha = alpha;

        std::vector<MoveEval> moveList(__builtin_popcountll(legalMask));

        int idx = 0;
        for (auto mask = bit::lsb(legalMask); legalMask; mask = bit::next_set_bit(legalMask)) {
            auto x = bit::bitboard_to_coord(mask);
            moveList[idx].move.init(x, node->board.get_flipped(x));
            if (moveList[idx].move.flip == node->board.O)
                return node->discCount + 1;
            ++idx;
        }

        int bestValue = SCORE_UNDEFINED;
        int cutoffs = 0;
        #if USE_ETC
            if (depth >= ETC_DEPTH && etc(node, moveList, depth, &alpha, beta, &bestValue, &cutoffs))
                return bestValue;
        #endif
        #if USE_MPC
            if (depth <= MPC_DEPTH && probcut(node, depth, alpha, beta, legalMask, &bestValue, pass, isEndSearch, running))
                return bestValue;
        #endif

        this->evaluate_move_list(node, depth, alpha, beta, moveList, hashMoves, running);

        uint_fast8_t bestMove = I_PASS;
        int value;

        for (int i = 0; i < moveList.size() - cutoffs; ++i) {
            swap_next_best_move(moveList, i);

            #if USE_ETC
                if (moveList[i].move.flip == 0ULL)
                    break;
            #endif

            node->play_move(moveList[i].move);
            if (bestValue == SCORE_UNDEFINED) {
                value = -pv_search(node, depth - 1, -beta, -alpha, false, moveList[i].legalMask, isEndSearch, running);
            } else {
                value = -null_window_search(node, depth - 1, -alpha - 1, false, moveList[i].legalMask, isEndSearch, running);
                if (alpha < value && value < beta)
                    value = -pv_search(node, depth - 1, -beta, -alpha, false, moveList[i].legalMask, isEndSearch, running);
            }
            node->undo_move(moveList[i].move);

            if (value > bestValue) {
                bestValue = value;
                bestMove = moveList[i].move.x;
                if (value > alpha) {
                    if (value >= beta)
                        break;
                    alpha = value;
                }
            }
        }

        if (*running)
            this->transpositionTable.store(node, hash, depth, originalAlpha, beta, bestValue, bestMove);

        return bestValue;
    }

    int Engine::alpha_beta1(SearchNode *node, int alpha, int beta, bool pass, uint64_t legalMask) {
        ++node->numNodes;

        if (legalMask == LEGAL_UNDEFINED)
            legalMask = node->board.get_legal_moves();

        if (legalMask == 0) {
            if (pass)
                return node->board.get_end_value(node->discCount);
            node->pass();
            auto value = -alpha_beta1(node, -beta, -alpha, true, LEGAL_UNDEFINED);
            node->pass();
            return value;
        }

        int bestValue = SCORE_UNDEFINED;

        for (auto x = bit::first_set_idx(legalMask); legalMask; x = bit::next_set_idx(legalMask)) {
            auto move = Move((uint_fast8_t)x, node->board.get_flipped(x));
            node->play_move(move);
            auto value = -node->evalFeatures.mid_evaluate(node);
            ++node->numNodes;
            node->undo_move(move);

            if (value > bestValue) {
                if (value >= beta)
                    return value;
                bestValue = value;
            }
        }

        return bestValue;
    }
}