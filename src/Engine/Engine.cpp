//
// Created by Benjamin Lee on 10/23/23.
//

#include "Engine.h"
#include <thread>

namespace engine {
    SearchResult Engine::search(const Game &game, double maxTime, Verbose verbose) {
        // set up timing
        auto search = SearchNode(game.get_bitboard());
        bool passed = game.get_last_move().is_pass();
        constexpr auto showProgressModes = Verbose::ALL | Verbose::PROGRESS;

        // make timer thread
        bool *running = new bool(true);
        bool *completed = new bool(false);
        Engine::make_timer_thread(maxTime, running).detach();

        // obtain search results from an iterative deepening search
        search.start();
        auto result = iterative_deepening_search(&search,
                                                 MAX_DEPTH, passed,
                                                 verbose & showProgressModes,
                                                 running, completed);
        if (*running)
            *running = false;
        else {
            delete running;
        }

        delete completed;

        print_stats(result, verbose);
        return result;
    }

    SearchResult Engine::search(SearchNode* search, bool passed, double maxTime, Verbose verbose) {
        // set up timing
        constexpr auto showProgressModes = Verbose::ALL | Verbose::PROGRESS;

        // make timer thread
        bool *running = new bool(true);
        bool *completed = new bool(false);
        Engine::make_timer_thread(maxTime, running).detach();

        // obtain search results from an iterative deepening search
        search->start();
        auto result = iterative_deepening_search(search,
                                                 MAX_DEPTH, passed,
                                                 verbose & showProgressModes,
                                                 running, completed);
        if (*running)
            *running = false;
        else {
            delete running;
        }

        delete completed;

        print_stats(result, verbose);
        return result;
    }

    SearchTask* Engine::search_task(const Game &game, Verbose verbose) {
        auto task = new SearchTask(game);
        auto passed = game.get_last_move().is_pass();
        this->continue_search_task(task, passed, verbose);
        return task;
    }

    void Engine::continue_search_task(engine::SearchTask *task, bool passed, engine::Engine::Verbose verbose) {
        if (task->search->board.get_disc_count() > 40)
            task->search->isEndgame = true;

        auto thread = std::thread([task, verbose, this, passed]() {
            constexpr auto showProgressModes = Verbose::ALL | Verbose::PROGRESS;
            std::cout << "continuing search..." << std::endl;
            // wait for the previous search to finish
            while (!*task->completed) {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }

            this->update();

            // obtain search results from an iterative deepening search
            task->start();
            auto result = iterative_deepening_search(task->search,
                                                     MAX_DEPTH, passed,
                                                     verbose & showProgressModes,
                                                     task->running, task->completed);
            task->stop();
            print_stats(result, verbose);

            // enable next search
            task->complete();
            std::cout << "async search completed" << std::endl;
        });
        thread.detach();
    }

    void Engine::continue_search_task_timed(engine::SearchTask *task, bool passed, double maxTime, engine::Engine::Verbose verbose) {
        this->continue_search_task(task, passed, verbose);
        Engine::make_timer_thread(maxTime, task->running, false).detach();
    }

    SearchResult Engine::search_to_depth(const Game &game, int depth, Verbose verbose, double maxTime) {
        auto search = SearchNode(game.get_bitboard());
        auto running = new bool(true);
        auto completed = new bool(false);
        Engine::make_timer_thread(maxTime, running).detach();
        auto result = this->iterative_deepening_search(&search, depth, game.get_last_move().is_pass(), verbose,
                                                       running, completed);
        delete running;
        delete completed;
        return result;
    }

    void Engine::print_stats(SearchResult &result, Verbose verbose) {
        // verbose mode bitmasks
        constexpr auto showProgressModes = Verbose::ALL | Verbose::PROGRESS;
        constexpr auto showFinalModes = showProgressModes | Verbose::SUMMARY | Verbose::RESULTS | Verbose::BEST;
        constexpr auto showValueModes = showFinalModes | Verbose::VALUE;
        constexpr auto showMoveModes = showFinalModes | Verbose::MOVE;
        constexpr auto showStatsModes = Verbose::ALL | Verbose::RESULTS | Verbose::STATS;
        constexpr auto showDepthModes = showStatsModes | Verbose::SUMMARY;

        if (verbose != Verbose::NONE) {
            std::cout << "\033[1mSearch Results:\033[0m\n";
            if (verbose & showValueModes) {
                std::cout << "\t\033[3mValue:\t\t\t\033[0m";
                if (result.value >= WIN)
                    std::cout << "win by " << result.value - WIN << '\n';
                else if (result.value <= LOSS)
                    std::cout << "lose by " << LOSS - result.value << '\n';
                else
                    std::cout << result.value << '\n';
            }

            if (verbose & showMoveModes)
                std::cout << "\t\033[3mBest Move:\t\t\033[0m" << result.move << '\n';
            if (verbose & showDepthModes)
                std::cout << "\t\033[3mDepth:\t\t\t\033[0m" << result.depth << '\n';
            if (verbose & showStatsModes) {
                std::cout << "\033[1mStatistics:\033[0m\n";
                std::cout << "\t\033[3mNodes searched:\t\033[0m" << util::format_number(result.numNodes) << " ("
                          << util::truncate_number(result.numNodes) << ")\n";
                std::cout << "\t\033[3mMPC Cutoffs:\t\033[0m" << util::format_number(result.numMPCCuts) << " ("
                          << util::truncate_number(result.numMPCCuts) << ")\n";
                std::cout << "\t\033[3mETC Cutoffs:\t\033[0m" << util::format_number(result.numETCCuts) << " ("
                          << util::truncate_number(result.numETCCuts) << ")\n";
                std::cout << "\t\033[3mSearch Time:\t\033[0m" << util::format_time(result.duration) << '\n';
                std::cout << "\t\033[3mSearch Speed:\t\033[0m" << util::format_number(result.nps) << " nodes/s ("
                          << util::truncate_number(result.nps) << " nps)";
            }
            std::cout << std::endl;
        }
    }


    std::thread Engine::make_timer_thread(double duration, bool*& running, bool deleteRunningOnCompletion, bool waitToStart) {
        if (running == nullptr)
            running = new bool(true);
        std::thread timer([running, duration, deleteRunningOnCompletion, waitToStart] {
            // wait for the search to start
            if (waitToStart) {
                while (!*running) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                }
            }

            auto endTime = std::chrono::high_resolution_clock::now() + std::chrono::duration<double>(duration);

            while (*running && std::chrono::high_resolution_clock::now() < endTime) {
                std::this_thread::sleep_for(std::chrono::milliseconds(5));
            }

            if (*running)
                *running = false;

            else if (deleteRunningOnCompletion)
                delete running;
        });

        return timer;
    }

}