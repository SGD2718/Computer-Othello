//
// Created by Benjamin Lee on 2/28/24.
//

#include "../Engine.h"

namespace engine {

    /**
     * @brief Enhanced Transposition Cutoff (ETC)
     * @param node  The current search node
     * @param moveList  The list of moves to search
     * @param depth  The current search depth
     * @param alpha  The current alpha value
     * @param beta   The current beta value
     * @param v     The current value
     * @param cutoffs  number of cutoffs
     * @return True if the search was cutoff, false otherwise
     *
     * From Nyanyan's Egaroucid Othello engine
     */
    bool Engine::etc(engine::SearchNode *node, std::vector<MoveEval> &moveList, int depth, int *alpha, const int beta,
                     int *v, int *cutoffs) {
        *cutoffs = 0;
        int l, u;
        for (auto &moveEval : moveList) {
            l = -SCORE_MAX;
            u = SCORE_MAX;
            node->play_move(moveEval.move);
            this->transpositionTable.load_bounds(node, TranspositionTable::get_hash(&node->board), depth - 1, &l, &u);
            node->undo_move(moveEval.move);

            // -u is lower bound from current player's perspective
            if (-u >= beta) { // fail high at current node
                *v = -u;
                ++node->numETCCuts;
                return true;
            }

            // -alpha is the upper bound from opponent's perspective
            if (-(*alpha) <= l) { // fail high at child node
                if (*v < -l)
                    *v = -l;
                moveEval.move.flip = 0ULL; // invalidate the move
                ++node->numETCCuts;
                ++(*cutoffs);
            } else if (-beta < u && u < -(*alpha) && *v < -u) { // within bounds
                *v = -u;
                if (*alpha < -u)
                    *alpha = -u;
            }
            if (*alpha >= beta) {
                // fail high
                ++node->numETCCuts;
                return true;
            }
        }

        return false;
    }

    /**
     * @brief Enhanced Transposition Cutoff (ETC)
     * @param node  The current search node
     * @param moveList  The list of moves to search
     * @param depth  The current search depth
     * @param alpha  The current alpha value
     * @param v     The current value
     * @param cutoffs  number of cutoffs
     * @return True if the search was cutoff, false otherwise
     *
     * From Nyanyan's Egaroucid Othello engine
     */
    bool Engine::etc_nws(engine::SearchNode *node, std::vector<MoveEval> &moveList, int depth, int alpha, int *v, int *cutoffs) {
        *cutoffs = 0;
        int l, u;
        for (auto &moveEval : moveList) {
            l = -SCORE_MAX;
            u = SCORE_MAX;
            node->play_move(moveEval.move);
            this->transpositionTable.load_bounds(node, TranspositionTable::get_hash(&node->board), depth - 1, &l, &u);
            node->undo_move(moveEval.move);

            // -u is lower bound from current player's perspective
            if (alpha < -u) { // fail high at current node
                *v = -u;
                ++node->numETCCuts;
                return true;
            }

            // -alpha is the upper bound from opponent's perspective
            if (-alpha <= l) { // fail high at child node
                if (*v < -l)
                    *v = -l;
                moveEval.move.flip = 0ULL; // invalidate the move
                ++node->numETCCuts;
                ++(*cutoffs);
            }
        }

        return false;
    }

} // engine::eval