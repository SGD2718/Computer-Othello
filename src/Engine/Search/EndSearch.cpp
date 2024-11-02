//
// Created by Benjamin Lee on 2/19/24.
//

#include "../Engine.h"
#include <iostream>

namespace engine {
    int Engine::end_search_nws(engine::SearchNode *node, int alpha, bool pass, uint64_t legalMask, bool *running) {
        if (!*running) return SCORE_UNDEFINED;

        auto numEmpty = 64 - node->discCount;

        /*if (numEmpty == 2) {
            auto empty = ~(node->board.P | node->board.O);
            uint_fast8_t x1 = bit::bitboard_to_coord(empty);
            uint_fast8_t x2 = bit::bitboard_to_coord(empty & (empty - 1));
            return last2(node, alpha, alpha + 1, x1, x2, node->board);
        }*/
        if (numEmpty == 0) {
            return node->board.get_end_value(64);
        }
        ++node->numNodes;

        if (legalMask == LEGAL_UNDEFINED)
            legalMask = node->board.get_legal_moves();

        // pass if no legal moves
        if (legalMask == 0) {
            if (pass)
                return node->board.get_end_value(node->discCount);
            node->pass();
            auto value = -end_search_nws(node, -alpha-1, true, LEGAL_UNDEFINED, running);
            node->pass(); // undo pass with another pass
            return value;
        }

        // hash lookup
        auto hash = TranspositionTable::get_hash(&node->board);
        int lower = -SCORE_MAX;
        int upper = SCORE_MAX;
        uint_fast8_t hashMoves[2] = {I_PASS, I_PASS};
        this->transpositionTable.load(node, hash,  numEmpty, &lower, &upper, hashMoves);

        if (lower == upper) return lower;
        if (lower > alpha) return lower;
        if (upper <= alpha) return upper;

        int bestValue = SCORE_UNDEFINED;
        #if USE_MPC
            if (numEmpty <= MPC_DEPTH && probcut(node, numEmpty, alpha, alpha+1, legalMask, &bestValue, pass, true, running))
                return bestValue;
        #endif

        // start with hash moves
        auto bestMove = I_PASS;
        auto beta = alpha + 1;
        int value;
        Move move;

        for (int i = 0; i < 2; ++i) {
            if (hashMoves[i] == I_PASS)
                break;
            if ((legalMask >> hashMoves[i]) & 1) {
                move.init(hashMoves[i], node->board.get_flipped(hashMoves[i]));

                node->play_move_end(move);
                value = -end_search_nws(node, -beta, false, LEGAL_UNDEFINED, running);
                node->undo_move_end(move);
                legalMask ^= 1ULL << hashMoves[i];

                if (value > bestValue) {
                    bestValue = value;
                    bestMove = hashMoves[i];

                    if (value > alpha)
                        break;
                }
            }
        }
        if (alpha < beta && legalMask) {

            // init move list
            std::vector<MoveEval> moveList(__builtin_popcountll(legalMask));

            int idx = 0;
            for (auto mask = bit::lsb(legalMask); legalMask; mask = bit::next_set_bit(legalMask)) {
                auto x = bit::bitboard_to_coord(mask);
                moveList[idx].move.init(x, node->board.get_flipped(x));
                if (moveList[idx].move.flip == node->board.O)
                    return node->discCount + 1;
                ++idx;
            }

            // evaluate move list
            this->evaluate_move_list_end_nws(node, moveList);

            for (int i = 0; i < moveList.size(); ++i) {
                // play the move
                swap_next_best_move(moveList, i);

                node->play_move_end(moveList[i].move);
                value = -end_search_nws(node, -beta, false, moveList[i].legalMask, running);
                node->undo_move_end(moveList[i].move);

                // update best move and value
                if (value > bestValue && value <= SCORE_MAX) {
                    bestValue = value;
                    bestMove = moveList[i].move.x;

                    if (value > alpha)
                        break;
                }
            }
        }

        if (*running) {
            transpositionTable.store(node, hash, numEmpty, alpha, beta, bestValue, bestMove);
        }

        return bestValue;
    }


}