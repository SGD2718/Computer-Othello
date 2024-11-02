//
// Created by Benjamin Lee on 1/16/24.
//

#ifndef OTHELLO_GAME_H
#define OTHELLO_GAME_H

#include "Board.h"


// wrapper class for Board that keeps track of the move history
class Game {
public:
    Game() = default;

    explicit Game(const std::vector<Move>& moves);

    explicit Game(const std::string& moveSequence);

    explicit Game(const Board& board) : board(board), moves() {}

    Game(const Board& board, const std::vector<Move>& moves) : board(board), moves(moves) {}

    [[nodiscard]] inline bool is_white_to_move() const {
        return this->moveIndex & 1;
    }

    [[nodiscard]] inline bool is_black_to_move() const {
        return !(this->moveIndex & 1);
    }

    [[nodiscard]] inline uint64_t get_legal_moves() const {
        return this->board.get_legal_moves();
    }

    // this is expensive, don't use for a search
    [[nodiscard]] inline bool is_terminal() const {
        return this->board.is_terminal();
    }

    [[nodiscard]] inline int get_disc_count() const {
        return this->board.get_disc_count();
    }

    [[nodiscard]] inline int get_black_score() const {
        return this->moveIndex & 1 ? this->board.get_opponent_score() : this->board.get_player_score();
    }

    [[nodiscard]] inline int get_white_score() const {
        return this->moveIndex & 1 ? this->board.get_player_score() : this->board.get_opponent_score();
    }

    [[nodiscard]] inline int get_player_score() const {
        return this->board.get_player_score();
    }

    [[nodiscard]] inline int get_opponent_score() const {
        return this->board.get_opponent_score();
    }

    [[nodiscard]] inline uint64_t get_black_bitboard() const {
        auto turnMask = -(this->moveIndex & 1ULL); // 1111... if white's turn, 0000 if not
        return turnMask & this->board.O | ~turnMask & this->board.P;
    }

    [[nodiscard]] inline uint64_t get_white_bitboard() const {
        auto turnMask = -(this->moveIndex & 1ULL); // 1111... if white's turn, 0000 if not
        return turnMask & this->board.P | ~turnMask & this->board.O;
    }

    [[nodiscard]] inline Board get_bitboard() const {
        return this->board;
    }

    [[nodiscard]] inline std::vector<Move> get_move_history() const {
        return this->moves;
    }

    [[nodiscard]] inline uint64_t get_flipped(uint8_t position) const {
        return this->board.get_flipped(position);
    }

    [[nodiscard]] inline uint64_t get_flipped(uint8_t col, uint8_t row) const {
        return this->board.get_flipped(col + (row << 3));
    }

    [[nodiscard]] inline uint64_t get_flipped(const std::string& coordinates) const {
        return this->board.get_flipped(coordinates[0] + (coordinates[1] << 3));
    }

    [[nodiscard]] inline Move get_last_move() const {
        if (this->moveIndex > 0)
            return this->moves[moveIndex-1];
        return PASS;
    }

    [[nodiscard]] inline bool is_move_legal(uint8_t x) {
        uint64_t legalMask;
        return x < 64 && (legalMask = this->board.get_legal_moves()) != 0 && legalMask & (1ULL << x);
    }

    [[nodiscard]] inline bool is_move_legal(uint8_t col, uint8_t row) {
        return col < 8 && this->is_move_legal(col + (row << 3));
    }

    [[nodiscard]] inline bool is_move_legal(const Move& move) {
        auto legalMask = this->board.get_legal_moves();
        return
                move.x < 64 &&
                (legalMask != 0 &&
                 !move.is_pass() &&
                 legalMask & (1ULL << move.x) &&
                 this->board.get_flipped(move.x) == move.flip)
                || (legalMask == 0 && move.is_pass());
    }

    [[nodiscard]] inline bool is_move_legal(const std::string& coordinates) {
        auto isPass = coordinates == "PASS" || coordinates == "pass" || coordinates == "Pass" || coordinates.empty();
        uint8_t row = 0, col = 0;

        if (!isPass && (coordinates.size() != 2 ||
                (col = std::tolower(coordinates[0]) - 'a') >= 8 ||
                (row = coordinates[1] - '1') >= 8))
            return false;

        auto legalMask = this->board.get_legal_moves();
        auto x = col + (row << 3);
        return (legalMask != 0 && !isPass && legalMask & (1ULL << x)) || (legalMask == 0 && isPass);
    }

    [[nodiscard]] std::string to_string() const;

    inline void print() const {
        std::cout << this->to_string();
    }

    bool play_move(const Move &move);
    bool play_move(uint8_t position);
    bool play_move(uint_fast8_t col, uint_fast8_t row);
    bool play_move(const std::string& coordinates);

    bool play_moves(const std::vector<Move>& moveSequence);
    bool play_moves(const std::vector<uint8_t>& moveSequence);
    bool play_moves(const std::vector<std::pair<uint8_t, uint8_t>>& moveSequence);
    bool play_moves(const std::vector<std::string>& moveSequence);
    bool play_moves(std::string moveSequence);

    bool pass();

    bool undo_move(bool skipPasses = false);
    bool redo_move(bool skipPasses = false);

    [[nodiscard]] inline int get_move_index() const {
        return this->moveIndex;
    }

    inline void reset() {
        this->set_move_index(0);
    }

    void set_board(uint64_t black, uint64_t white, bool whiteToMove);
    void set_move_index(int index);

private:
    Board board;
    std::vector<Move> moves;
    int moveIndex = 0;
};

#endif //OTHELLO_GAME_H
