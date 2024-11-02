//
// Created by Benjamin Lee on 1/1/24.
//

#include "../Engine.h"
#include <iostream>

namespace engine {

    constexpr int W_CELL_MID = 7;
    constexpr int W_MOBILITY_MID = 37;
    constexpr int W_POTENTIAL_MOBILITY_MID = 11;
    constexpr int W_VALUE_MID = 289;
    constexpr int W_DEPTH_MID = 92;

    constexpr int OFFSET_ALPHA_MID = 12;
    constexpr int OFFSET_BETA_MID = 12;

    constexpr int W_MOBILITY_NWS = 17;
    constexpr int W_POTENTIAL_MOBILITY_NWS = 19;
    constexpr int W_VALUE_NWS = 14;
    constexpr int W_DEPTH_NWS = 11;

    constexpr int OFFSET_ALPHA_NWS = 12;
    constexpr int OFFSET_BETA_NWS = 6;

    constexpr int W_MOBILITY_END = 41;
    constexpr int W_VALUE_END = 6;
    constexpr int W_PARITY_END = 4;

    constexpr int W_MOBILITY_END_NWS = 32;
    //constexpr int W_PARITY_END_NWS = 0;
    constexpr int W_VALUE_END_NWS = 6;

    /** Evaluate a move with position weight, mobility, potential mobility, and shallow searches.
     * Then, add it to the move list.
     *
     * @param board: the current board
     * @param x: the location of the move
     * @param depth: the depth of the shallow search
     * @param alpha: lower value bound
     * @param beta: upper value bound
     * @param moveEval: the move eval pair
     * @param running: pointer to the running flag
     */
    void Engine::move_evaluate(SearchNode *node, int depth, int alpha, int beta, MoveEval *moveEval, bool *running) {
        node->play_move(moveEval->move);
            moveEval->legalMask = node->board.get_legal_moves();

            moveEval->value = eval::CELL_WEIGHTS[moveEval->move.x] * W_CELL_MID;
            moveEval->value -= eval::get_weighted_mobility(moveEval->legalMask) * W_MOBILITY_MID;
            moveEval->value -= eval::get_potential_mobility(node->board.P, node->board.O) * W_POTENTIAL_MOBILITY_MID;

            switch (depth) {
                case 0:
                    moveEval->value -= node->evalFeatures.mid_evaluate(node) * W_VALUE_MID;
                    break;
                case 1:
                    moveEval->value -= alpha_beta1(node, alpha, beta, false, moveEval->legalMask) *
                                       (W_VALUE_MID + W_DEPTH_MID);
                    break;
                default: {
                    auto selectivity = node->selectivity;
                    node->selectivity = MPC_LEVEL_88;
                    moveEval->value -=
                            this->pv_search(node, depth, alpha, beta, false, moveEval->legalMask, false, running) *
                            (W_VALUE_MID + W_DEPTH_MID * depth);
                    node->selectivity = selectivity;
                }
            }
        node->undo_move(moveEval->move);
    }

    /** Evaluate a move for mid-game null window search
     *
     * @param board: the current board
     * @param x: the location of the move
     * @param depth: the depth of the shallow search
     * @param alpha: lower value bound
     * @param beta: upper value bound
     * @param moveEval: the move eval pair
     * @param running: pointer to the running flag
     */
    void Engine::move_evaluate_nws(SearchNode *node, int depth, int alpha, int beta, MoveEval *moveEval, bool* running) {
        node->play_move(moveEval->move);
            moveEval->legalMask = node->board.get_legal_moves();

            moveEval->value = -eval::get_weighted_mobility(moveEval->legalMask) * W_MOBILITY_NWS;
            moveEval->value -= eval::get_potential_mobility(node->board.P, node->board.O) * W_POTENTIAL_MOBILITY_NWS;

            switch (depth) {
                case 0:
                    moveEval->value -= node->evalFeatures.mid_evaluate(node) * W_VALUE_NWS;
                    break;
                case 1:
                    moveEval->value -=
                            alpha_beta1(node, alpha, beta, false, moveEval->legalMask) * (W_VALUE_NWS + W_DEPTH_NWS);
                    break;
                default: {
                    auto selectivity = node->selectivity;
                    node->selectivity = MPC_LEVEL_88;
                    moveEval->value -=
                            this->pv_search(node, depth, alpha, beta, false, moveEval->legalMask, false, running) *
                            (W_VALUE_NWS + W_DEPTH_NWS * depth);
                    node->selectivity = selectivity;
                }
            }
        node->undo_move(moveEval->move);
    }

    /** Evaluate a move for endgame
     *
     * @param board: the current board
     * @param x: the location of the move
     * @param moveEval: the move eval pair
     */
    void Engine::move_evaluate_end(SearchNode *node, MoveEval *moveEval) {
        moveEval->value = 0; //eval::CELL_WEIGHTS[moveEval->move.x];
        if (node->parity & eval::PARITY_BITS[moveEval->move.x])
            moveEval->value += W_PARITY_END;

        node->play_move_end(moveEval->move);
            moveEval->legalMask = node->board.get_legal_moves();
            moveEval->value -= __builtin_popcountll(moveEval->legalMask) * W_MOBILITY_END;
            moveEval->value -= node->evalFeatures.end_evaluate_move_ordering(node) * W_VALUE_END;
        node->undo_move_end(moveEval->move);
    }

    /** Evaluate a move for endgame
     *
     * @param board: the current board
     * @param x: the location of the move
     * @param moveEval: the move eval pair
     */
    void Engine::move_evaluate_end_nws(SearchNode *node, MoveEval *moveEval) {
        moveEval->value = 0;
        node->play_move_end(moveEval->move);
            moveEval->legalMask = node->board.get_legal_moves();
            moveEval->value -= __builtin_popcountll(moveEval->legalMask) * W_MOBILITY_END_NWS;
            moveEval->value -= node->evalFeatures.end_evaluate_move_ordering(node) * W_VALUE_END_NWS;
        node->undo_move_end(moveEval->move);
    }

    /** Evaluate a move for endgame
     *
     * @param node: search node
     * @param x: the location of the move
     * @param moveEval: the move eval pair
     */
    void Engine::move_evaluate_end_fast(SearchNode *node, MoveEval *moveEval) {
        moveEval->value = eval::CELL_WEIGHTS[moveEval->move.x];
        if (node->parity & eval::PARITY_BITS[moveEval->move.x])
            moveEval->value += W_PARITY_END;

        node->play_move_end(moveEval->move);
            moveEval->legalMask = node->board.get_legal_moves();
            moveEval->value -= __builtin_popcountll(moveEval->legalMask) * W_MOBILITY_END;

        node->undo_move_end(moveEval->move);
    }

    /** Sort moves with heuristic move evaluation
     *
     * @param board: the current state of the game
     * @param depth: the search depth remaining
     * @param alpha: the current alpha value
     * @param beta: the current beta value
     * @param legalMask: bitboard of legalMask moves
     * @param moveList: the move list
     * @param hashMoves: the hash moves
     * @param running: pointer to the running flag
     */
    void Engine::evaluate_move_list(SearchNode *node, int depth, int alpha, int beta, std::vector<MoveEval> &moveList, const uint_fast8_t hashMoves[], bool *running) {
        int evalDepth = depth >> 3;
        if (depth >= 16)
            evalDepth += (depth - 14) >> 1;
        int evalAlpha = -std::min(64, beta + OFFSET_BETA_MID);
        int evalBeta = -std::max(-64, alpha - OFFSET_ALPHA_MID);

        for (auto &moveEval : moveList) {
            if (moveEval.move.x == hashMoves[0])
                moveEval.value = FIRST_HASH_MOVE_SCORE;
            else if (moveEval.move.x == hashMoves[1])
                moveEval.value = SECOND_HASH_MOVE_SCORE;
            else
                this->move_evaluate(node, evalDepth, evalAlpha, evalBeta, &moveEval, running);
        }
    }

    /**
     * Evaluate move list for mid-game without hash moves
     * @param node search node
     * @param depth search depth
     * @param alpha alpha value
     * @param beta beta value
     * @param moveList move list
     * @param running running flag
     */
    void Engine::evaluate_move_list(SearchNode *node, int depth, int alpha, int beta, std::vector<MoveEval> &moveList, bool *running) {
        int evalDepth = depth >> 3; // shallow search depth
        if (depth >= 16) evalDepth += (depth - 14) >> 1;
        int evalAlpha = -std::min(64, beta + OFFSET_BETA_MID);
        int evalBeta = -std::max(-64, alpha - OFFSET_ALPHA_MID);
        for (auto & moveEval : moveList) {
            this->move_evaluate(node, evalDepth, evalAlpha, evalBeta, &moveEval, running);
        }
    }

    /** Evaluate move list for mid-game null window search
     *
     * @param board: the current state of the game
     * @param depth: the search depth remaining
     * @param alpha: the current alpha value
     * @param legal: bitmask of legal moves
     * @param moves: vector of legal moves (to be filled)
     * @param result: search result
     * @param running: pointer to the running flag
     */
    void Engine::evaluate_move_list_nws(SearchNode *node, int depth, int alpha, std::vector<MoveEval> &moveList, uint_fast8_t hashMoves[], bool *running) {
        depth >>= 4; // shallow search depth

        int evalAlpha = -std::min(64, alpha + OFFSET_BETA_NWS);
        int evalBeta = -std::max(-64, alpha - OFFSET_ALPHA_NWS);

        for (auto & moveEval : moveList) {
            if (moveEval.move.x == hashMoves[0])
                moveEval.value = FIRST_HASH_MOVE_SCORE;
            else if (moveEval.move.x == hashMoves[1])
                moveEval.value = SECOND_HASH_MOVE_SCORE;
            else
                this->move_evaluate_nws(node, depth, evalAlpha, evalBeta, &moveEval, running);
        }
    }

    /** evaluate move list for endgame
     *
     * @param board: the current state of the game
     * @param depth: the search depth remaining
     * @param alpha: the current alpha value
     * @param beta: the current beta value
     * @param legal: bitmask of legal moves
     * @param moves: vector of legal moves (to be filled)
     * @param result: search result
     */
    void Engine::evaluate_move_list_end(SearchNode *node, std::vector<MoveEval> &moveList) {
        for (auto & moveEval : moveList) {
            this->move_evaluate_end(node, &moveEval);
        }
    }

    /** evaluate move list for endgame
     *
     * @param board: the current state of the game
     * @param depth: the search depth remaining
     * @param alpha: the current alpha value
     * @param beta: the current beta value
     * @param legal: bitmask of legal moves
     * @param moves: vector of legal moves (to be filled)
     * @param result: search result
     */
    void Engine::evaluate_move_list_end_nws(engine::SearchNode *node, std::vector<MoveEval> &moveList) {
        for (auto & moveEval : moveList) {
            this->move_evaluate_end_nws(node, &moveEval);
        }
    }

    void Engine::evaluate_move_list_end_fast(SearchNode *node, std::vector<MoveEval> &moveList) {
        for (auto & moveEval : moveList) {
            this->move_evaluate_end_fast(node, &moveEval);
        }
    }

    /** Swap the best move to the front of the move list
     *
     * @param moveList: the move list
     * @param index: the index of the top of the list
     */
    void Engine::swap_next_best_move(std::vector<MoveEval> &moveList, int index) {
        int bestValue = SCORE_UNDEFINED;
        int bestIndex = index;

        for (int i = index; i < moveList.size(); ++i) {
            if (moveList[i].value > bestValue) {
                bestValue = moveList[i].value;
                bestIndex = i;
            }
        }
        if (index != bestIndex) {
            auto tmp = moveList[index];
            moveList[index] = moveList[bestIndex];
            moveList[bestIndex] = tmp;
        }
    }
}