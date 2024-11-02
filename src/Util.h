//
// Created by Benjamin Lee on 1/26/24.
//

#ifndef OTHELLO_UTIL_H
#define OTHELLO_UTIL_H

#include <string>
#include <iostream>
#include <cstdint>

namespace util {
    std::string format_time(long long duration, bool showMs = false);
    std::string truncate_number(long long n);
    std::string format_number(long long n);

    void verbose(int current, int outOf, int updateRate, int numBackspaces = 10);
    void print_bitboard(uint64_t bitboard);
    void print_bitboards(std::vector<uint64_t> bitboards, int numColumns = -1);

    enum ProgressMode: char {
        NONE = 0,
        PERCENTAGE = 1,
        FRACTION = 2
    };

    class ProgressBar {
    public:
        ProgressBar(int total = 100, std::string prefix = "", ProgressMode progressMode = PERCENTAGE, bool printNewlineOnFinish = true, int width = 50,
                    int stepSize = 1, std::string  progressColor = GREEN, std::string  remainingColor = WHITE);
        void update(int progress=-1);
        void print(bool force = false);
        void finish();
        std::chrono::duration<long long, std::ratio<1, 1000000000>> get_time_elapsed();
        void start_timer();
        void stop_timer();
        void reset(bool print = false);
        void configure(int num = 100, const std::string& pref = "", ProgressMode mode = PERCENTAGE, int wid = 50,
                       int stepSize = 1, const std::string& completedColor = GREEN, const std::string& unfinishedColor = WHITE);
        void set_step_size(int stepSize);
        void set_progress_color(const std::string& color, bool print = false);
        void set_remaining_color(const std::string& color, bool print = false);
        void set_width(int width, bool print = false);
        void set_total(int total, bool print = false);
        void set_progress_mode(ProgressMode mode, bool print = false);
        void set_progress(int progress, bool print = false);
        void set_prefix(const std::string& prefix, bool print = false);
        void set_print_newline_on_finish(bool printNewlineOnFinish);
        void clear() const;

        explicit operator std::string();

        static const std::string RED;
        static const std::string GREEN;
        static const std::string YELLOW;
        static const std::string BLUE;
        static const std::string MAGENTA;
        static const std::string CYAN;
        static const std::string WHITE;
        static const std::string BLACK;
        static const std::string NONE;

    private:
        bool printNewlineOnFinish;
        int total;
        int width;
        int progress;
        int prevProgress;
        int stepSize{};
        ProgressMode progressMode;
        std::string prefix;
        std::string progressColor;
        std::string remainingColor;
        std::chrono::time_point<std::chrono::steady_clock> start;
        std::chrono::time_point<std::chrono::steady_clock> end;
        std::string asString;

        static const std::string BLOCKS[];
        static const std::string RESET;
        static const std::string BG_COLOR;
        static const std::string FG_COLOR;

    };

    void operator << (std::ostream& os, const ProgressBar& bar);
} // util

#endif //OTHELLO_UTIL_H
