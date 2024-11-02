//
// Created by Benjamin Lee on 10/2/23.
//

#ifndef OTHELLO_BOARD_H
#define OTHELLO_BOARD_H

#include <vector>
#include <array>
#include <cassert>
#include <string>
#include "../Const.h"
#include "../Engine/Masks.h"
#include "Move.h"
#include "../Bit.h"

struct Move;

class Board {
public:
    explicit Board(): P(0x0000000810000000), O(0x0000001008000000) {};
    explicit Board(uint64_t P, uint64_t O): P(P), O(O) {};

    explicit Board(const uint_fast8_t arr[64]): P(0), O(0) {
        for (int i = 0; i < 64; ++i) {
            if (arr[i] == PLAYER)
                this->P |= 1ULL << i;
            else if (arr[i] == OPPONENT)
                this->O |= 1ULL << i;
        }
    }

    Board(const Board& board) = default;

    [[nodiscard]] uint64_t get_legal_moves() const;

    [[nodiscard]] inline bool is_full() const {
        return (this->P | this->O) == -1ULL;
    }

    // this is expensive, don't use for a search
    [[nodiscard]] inline bool is_terminal() const {
        return this->is_full() || this->get_legal_moves() == 0 && this->pass_and_copy().get_legal_moves() == 0;
    }

    [[nodiscard]] inline int get_disc_count() const {
        return __builtin_popcountll(P | O);
    }

    [[nodiscard]] inline int get_player_score() const {
        return __builtin_popcountll(P);
    }

    [[nodiscard]] inline int get_opponent_score() const {
        return __builtin_popcountll(O);
    }

    [[nodiscard]] inline int get_disc_difference() const {
        return __builtin_popcountll(P) - __builtin_popcountll(O);
    }

    [[nodiscard]] inline int get_end_value(int discCount) const {
        // O = 64 - P - numEmpty
        return __builtin_popcountll(P) * 2 - discCount;
    }

    [[nodiscard]] inline uint64_t get_flipped(uint_fast8_t x) const {
        // get the position of the mask
        auto row = x >> 3; // x / 8
        auto col = x & 7;  // x % 8

        return
                bit::h8_to_h(FLIP[bit::h_to_h8(P, row)][bit::h_to_h8(O, row)][col], row) |
                bit::h8_to_v(FLIP[bit::v_to_h8(P, col)][bit::v_to_h8(O, col)][row], col) |
                bit::h8_to_d7(FLIP[bit::d7_to_h8(P, row + col)][bit::d7_to_h8(O, row + col)][std::min(row, 7 - col)], row + col) |
                bit::h8_to_d9(FLIP[bit::d9_to_h8(P, col + 7 - row)][bit::d9_to_h8(O, col + 7 - row)][std::min(row, col)], col + 7 - row);
    }

    [[nodiscard]] inline uint64_t get_flipped(uint_fast8_t col, uint_fast8_t row) const {
        return this->get_flipped(col + (row << 3));
    }

    [[nodiscard]] inline uint64_t get_flipped(const std::string& coordinates) const {
        return this->get_flipped(std::tolower(coordinates[0]) - 'a' + ((coordinates[1] - '1') << 3));
    }

    static inline int count_n_flipped(uint64_t P, uint_fast8_t x) {
        // get the position of the mask
        auto row = x >> 3; // x / 8
        auto col = x & 7;  // x % 8

        return
                N_FLIPPED[bit::h_to_h8(P, row)][col] +
                N_FLIPPED[bit::v_to_h8(P, col)][row] +
                N_FLIPPED[bit::d7_to_h8(P, row + col)][std::min(row, 7 - col)] +
                N_FLIPPED[bit::d9_to_h8(P, col + 7 - row)][std::min(row, col)];
    }

    [[nodiscard]] inline int count_n_flipped(uint_fast8_t x) const {
        return Board::count_n_flipped(this->P, x);
    }

    [[nodiscard]] inline int count_n_flipped(uint_fast8_t col, uint_fast8_t row) const {
        return this->count_n_flipped(col + (row << 3));
    }

    [[nodiscard]] inline int count_n_flipped(const std::string& coordinates) const {
        return this->count_n_flipped(std::tolower(coordinates[0]) - 'a' + ((coordinates[1] - '1') << 3));
    }


    [[nodiscard]] inline Move get_move(uint_fast8_t x) const {
        return {x, this->get_flipped(x)};
    }

    [[nodiscard]] inline uint64_t calc_move(Move& move, uint_fast8_t x) const {
        move.x = x;
        move.flip = this->get_flipped(x);
        return move.flip;
    }

    inline void play_move(const Move &move) {
        auto tmp = O ^ move.flip;
        O = P ^ move.flip ^ (1ULL << move.x);
        P = tmp;
    }

    inline void play_move(uint_fast8_t x) {
        auto flip = this->get_flipped(x);
        auto tmp = O ^ flip;
        O = P ^ flip ^ (1ULL << x);
        P = tmp;
    }

    inline void play_move(uint_fast8_t col, uint_fast8_t row) {
        this->play_move(col + (row << 3));
    }

    inline void play_move(const std::string& coordinates) {
        this->play_move(coordinates[0] + (coordinates[1] << 3));
    }

    [[nodiscard]] inline Board move_and_copy(const Move& move) const {
        return Board(O ^ move.flip, P ^ move.flip ^ (1ULL << move.x));
    }

    [[nodiscard]] inline Board move_and_copy(uint_fast8_t x) const {
        auto flip = get_flipped(x);
        return Board(O ^ flip, P ^ flip ^ (1ULL << x));
    }

    [[nodiscard]] inline Board move_and_copy(uint_fast8_t col, uint_fast8_t row) const {
        return move_and_copy(col + (row << 3));
    }

    [[nodiscard]] inline Board move_and_copy(const std::string& coordinates) const {
        return move_and_copy(std::tolower(coordinates[0]) - 'a' + ((coordinates[1] - '1') << 3));
    }

    inline void pass() {
        auto tmp = this->P;
        this->P = this->O;
        this->O = tmp;
    }

    [[nodiscard]] Board pass_and_copy() const {
        return Board(this->O, this->P);
    }

    inline void set(uint64_t p, uint64_t o) {
        this->P = p;
        this->O = o;
    }

    inline void undo_move(const Move& move) {
        auto tmp = P ^ move.flip;
        P = O ^ move.flip ^ (1ULL << move.x);
        O = tmp;
    }

    inline void undo_move(uint_fast8_t x, uint64_t flip) {
        auto tmp = P ^ flip;
        P = O ^ flip ^ (1ULL << x);
        O = tmp;
    }

    inline void undo_move(uint_fast8_t col, uint_fast8_t row, uint64_t flip) {
        undo_move(col + (row << 3), flip);
    }

    inline void undo_move(const std::string& coordinates, uint64_t flip) {
        undo_move(std::tolower(coordinates[0]) - 'a' + ((coordinates[1] - '1') << 3), flip);
    }

    [[nodiscard]] inline Board undo_move_and_copy(const Move& move) const {
        return Board(O ^ move.flip ^ move.x, P ^ move.flip);
    }

    [[nodiscard]] inline Board undo_move_and_copy(uint_fast8_t x, const uint64_t flip) const {
        return Board(O ^ ((1ULL << x) | flip), P ^ flip);
    }

    [[nodiscard]] inline Board undo_move_and_copy(uint_fast8_t col, uint_fast8_t row, uint64_t flip) const {
        return undo_move_and_copy(col + (row << 3), flip);
    }

    [[nodiscard]] inline Board undo_move_and_copy(const std::string& coordinates, uint64_t flip) const {
        return undo_move_and_copy(std::tolower(coordinates[0]) - 'a' + ((coordinates[1] - '1') << 3), flip);
    }

    inline void reset() {
        this->P = 0x0000000810000000;
        this->O = 0x0000001008000000;
    }

    inline Board rotate_90() const {
        return Board(bit::rotate_90(P), bit::rotate_90(O));
    }

    inline Board rotate_180() const {
        return Board(bit::rotate_180(P), bit::rotate_180(O));
    }

    inline Board rotate_270() const {
        return Board(bit::rotate_270(P), bit::rotate_270(O));
    }

    inline Board flip_horizontal() const {
        return Board(bit::mirror_horizontal(P), bit::mirror_horizontal(O));
    }

    inline Board flip_vertical() const {
        return Board(bit::mirror_vertical(P), bit::mirror_vertical(O));
    }

    inline Board flip_diagonal_7() const {
        return Board(bit::mirror_d7(P), bit::mirror_d7(O));
    }

    inline Board flip_diagonal_9() const {
        return Board(bit::mirror_d9(P), bit::mirror_d9(O));
    }

    inline void to_array(uint_fast8_t arr[64]) const {
        // compute the ternary digits of the board in 8-bit chunks (each chunk is a column)
        // 0 = player, 1 = opponent, 2 = empty
        auto E = ~(P | O);
        uint64_t pArr[] = {
                O      & 0x0101010101010101ULL | E << 1 & 0x0202020202020202ULL,
                O >> 1 & 0x0101010101010101ULL | E      & 0x0202020202020202ULL,
                O >> 2 & 0x0101010101010101ULL | E >> 1 & 0x0202020202020202ULL,
                O >> 3 & 0x0101010101010101ULL | E >> 2 & 0x0202020202020202ULL,
                O >> 4 & 0x0101010101010101ULL | E >> 3 & 0x0202020202020202ULL,
                O >> 5 & 0x0101010101010101ULL | E >> 4 & 0x0202020202020202ULL,
                O >> 6 & 0x0101010101010101ULL | E >> 5 & 0x0202020202020202ULL,
                O >> 7 & 0x0101010101010101ULL | E >> 6 & 0x0202020202020202ULL
        };

        // extract 8 bit chunks
        auto ternArr = (uint_fast8_t*)(&pArr);
        std::copy(ternArr, ternArr + 64, arr);
    }

    inline bool operator==(const Board& board) const {
        return this->P == board.P && this->O == board.O;
    }

    std::string to_string(bool isBlackPlayer=true) const;
    std::string to_string(bool isBlackPlayer, uint64_t highlightMask) const;

    void print(bool isBlackPlayer=true) const;
    void print(bool isBlackPlayer, uint64_t highlightMask) const;

    uint64_t P; // current player bitboard
    uint64_t O; // opponent bitboard

    /* initialize the outflank and flip tables
     * @return 0
     */
    [[nodiscard]] static int init_flip();
private:
    static uint_fast8_t FLIP[256][256][8];  // usage: FLIP[P][O][x] = flip mask
    static int N_FLIPPED[256][8];           // usage: N_FLIPPED[x][outflank] = number of discs flipped
};



#endif //OTHELLO_BOARD_H
