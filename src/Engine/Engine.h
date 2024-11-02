//
// Created by Benjamin Lee on 10/2/23.
//

#ifndef OTHELLO_ENGINE_H
#define OTHELLO_ENGINE_H

#include "../Game/Game.h"
#include <atomic>
#include <chrono>
#include "../Const.h"
#include "Masks.h"
#include "Search/SearchStructs.h"
#include "Evaluation/Evaluation.h"
#include "Evaluation/StaticEvaluations.h"
#include "Search/TranspositionTable.h"
#include "../Bit.h"
#include "../Util.h"

namespace engine {
    class Engine {
    public:
        Engine() = default;

        enum Verbose: int {
            ALL = 1,         // search stats, evaluation at each iteration, final value, best move
            PROGRESS = 2,    // evaluation at each iteration, final value, best move
            BEST = 4,        // final value, best move
            MOVE = 8,        // best move
            VALUE = 16,      // final value
            STATS = 32,      // search stats
            SUMMARY = 64,    // depth, final value, best move
            RESULTS = 128,   // search stats, final value, best move
            NONE = 0         // nothing
        };

        SearchResult search(const Game &game, double maxTime = 3, Verbose verbose = Verbose::ALL);
        SearchResult search(SearchNode* node, bool passed, double maxTime = 3, Verbose verbose = Verbose::ALL);
        SearchTask* search_task(const Game &game, Verbose verbose = Verbose::ALL);
        void continue_search_task(SearchTask* task, bool passed, Verbose verbose = Verbose::ALL);
        void continue_search_task_timed(engine::SearchTask *task, bool passed, double maxTime = 3, engine::Engine::Verbose verbose = Verbose::ALL);

        /** call this function after a move has been played to update the transposition table's age */
        inline void update() {
            this->transpositionTable.happy_birthday();
        }

        inline void clear_transposition_table() {
            this->transpositionTable.clear();
        }

        SearchResult search_to_depth(const Game &game, int depth, Verbose verbose = Verbose::ALL, double maxTime = 86400);

        static std::thread make_timer_thread(double duration, bool*& running, bool deleteRunningOnCompletion = true, bool waitToStart = true);
        static void print_stats(SearchResult& result, Verbose verbose);
        void collect_prob_cut_data(int numGames, int numThreads = 1);
        static void probcut_init();

    private:
        SearchResult iterative_deepening_search(SearchNode* node, int maxDepth, bool pass, bool useVerbose, bool* running, bool* completed);

        std::pair<int, int> first_pv_search(SearchNode* node, int depth, int alpha, int beta, bool pass, uint64_t legalMask, bool isEndSearch, bool* running);
        int pv_search(SearchNode* node, int depth, int alpha, int beta, bool pass, uint64_t legalMask, bool isEndSearch, bool* running);
        int alpha_beta1(SearchNode* node, int alpha, int beta, bool pass, uint64_t legalMask);

        int null_window_search(SearchNode* node, int depth, int alpha, bool pass, uint64_t legalMask, bool isEndSearch, bool* running);
        int alpha_beta_nws1(SearchNode* node, int alpha, bool pass, uint64_t legalMask);

        int end_search_nws(SearchNode* node, int alpha, bool pass, uint64_t legalMask, bool* running);

        int last4(SearchNode* node, int alpha, int beta);
        int last3(SearchNode* node, int alpha, int beta, uint_fast8_t x1, uint_fast8_t x2, uint_fast8_t x3, int sort3, Board board);
        int last2(SearchNode* node, int alpha, int beta, uint_fast8_t x1, uint_fast8_t x2, Board board);
        int last1(SearchNode* node, uint_fast8_t x, uint64_t P);

        void evaluate_move_list(SearchNode* node, int depth, int alpha, int beta, std::vector<MoveEval>& moveList,
                                const uint_fast8_t hashMoves[], bool* running);
        void evaluate_move_list(SearchNode* node, int depth, int alpha, int beta, std::vector<MoveEval>& moveList, bool* running);
        void evaluate_move_list_nws(SearchNode* node, int depth, int alpha, std::vector<MoveEval>& moveList, uint_fast8_t hashMoves[], bool *running);
        void evaluate_move_list_end(SearchNode* node, std::vector<MoveEval>& moveList);
        void evaluate_move_list_end_nws(SearchNode* node, std::vector<MoveEval>& moveList);
        void evaluate_move_list_end_fast(SearchNode* node, std::vector<MoveEval>& moveList);

        void move_evaluate(SearchNode* node, int depth, int alpha, int beta, MoveEval* moveEval, bool* running);
        void move_evaluate_nws(SearchNode* node, int depth, int alpha, int beta, MoveEval* moveEval, bool *running);
        void move_evaluate_end(SearchNode* node, MoveEval* moveEval);
        void move_evaluate_end_nws(SearchNode* node, MoveEval* moveEval);
        void move_evaluate_end_fast(SearchNode* node, MoveEval* moveEval);

        static void swap_next_best_move(std::vector<MoveEval>& moveList, int i);

        bool probcut(SearchNode *node, int depth, int alpha, int beta, uint64_t legalMask, int* v, bool passed, bool isEndSearch, bool* running);

        bool etc(SearchNode* node, std::vector<MoveEval>& moveList, int depth, int* alpha, int beta, int* v, int* cutoffs);
        bool etc_nws(SearchNode* node, std::vector<MoveEval>& moveList, int depth, int alpha, int* v, int* cutoffs);

        TranspositionTable transpositionTable;
    };
}


#endif //OTHELLO_ENGINE_H
