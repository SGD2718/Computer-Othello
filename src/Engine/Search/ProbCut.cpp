//
// Created by Benjamin Lee on 4/20/24.
//

#include "../Engine.h"
#include "../../Util.h"
#include <random>
#include <iostream>


namespace engine {
    /*constexpr double PROBCUT_A = 0.7146898310704181;
    constexpr double PROBCUT_B = -13.597964481565334;
    constexpr double PROBCUT_C = 1.3567493938287527;
    constexpr double PROBCUT_D = 0.4077673444397871;
    constexpr double PROBCUT_E = 2.1042212354434326;
    constexpr double PROBCUT_F = 4.028484368537427;
    constexpr double PROBCUT_G = 4.850073713326121;*/

    constexpr double PROBCUT_A = 0.9006402774092823;
    constexpr double PROBCUT_B = -7.8988857964929275;
    constexpr double PROBCUT_C = 0.799688863608567;
    constexpr double PROBCUT_D = 1.05409818138701;
    constexpr double PROBCUT_E = 3.1602332178243904;
    constexpr double PROBCUT_F = 3.609493332834981;
    constexpr double PROBCUT_G = 2.602946546930604;

    constexpr int SHALLOW_DEPTHS[MPC_DEPTH+1] = {
            0, 0, 0, 1, 2, 3, 2, 3, 4, 5, 4, 5, 6, 7, 6, 7, 8, 9, 8, 9, 10
    };
    constexpr double MPC_T[MAX_MPC_LEVEL+1] = {1.13, 1.55, 1.81, 2.32, 2.57, 9.99};
    int PROBCUT_ERRORS[MAX_MPC_LEVEL+1][61][MPC_DEPTH+1][MPC_DEPTH+1];

    /**
     * @brief ProbCut for selective search
     * @param node search node
     * @param depth search depth
     * @param alpha alpha value
     * @param beta beta value
     * @param legalMask legal moves
     * @param v variable to store shallow search value
     * @param passed whether the previous move was a pass
     * @param isEndSearch whether the current search is an endgame search
     * @param running termination flag
     * @return
     */
    bool Engine::probcut(engine::SearchNode *node, int depth, int alpha, int beta,
                          uint64_t legalMask, int* v, bool passed, bool isEndSearch, bool *running) {
        if (node->selectivity >= MPC_LEVEL_100)
            return false;
        auto selectivity = node->selectivity;
        node->selectivity = MPC_LEVEL_100;

        auto shallow = SHALLOW_DEPTHS[depth];
        auto error0 = PROBCUT_ERRORS[node->selectivity][node->discCount - 4][0][depth];
        auto errorShallow = PROBCUT_ERRORS[node->selectivity][node->discCount - 4][shallow][depth];

        auto eval = node->evalFeatures.mid_evaluate(node);

        if (eval >= beta + (errorShallow + error0) / 2){
            int pcBeta = beta + errorShallow;
            if (pcBeta < WIN){
                if (null_window_search(node, shallow, pcBeta - 1, passed, legalMask, false, running) >= pcBeta){
                    *v = beta;
                    if (isEndSearch)
                        *v += beta & 1;
                    node->selectivity = selectivity;
                    ++node->numProbCuts;
                    return true;
                }
            }
        }
        if (eval <= alpha - (errorShallow + error0) / 2){
            int pcAlpha = alpha - errorShallow;
            if (pcAlpha > LOSS){
                if (null_window_search(node, shallow, pcAlpha, passed, legalMask, false, running) <= pcAlpha){
                    *v = alpha;
                    if (isEndSearch)
                        *v -= alpha & 1;
                    node->selectivity = selectivity;
                    ++node->numProbCuts;
                    return true;
                }
            }
        }
        node->selectivity = selectivity;
        return false;
    }

    void Engine::probcut_init() {
        for (auto level = 0; level <= MAX_MPC_LEVEL; ++level) {
            for (auto discCount = 4; discCount <= 64; ++discCount) {
                for (auto s = 0; s < MPC_DEPTH; ++s) {
                    for (auto d = s+1; d <= MPC_DEPTH; ++d) {
                        double error = PROBCUT_A * ((double)discCount / 64.0) + PROBCUT_B * ((double)s / 60.0) + PROBCUT_C * ((double)d / 60.0);
                        error = PROBCUT_D * error * error * error + PROBCUT_E * error * error + PROBCUT_F * error + PROBCUT_G;
                        error = std::ceil(error * MPC_T[level]);
                        PROBCUT_ERRORS[level][discCount - 4][s][d] = (int)error;
                    }
                }
            }
        }
    }

    /**
     * @brief collect data for prob cut tuning
     * @param numGames number of games to play.
     * @param depths list of depths to search at.
     * @param numThreads number of threads to use.
     */
    void Engine::collect_prob_cut_data(int numGames, int numThreads) {
        std::ofstream file(MPC_DATA_FILEPATH);

        std::vector<std::thread> threads;
        std::mutex fileMtx;
        std::mutex gameNoMtx;
        std::atomic<int> g(0);
        std::atomic<bool> printLock(false);

        util::ProgressBar progressBar(numGames, "Collecting ProbCut Data", util::FRACTION);
        progressBar.print();

        threads.reserve(numThreads);
        for (int t = 0; t < numThreads; ++t) {
            threads.emplace_back([&file, &fileMtx, &gameNoMtx, &g, &printLock, &progressBar, numGames, this]() {
                std::random_device rd;
                std::mt19937 gen(rd());
                std::vector<int> values;
                values.reserve(28);
                auto* running = new bool(true);
                bool gameOver = false;
                bool passed;
                uint64_t legalMask;
                Move move;

                while (g < numGames) {
                    SearchNode node((Board()));
                    node.selectivity = MPC_LEVEL_100;

                    while (node.discCount < 64 && !gameOver) {
                        // get legal moves
                        passed = false;
                        legalMask = node.board.get_legal_moves();
                        if (!legalMask) {
                            node.pass();
                            passed = true;
                            legalMask = node.board.get_legal_moves();
                            if (!legalMask) {
                                gameOver = true;
                                break;
                            }
                        }

                        int d = 0;
                        *running = true;
                        auto timer = make_timer_thread(3, running, false, false);
                        while (*running && d + node.discCount <= 64) {
                            values.emplace_back(pv_search(&node, d++, -127, 127, passed, legalMask, false, running));
                        }

                        if (!*running)
                            values.pop_back();

                        *running = false;
                        timer.join();

                        // pick a random move
                        std::uniform_int_distribution<> dis(0, __builtin_popcountll(legalMask) - 1);
                        int moveIndex = dis(gen);
                        for (int _ = 0; _ < moveIndex; ++_)
                            legalMask &= legalMask - 1;
                        move = Move(node.board, bit::bitboard_to_coord(legalMask));
                        node.play_move(move);

                        // update everything
                        fileMtx.lock();
                        this->transpositionTable.happy_birthday(255);
                        for (d = 1; d < values.size(); ++d) {
                            for (int s = 0; s < d; ++s) {
                                file << (int)node.discCount << " " << s << " " << d << " " << values[d] - values[s] << std::endl;
                            }
                        }
                        fileMtx.unlock();
                        values.clear();
                    }

                    // update progress
                    gameNoMtx.lock();
                    ++g;
                    gameNoMtx.unlock();

                    if (!printLock) {
                        printLock = true;
                        progressBar.update(g + 1);
                        printLock = false;
                    }
                }
                delete running;
            });
        }
        for (auto &t : threads)
            t.join();
    }
}