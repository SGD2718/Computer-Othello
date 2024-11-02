//
// Created by Benjamin Lee on 1/30/24.
//

#ifndef OTHELLO_SEARCHSTRUCTS_H
#define OTHELLO_SEARCHSTRUCTS_H

#include "../Evaluation/StaticEvaluations.h"
#include "../../Game/Game.h"
#include "../Evaluation/Evaluation.h"
#include <thread>
#include <QThread>

namespace engine {

    struct MoveEval {
        MoveEval() = default;
        MoveEval(const Move &move, int value, uint64_t legalMask) :
                move(move),
                value(value),
                legalMask(legalMask) {}

        Move move = PASS;
        int value = SCORE_UNDEFINED;
        uint64_t legalMask = LEGAL_UNDEFINED;
    };

    struct SearchNode {
        explicit SearchNode(const Board &board) :
                board(board),
                discCount(board.get_disc_count()),
                rootDiscCount(board.get_disc_count()),
                startTime(std::chrono::high_resolution_clock::now()),
                endTime(std::chrono::high_resolution_clock::now() - std::chrono::milliseconds(1000)),
                evalFeatures(&board),
                move(MOVE_UNDEFINED) {
            auto empty = ~(board.P | board.O);
            this->parity  =  __builtin_popcountll(empty & 0x000000000F0F0F0FULL) & 1;
            this->parity |= (__builtin_popcountll(empty & 0x00000000F0F0F0F0ULL) & 1) << 1;
            this->parity |= (__builtin_popcountll(empty & 0x0F0F0F0F00000000ULL) & 1) << 2;
            this->parity |= (__builtin_popcountll(empty & 0xF0F0F0F000000000ULL) & 1) << 3;
        }

        inline void start() {
            this->startTime = std::chrono::high_resolution_clock::now();
            this->endTime = this->startTime - std::chrono::milliseconds(1000);
        }

        inline void stop() {
            this->endTime = std::chrono::high_resolution_clock::now();
        }

        [[nodiscard]] inline long long get_duration() const {
            if (this->endTime <= this->startTime)
                return std::chrono::duration_cast<std::chrono::milliseconds>(
                        std::chrono::high_resolution_clock::now() - this->startTime).count();
            return std::chrono::duration_cast<std::chrono::milliseconds>(this->endTime - this->startTime).count();
        }

        inline void play_move(const Move &flip) {
            this->board.play_move(flip);
            this->evalFeatures.play_move(&flip);
            ++this->discCount;
            parity ^= eval::PARITY_BITS[flip.x];
        }

        inline void play_move_end(const Move &flip) {
            this->board.play_move(flip);
            this->evalFeatures.play_move_end(&flip);
            ++this->discCount;
            parity ^= eval::PARITY_BITS[flip.x];
        }

        inline void undo_move(const Move &flip) {
            this->board.undo_move(flip);
            --this->discCount;
            parity ^= eval::PARITY_BITS[flip.x];
            this->evalFeatures.undo_move(&flip);
        }

        inline void undo_move_end(const Move &flip) {
            this->board.undo_move(flip);
            this->evalFeatures.undo_move_end(&flip);
            --this->discCount;
            parity ^= eval::PARITY_BITS[flip.x];
        }

        inline void pass() {
            this->board.pass();
            this->evalFeatures.pass();
        }

        Move move = PASS;               // the best move to play from the position
        int value = 0;                  // value of the current state
        int depth = 0;                  // maximum search depth reached
        int discCount = 4;              // number of discs on the board
        uint_fast8_t selectivity = 5;   // selectivity level of the node
        Board board;                    // board
        uint_fast8_t parity = 0;        // parity of the node
        bool isEndgame = false;         // whether the node is an endgame node

        std::chrono::time_point<std::chrono::high_resolution_clock> startTime;  // time the search started
        std::chrono::time_point<std::chrono::high_resolution_clock> endTime;    // time the search ended. will be less than start time if the search is not finished

        int rootDiscCount = 4;   // number of discs on the board at the root node
        long long numNodes = 0;  // number of nodes searched
        long long numProbCuts = 0;  // number of nodes searched
        long long numETCCuts = 0;  // number of nodes searched
        eval::EvaluationFeatures evalFeatures;
    };

    struct SearchResult {
        SearchResult() = default;

        explicit SearchResult(const Move &move = PASS, int value = 0, int depth = 0, long long numNodes = 0,
                              long long nps = 0) :
                move(move),
                value(value),
                depth(depth),
                numNodes(numNodes),
                nps(nps) {}

        explicit SearchResult(SearchNode *searchNode) :
                move(searchNode->move),
                value(searchNode->value),
                depth(searchNode->depth),
                numNodes(searchNode->numNodes),
                numMPCCuts(searchNode->numProbCuts),
                numETCCuts(searchNode->numETCCuts),
                nps(searchNode->numNodes * 1000 / searchNode->get_duration()),
                duration(searchNode->get_duration()) {}

        Move move = PASS;        // the best move to play from the position
        int value = 0;           // value of the current state
        int depth = 0;           // maximum search depth reached
        long long numNodes = 0;  // number of nodes searched
        long long nps = 0;       // nodes per second
        long long duration = 0;  // duration of the search in milliseconds
        long long numMPCCuts = 0;  // number of cutoffs with mpc
        long long numETCCuts = 0;  // number of cutoffs with etc
    };

    struct SearchTask {
        SearchTask() = default;

        explicit SearchTask(SearchNode *search, bool *running = nullptr, bool *stopped = nullptr) :
                search(search),
                running(running == nullptr ? new bool(false) : running),
                completed(stopped == nullptr ? new bool(true) : stopped) {}

        explicit SearchTask(const Game &game, bool *running = nullptr, bool *stopped = nullptr) :
                search( new SearchNode(game.get_bitboard())),
                running(running == nullptr ? new bool(false) : running),
                completed(stopped == nullptr ? new bool(true) : stopped) {}

        ~SearchTask() {
            delete this->running;
            delete this->search;
            delete this->completed;
        }

        [[nodiscard]] inline SearchResult get_result() const {
            return SearchResult(this->search);
        }

        inline void stop_after(double duration) const {
            std::thread([this, duration]() {
                std::this_thread::sleep_for(std::chrono::milliseconds((long long) (duration * 1000)));
                *this->running = false;
            }).detach();
        }

        inline void stop() const {
            if (*this->running) {
                *this->running = false;
                this->search->stop();
            }
        }

        inline void complete() const {
            if (!*this->running) {
                *this->completed = true;
            }
        }

        inline void start() const {
            if (!*this->running && *this->completed) {
                *this->running = true;
                *this->completed = false;
                this->search->start();
            } else {
                std::cerr << "Search already running" << std::endl;
            }
        }

        inline void pass() const {
            this->search->pass();
        }

        inline void set_board(const Board& board) const {
            auto value = this->search->value;
            *this->search = SearchNode(board);
            this->search->value = -value;
        }

        inline void wait_for_completion(int dt = 100) const {
            while (this->search->move == MOVE_UNDEFINED) {
                std::this_thread::sleep_for(std::chrono::milliseconds(dt));
            }
        }

        inline void qt_wait_for_completion(int dt = 100) const {
            while (this->search->move == MOVE_UNDEFINED) {
                QThread::msleep(dt);
            }
        }

        SearchNode *search = nullptr;
        bool *running = nullptr;
        bool *completed = nullptr;
    };
}

#endif //OTHELLO_SEARCHSTRUCTS_H
