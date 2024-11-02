//
// Created by Benjamin Lee on 2/24/24.
//

#include "../Engine.h"

namespace engine {
    int
    Engine::null_window_search(SearchNode *node, int depth, int alpha, bool pass, uint64_t legalMask, bool isEndSearch, bool *running) {
        if (!*running)
            return SCORE_UNDEFINED;

        // check if we have reached the maximum depth
        if (!isEndSearch) {
            if (depth == 1)
                return alpha_beta_nws1(node, alpha, pass, legalMask);
            if (depth == 0) {
                ++node->numNodes;
                return node->evalFeatures.mid_evaluate(node);
            }
        }
        if (isEndSearch && depth <= MID_TO_END_DEPTH)
            return end_search_nws(node, alpha, pass, legalMask, running);

        ++node->numNodes;

        if (legalMask == LEGAL_UNDEFINED)
            legalMask = node->board.get_legal_moves();

        // pass if no legal moves
        if (legalMask == 0) {
            if (pass)
                return node->board.get_end_value(node->discCount);

            node->pass();
            auto value = -null_window_search(node, depth, -alpha-1, true, LEGAL_UNDEFINED, isEndSearch, running);
            node->pass(); // undo pass with another pass
            return value;
        }

        // hash lookup
        auto hash = TranspositionTable::get_hash(&node->board);
        int lower = -SCORE_MAX;
        int upper = SCORE_MAX;
        uint_fast8_t hashMoves[2] = {I_PASS, I_PASS};
        this->transpositionTable.load(node, hash, depth, &lower, &upper, hashMoves);

        if (lower == upper) return lower;
        if (lower > alpha) return lower;
        if (upper <= alpha) return upper;

        int v = SCORE_UNDEFINED;

        #if USE_MPC
            if (depth <= MPC_DEPTH && probcut(node, depth, alpha, alpha+1, legalMask, &v, pass, isEndSearch, running)) {
                return v;
            }
        #endif

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

        #if USE_ETC
            int cutoffs = 0;
            if (depth >= ETC_DEPTH && etc_nws(node, moveList, depth, alpha, &v, &cutoffs)) {
                return v;
            }
        #endif

        // evaluate move list
        this->evaluate_move_list_nws(node, depth, alpha, moveList, hashMoves, running);

        // search
        auto beta = alpha + 1;
        uint_fast8_t bestMove = I_PASS;
        int g;

        #if USE_ETC
            for (int i = 0; i < moveList.size() - cutoffs; ++i) {
        #else
            for (int i = 0; i < moveList.size(); ++i) {
        #endif
            swap_next_best_move(moveList, i);

            // etc
            #if USE_ETC
                if (moveList[i].move.flip == 0ULL)
                    break;
            #endif

            node->play_move(moveList[i].move);
                g = -null_window_search(node, depth - 1, -beta, false, moveList[i].legalMask, isEndSearch, running);
            node->undo_move(moveList[i].move);

            if (g > v) {
                v = g;
                bestMove = moveList[i].move.x;

                if (g >= beta)
                    break;
            }
        }

        if (*running) {
            this->transpositionTable.store(node, hash, depth, alpha, beta, v, bestMove);
        }

        return v;
    }

    int Engine::alpha_beta_nws1(engine::SearchNode *node, int alpha, bool pass, uint64_t legalMask) {
        ++node->numNodes;

        // check if the game is over, or we have reached the maximum depth
        if (node->discCount == 64)
            return node->board.get_end_value(64);
        if (legalMask == LEGAL_UNDEFINED)
            legalMask = node->board.get_legal_moves();

        // pass if no legal moves
        if (legalMask == 0) {
            if (pass)
                return node->board.get_end_value(node->discCount);
            node->pass();
            auto value = -alpha_beta_nws1(node, -1-alpha, true, LEGAL_UNDEFINED);
            node->pass(); // undo pass with another pass
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
                if (value > alpha)
                    return value;
                bestValue = value;
            }
        }
        return bestValue;
    }
} // engine