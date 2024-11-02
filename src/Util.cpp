//
// Created by Benjamin Lee on 1/26/24.
//

#include "Util.h"
#include "Engine/Masks.h"

#include <sstream>
#include <iomanip>
#include <vector>
#include <utility>

namespace util {

    /**
     * @brief Format time as dd::hh::mm::ss.ms_
     * @param duration: duration in milliseconds.
     * @param showMs: whether to show milliseconds.
     */
    std::string format_time(long long duration, bool showMs) {
        std::string unit;

        auto ms = duration % 1000;
        duration /= 1000;
        auto sec = duration % 60;
        duration /= 60;
        auto min = duration % 60;
        duration /= 60;
        auto hr = duration % 24;
        duration /= 24;
        auto day = duration;

        std::ostringstream oss;

        // days
        if (day != 0) {
            oss << day << ':' << std::setfill('0');
            unit = "d";
        }

        // hours
        if (!unit.empty())
            oss << std::setw(2) << hr << ':';
        else if (hr != 0) {
            oss << hr << ':' << std::setfill('0');
            unit = "h";
        }

        // minutes
        if (!unit.empty())
            oss << std::setw(2) << min << ':';
        else if (min != 0) {
            oss << min << ':' << std::setfill('0');
            unit = "min";
        }

        // seconds
        if (!unit.empty())
            oss << std::setw(2) << sec;
        else if (sec != 0) {
            oss << sec;
            unit = "s";
        }

        // milliseconds
        if (showMs)
            oss << '.' << std::setfill('0') << std::setw(3) << ms;
        else if (unit.empty()) {
            oss << ms;
            unit = "ms";
        }

        oss << ' ' << unit;

        return oss.str();
    }

    /**
     * @brief Truncate a number to either 2 or 3 sig figs and add an indicator for thousands (k), millions (M),
     * billions (B), trillions (T), quadrillions (Qa), or quintillions (Qi) to the end.
     *
     * @param n: the number to truncate
     */
    std::string truncate_number(long long n) {
        static const std::string suffixes[] = {"", "k", "M", "B", "T", "Qa", "Qi"};

        auto strN = std::to_string(std::abs(n));
        auto nLog10 = (int)strN.size() - 1;
        auto suffix = suffixes[nLog10 / 3];

        std::ostringstream oss;
        if (n < 0) oss << '-'; // add negative sign

        // if the number is less than 1000, don't truncate
        if (std::abs(n) < 1000) {
            oss << strN;
            return oss.str();
        }

        int nRounded; // for rounding

        // truncate to 2 or 3 sig figs depending on the number of digits before the decimal point
        switch (nLog10 % 3 + 1) {
            case 1:
                nRounded = (int)std::round(std::stod(strN.substr(0, 3)) / 10.0);
                oss << nRounded / 10 << '.' << nRounded % 10 << suffix;
                break;
            case 2:
                nRounded = (int)std::round(std::stod(strN.substr(0, 4)) / 10.0);
                oss << nRounded / 10 << '.' << nRounded % 10 << suffix;
                break;
            case 3:
                nRounded = (int)std::round(std::stod(strN.substr(0, 4)) / 10.0);
                oss << nRounded << suffix;
                break;
        }

        return oss.str();
    }

    /**
     * @brief Separate a number into groups of 3 digits with commas.
     *
     * @param n: the number to format
     */
    std::string format_number(long long n) {
        auto strN = std::to_string(std::abs(n));
        auto nLog10 = (int)strN.size() - 1;

        std::ostringstream oss;
        if (n < 0) oss << '-'; // add negative sign

        for (int i = 0; i <= nLog10; ++i) {
            oss << strN[i];
            if ((nLog10 - i) % 3 == 0 && i != nLog10) oss << ',';
        }

        return oss.str();
    }

    /**
     * @brief Print progress to the console in percentage.
     *
     * @param current: the current progress
     * @param outOf: the total progress
     * @param barWidth: the width of the progress bar
     */
    void verbose(int current, int outOf, int updateRate, int numBackspaces) {
        if (current < outOf) {
            if (current % updateRate == 0) {
                auto prev = std::max(0, current - updateRate);
                std::string backspaces = std::string(numBackspaces + std::to_string(100 * prev / outOf).length(),
                                                     '\b');
                std::cout << backspaces << 100 * current / outOf << "% complete";
                std::cout.flush();
            }
        } else {
            std::string backspaces = std::string(
                    numBackspaces + std::to_string(std::max(100 * (outOf - outOf % updateRate) / outOf, 0)).length(),
                    '\b');
            std::cout << backspaces << "100% complete" << std::endl;
        }
    }

    void print_bitboard(uint64_t bitboard) {
        static const std::string color[2][2] = {
                {"\033[0m\033[48;2;0;0;0m", "\033[1;48;2;235;0;0m"},
                {"\033[0m\033[48;2;20;20;20m", "\033[1;48;2;255;0;0m"}
        };
        int i = 0;
        for (uint64_t mask = 1; mask; mask <<= 1, ++i) {
            bool set = bitboard & mask;
            std::cout << color[((i >> 3) + (i & 7)) & 1][set] << "   ";
            if ((i & 7) == 7) std::cout << "\033[0m" << std::endl;
        }
        std::cout << "\033[0m" << std::endl;
    }

    void print_bitboards(std::vector<uint64_t> bitboards, int numColumns) {
        static const std::string color[2][2] = {
                {"\033[0m\033[48;2;0;0;0m", "\033[1;48;2;235;0;0m"},
                 {"\033[0m\033[48;2;20;20;20m", "\033[1;48;2;255;0;0m"}
                };
        int boardIdx = 0;
        if (numColumns == -1)
            numColumns = (int)bitboards.size();

        while (boardIdx < bitboards.size()) {
            for (int r = 0; r < 8; ++r) {
                for (int end = std::min(boardIdx + numColumns, (int) bitboards.size()), i = boardIdx; i < end; ++i) {
                    for (int c = 0; c < 8; ++c) {
                        auto mask = 1ULL << ((r * 8) + c);
                        bool set = bitboards[i] & mask;
                        std::cout << color[(r + c) & 1][set] << "   ";
                    }
                    std::cout << "\033[0m       ";
                }
                std::cout << '\n';
            }
            std::cout << "\033[0m" << std::endl;
            boardIdx += numColumns;
        }
    }

    const std::string ProgressBar::BLACK = "0m";
    const std::string ProgressBar::RED = "1m";
    const std::string ProgressBar::GREEN = "2m";
    const std::string ProgressBar::YELLOW = "3m";
    const std::string ProgressBar::BLUE = "4m";
    const std::string ProgressBar::MAGENTA = "5m";
    const std::string ProgressBar::CYAN = "6m";
    const std::string ProgressBar::WHITE = "7m";
    const std::string ProgressBar::NONE = "9m";

    const std::string ProgressBar::RESET = "\033[0m";
    const std::string ProgressBar::BG_COLOR = "\033[4";
    const std::string ProgressBar::FG_COLOR = "\033[3";
    const std::string ProgressBar::BLOCKS[] = {"", "▏", "▎", "▍", "▌", "▋", "▊", "▉", "█"};

    /**
     * @brief Construct a progress bar.
     * @param total total progress to be made
     * @param prefix message to display before the progress bar
     * @param progressMode mode of the progress bar
     * @param width width of the progress bar
     * @param stepSize step size of the progress bar
     * @param progressColor color of the completed progress
     * @param remainingColor color of the remaining progress
     */
    ProgressBar::ProgressBar(int total, std::string  prefix, ProgressMode progressMode, bool printNewlineOnFinish, int width,
                             int stepSize, std::string  progressColor, std::string  remainingColor) :
            total(total), width(width), progress(0), prevProgress(0), progressMode(progressMode), printNewlineOnFinish(printNewlineOnFinish),
            prefix(std::move(prefix)), progressColor(std::move(progressColor)), remainingColor(std::move(remainingColor)),
            start(std::chrono::steady_clock::now()), asString() {
        this->set_step_size(stepSize);
    }

    /**
     * @brief update the progress bar.
     * @param progress new progress (leave as -1 to increment by 1)
     */
    void ProgressBar::update(int progress) {
        if (progress == -1)
            this->progress++;
        else
            this->progress = std::min(this->total, progress);
        this->print();
    }

    void ProgressBar::set_print_newline_on_finish(bool printNewlineOnFinish) {
        this->printNewlineOnFinish = printNewlineOnFinish;
    }

    /**
     * @brief Set the step size of the progress bar.
     * @param stepSize new step size
     */
    void ProgressBar::set_step_size(int stepSize) {
        this->stepSize = stepSize * this->total / this->width;
    }

    /**
     * @brief Print the progress bar.
     * @param force whether to ignore the step size and print the progress bar anyway
     */
    void ProgressBar::print(bool force) {
        if (force || ((this->progress - this->prevProgress) << 3) >= this->stepSize || this->asString.empty() || progress == total)
            std::cout << std::string(this->asString.length(), '\b') << std::string(*this);
    }

    /**
     * @brief Finish the progress bar.
     */
    void ProgressBar::finish() {
        this->stop_timer();
        this->progress = this->total;
        this->print(true);
        std::cout << std::endl;
    }

    /**
     * @brief Get the time elapsed.
     */
    std::chrono::duration<long long, std::ratio<1, 1000000000>> ProgressBar::get_time_elapsed() {
        if (end < start)
            return std::chrono::steady_clock::now() - this->start;
        return this->end - this->start;
    }

    /**
     * @brief Start the timer.
     */
    void ProgressBar::start_timer() {
        this->start = std::chrono::steady_clock::now();
    }

    /**
     * @brief Stop the timer.
     */
    void ProgressBar::stop_timer() {
        this->end = std::chrono::steady_clock::now();
    }

    /**
     * @brief Configure the progress bar.
     * @param num new total
     * @param pref new prefix
     * @param mode new progress mode
     * @param wid new width
     * @param completedColor new color for completed progress
     * @param unfinishedColor new color for remaining progress
     */
    void ProgressBar::configure(int num, const std::string& pref, ProgressMode mode, int wid, int stepSize,
                                const std::string& completedColor, const std::string& unfinishedColor) {
        this->total = num;
        this->prefix = pref;
        this->width = wid;
        this->progressMode = mode;
        this->progressColor = completedColor;
        this->remainingColor = unfinishedColor;
        this->set_step_size(stepSize);

        if (!this->asString.empty())
            this->print();
    }

    /**
     * @brief Reset the progress bar.
     * @param print whether to re-print the progress bar
     */
    void ProgressBar::reset(bool print) {
        this->progress = 0;
        if (print)
            this->print();
    }

    /**
     * @brief Set the color of the progress completed.
     * @param color new color
     * @param print whether to re-print the progress bar
     */
    void ProgressBar::set_progress_color(const std::string& color, bool print) {
        this->progressColor = color;
        if (print)
            this->print();
    }

    /**
     * @brief Set the color of the remaining progress.
     * @param color new color
     * @param print whether to re-print the progress bar
     */
    void ProgressBar::set_remaining_color(const std::string& color, bool print) {
        this->remainingColor = color;
        if (print)
            this->print();
    }

    /**
     * @brief Set the width of the progress bar.
     * @param width new width
     * @param print whether to re-print the progress bar
     */
    void ProgressBar::set_width(int width, bool print) {
        this->width = width;
        if (print)
            this->print();
    }

    /**
     * @brief Set the total of the progress bar.
     * @param total new total
     * @param print whether to re-print the progress bar
     */
    void ProgressBar::set_total(int total, bool print) {
        this->total = total;
        if (total > this->progress)
            this->progress = this->total;
        if (print)
            this->print();
    }

    /**
     * @brief Set the progress mode of the progress bar.
     * @param mode new progress mode
     * @param print whether to re-print the progress bar
     */
    void ProgressBar::set_progress_mode(ProgressMode mode, bool print) {
        this->progressMode = mode;
        if (print)
            this->print();
    }

    /**
     * @brief Set the progress of the progress bar.
     * @param progress new progress
     * @param print whether to re-print the progress bar
     */
    void ProgressBar::set_progress(int progress, bool print) {
        this->progress = std::min(progress, this->total);
        if (print)
            this->print();
    }

    /**
     * @brief Set the prefix of the progress bar.
     * @param prefix new prefix
     * @param print whether to re-print the progress bar
     */
    void ProgressBar::set_prefix(const std::string& prefix, bool print) {
        this->prefix = prefix;
        if (print)
            this->print();
    }

    /**
     * @brief Clear the progress bar from the console.
     */
    void ProgressBar::clear() const {
        std::cout << std::string(this->asString.length(), '\b') << std::string(this->asString.length(), ' ')
                  << std::string(this->asString.length(), '\b');
    }

    /**
     * @brief Convert the progress bar to a string.
     */
    ProgressBar::operator std::string() {
        if (!this->asString.empty() && ((this->progress - this->prevProgress) << 3) < this->stepSize && progress != total)
            return this->asString;

        std::ostringstream oss;
        oss.str(std::string(this->asString.length(), ' '));
        oss << this->prefix << " - ";

        switch (this->progressMode) {
            case PERCENTAGE:
                oss << std::setw(3) << this->progress * 100 / this->total << "% ";
                break;
            case FRACTION:
                oss << std::setw((int)std::to_string(this->total).length()) << this->progress << '/' << this->total << " ";
                break;
            default:
                break;
        }

        oss << ProgressBar::FG_COLOR << this->progressColor << ProgressBar::BG_COLOR
            << this->remainingColor << (this->progress != 0 ? "▐" : "  ");

        int completed = (this->progress * this->width << 3) / this->total;

        int numBlocks = completed >> 3;
        int fractionalBlock = completed & 7;
        int remaining = this->width - numBlocks - (fractionalBlock != 0);

        // add progress
        for (int i = 0; i < numBlocks; ++i)
            oss << ProgressBar::BLOCKS[8];

        // add fractional block and remaining
        oss << ProgressBar::BLOCKS[fractionalBlock] << std::string(remaining, ' ') << ((numBlocks < this->width) ? " " : "▌") << ProgressBar::RESET;

        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(this->get_time_elapsed()).count();

        if (this->progress == 0) {
            oss << std::flush;
        }
        else if (this->progress < this->total) {
            auto approxTime = duration * (this->total - this->progress) / this->progress;
            oss << " - ETA: " << format_time(approxTime) << std::flush;
        }
        else {
            oss << " - Time elapsed: " << format_time(duration);
            if (this->printNewlineOnFinish)
                oss << std::endl;
            else
                oss << ' ' << std::flush;
        }

        this->prevProgress = this->progress;
        this->asString = oss.str();

        return this->asString;
    }
} // util