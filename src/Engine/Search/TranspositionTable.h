//
// Created by Benjamin Lee on 2/28/24.
//

#ifndef OTHELLO_TRANSPOSITIONTABLE_H
#define OTHELLO_TRANSPOSITIONTABLE_H

#include "../../Const.h"
#include "../../Game/Board.h"
#include "SearchStructs.h"
#include <iostream>
#include <cstdint>

namespace engine {
    inline uint32_t get_write_priority(uint8_t age, uint8_t depth, uint8_t selectivity) {
        return (uint32_t)age << 16 | (uint32_t)depth << 8 | (uint32_t)selectivity;
    }

    inline uint32_t get_write_priority(uint_fast8_t age, int depth, uint_fast8_t selectivity) {
        return (uint32_t)age << 16 | (uint32_t)depth << 8 | (uint32_t)selectivity;
    }

    inline uint32_t get_read_priority(int depth, uint_fast8_t selectivity) {
        return (uint32_t)depth << 8 | (uint32_t)selectivity;
    }

    inline uint32_t get_read_priority(uint_fast8_t depth, uint_fast8_t selectivity) {
        return (uint32_t)depth << 8 | (uint32_t)selectivity;
    }

    // https://github.com/Nyanyan/Egaroucid/blob/main/src/engine/spinlock.hpp
    struct Spinlock {
        std::atomic<bool> lock_ = {false};

        void lock(){
            for (;;) {
                if (!lock_.exchange(true, std::memory_order_acquire)) {
                    return;
                }
            }
        }

        bool try_lock() noexcept {
            return !lock_.load(std::memory_order_relaxed) && !lock_.exchange(true, std::memory_order_acquire);
        }

        void unlock() noexcept {
            lock_.store(false, std::memory_order_release);
        }
    };

    class HashData {
    private:
        int8_t lower = -127;
        int8_t upper = +127;
        uint8_t moves[2] = {I_PASS, I_PASS};
        uint8_t depth = 0;
        uint8_t age = 0;
        uint8_t selectivity = 0;
    public:

        inline void load_bounds(int *l, int *u) const {
            *l = this->lower;
            *u = this->upper;
        }

        inline void load_moves(uint_fast8_t *m) const {
            m[0] = this->moves[0];
            m[1] = this->moves[1];
        }

        [[nodiscard]] inline int get_first_move() const {
            return this->moves[0];
        }

        inline void reset_age() {
            this->age = 0;
        }

        inline void decrease_age(int amount = 1) {
            if (amount >= age)
                this->age = 0;
            else
                this->age -= amount;
        }

        [[nodiscard]] inline uint32_t get_write_priority() const {
            return engine::get_write_priority(this->age, this->depth, this->selectivity);
        }

        [[nodiscard]] inline uint32_t get_read_priority() const {
            return (uint32_t) this->depth << 8 | (uint32_t)this->selectivity;
        }

        /**
         * @brief Update entry with the same priority
         * @param alpha alpha value of node
         * @param beta beta value of node
         * @param value value of node
         * @param move best move at the node
         * @param running whether the search was still running
         */
        inline void
        update_same_priority(int alpha, int beta, int value, uint8_t move) {
            if (value < beta && value < this->upper) // shrink upper bound
                this->upper = (char)value;
            if (value > alpha && value > this->lower) // shrink lower bound
                this->lower = (char)value;

            // update move if it's not the same as the current best move, and it doesn't fail low
            if ((value > alpha || value == -127) && this->moves[0] != move) {
                this->moves[1] = this->moves[0];
                this->moves[0] = move;
            }
        }

        /**
         * @brief Update entry with higher priority
         *
         * @param a: current hash table age
         * @param d: the depth of the entry
         * @param alpha: the alpha value of the node
         * @param beta: the beta value of the node
         * @param value: the value of the node
         * @param move: best move at the node
         * @param running: whether the search was still running
         */
        inline void
        update_higher_priority(uint8_t a, int d, int alpha, int beta, int value, uint8_t move, uint8_t mpcLevel = 0) {
            if (value > alpha)
                lower = (char) value;
            else
                lower = (char) -127;
            if (value < beta)
                upper = (char) value;
            else
                upper = (char) 127;

            // update move if it's not the same as the current best move, and it doesn't fail low
            if ((value > alpha || value == -127) && this->moves[0] != move) {
                this->moves[1] = this->moves[0];
                this->moves[0] = move;
            }

            this->depth = (uint8_t) d;
            this->age = a;
            this->selectivity = mpcLevel;
        }

        /**
         * @brief Completely overwrite the hash entry (new board)
         * @param a: current hash table age
         * @param d: the depth of the entry
         * @param alpha: the alpha value of the node
         * @param beta: the beta value of the node
         * @param value: the value of the node
         * @param move: best move at the node
         * @param running: whether the search was still running
         */
        inline void
        overwrite(uint8_t a, int d, int alpha, int beta, int value, uint8_t move, uint8_t mpcLevel) {
            if (value > alpha)
                lower = (char) value;
            else
                lower = (char) -127;
            if (value < beta)
                upper = (char) value;
            else
                upper = (char) 127;

            //this->moves[0] = move;
            if (value > alpha || value == -127)
                this->moves[0] = move;
            else
                this->moves[0] = I_PASS;
            this->moves[1] = I_PASS;

            this->depth = (uint8_t) d;
            this->age = a;
            this->selectivity = mpcLevel;
        }
    };

    struct HashEntry {
        Board board = Board(0, 0);
        HashData data;
        Spinlock lock;

        inline void reset() {
            this->board = Board(0, 0);
            this->data = HashData();
            this->lock.unlock();
        }
    };

    class TranspositionTable {
    public:
        /*
         * @param hashSize: number of bits in the hash key
         */
        TranspositionTable();

        ~TranspositionTable() {
            delete[] this->table;
        }

        inline void clear() {
            for (int i = 0; i < (1UL << HASH_BITS); ++i)
                this->table[i].reset();
        }

        /**
         * @brief Increase the age of the transposition table
         * If the age is 255, decrease the age of all entries by 128
         *
         * (This is a great name for a function)
         *
         * @param overflowReduction: the amount to decrease the age of all entries by if the age is 255
         */
        inline void happy_birthday(uint8_t overflowReduction = 128) {
            if (this->age == 255) {
                for (int i = 0; i < (1UL << HASH_BITS); ++i) {
                    #if LOCK_TT
                        this->table[i].lock.lock();
                    #endif

                    this->table[i].data.decrease_age(overflowReduction);

                    #if LOCK_TT
                        this->table[i].lock.unlock();
                    #endif
                }
                this->age -= overflowReduction;
            }
            ++this->age;
        }

        /**
         * @brief Store an entry in the transposition table
         * @param searchNode: the search node
         * @param hash: the hash key
         * @param depth: the depth of the node
         * @param alpha: the alpha value of the node
         * @param beta: the beta value of the node
         * @param value: the value of the node
         * @param move: best move at the node
         * @param running: whether the search was still running
         */
        inline void
        store(SearchNode *searchNode, uint32_t hash, int depth, int alpha, int beta, int value, uint8_t move) {
            uint64_t index = hash & HASH_MASK;
            HashEntry *entry = &this->table[index];

            #if LOCK_TT
                entry->lock.lock();
            #endif

            auto currentPriority = entry->data.get_write_priority();
            auto newPriority = get_write_priority(this->age, depth, searchNode->selectivity);

            if (newPriority >= currentPriority) {
                if (entry->board == searchNode->board) {
                    if (newPriority > currentPriority) {
                        entry->data.update_higher_priority(this->age, depth, alpha, beta, value, move, searchNode->selectivity);
                    } else {
                        entry->data.update_same_priority(alpha, beta, value, move);
                    }
                } else {
                    entry->board = searchNode->board;
                    entry->data.overwrite(this->age, depth, alpha, beta, value, move, searchNode->selectivity);
                }
            }
            #if LOCK_TT
                entry->lock.unlock();
            #endif
        }

        /**
         * @brief Load an entry from the transposition table
         * @param searchNode: the search node
         * @param hash: the hash key
         * @param depth: the depth of the node
         * @param lower: the lower bound of the node
         * @param upper: the upper bound of the node
         * @param moves: the best moves at the node
         */
        inline void
        load(SearchNode *searchNode, uint32_t hash, int depth, int *lower, int *upper, uint_fast8_t *moves) const {
            HashEntry *entry = &this->table[hash & HASH_MASK];
            #if LOCK_TT
                entry->lock.lock();
            #endif
            if (searchNode->board.P == entry->board.P && searchNode->board.O == entry->board.O) {
                entry->data.load_moves(moves);
                if (entry->data.get_read_priority() >= get_read_priority(depth, searchNode->selectivity)) {
                    entry->data.load_bounds(lower, upper);
                }
            }
            #if LOCK_TT
                entry->lock.unlock();
            #endif
        }

        /**
         * @brief Load bounds from the transposition table
         * @param searchNode: the search node
         * @param hash: the hash key
         * @param depth: the depth of the node
         * @param lower: the lower bound of the node
         * @param upper: the upper bound of the node
         */
        inline void load_bounds(SearchNode *searchNode, uint32_t hash, int depth, int *lower, int *upper) const {
            HashEntry *entry = &this->table[hash & HASH_MASK];
            #if LOCK_TT
                entry->lock.lock();
            #endif
            if (searchNode->board.P == entry->board.P && searchNode->board.O == entry->board.O && entry->data.get_read_priority() >= get_read_priority(depth, searchNode->selectivity)) {
                entry->data.load_bounds(lower, upper);
            }
            #if LOCK_TT
                entry->lock.unlock();
            #endif
        }

        /** @brief load best moves from the transposition table
         * @param searchNode: the search node
         * @param hash: the hash key
         * @param moves: the best moves at the node
         */
        inline void load_moves(SearchNode *searchNode, uint32_t hash, uint_fast8_t *moves) const {
            HashEntry *entry = &this->table[hash & HASH_MASK];
            #if LOCK_TT
                entry->lock.lock();
            #endif
            if (searchNode->board.P == entry->board.P && searchNode->board.O == entry->board.O) {
                entry->data.load_moves(moves);
            }
            #if LOCK_TT
                entry->lock.unlock();
            #endif
        }

        /** @brief Get best move from the transposition table
         * @param board: the board pointer
         * @param hash: the hash key
         *
         * @return the best move at the node
         */
        inline int get_best_move(const Board *board, uint32_t hash) {
            HashEntry *entry = &this->table[hash & HASH_MASK];
            if (board->P == entry->board.P && board->O == entry->board.O) {
                return entry->data.get_first_move();
            }
            return I_PASS;
        }

        /** @brief Get hash code
         *
         * @param P  player bitboard
         * @param O  opponent bitboard
         * @return   hash code
         */
        [[nodiscard]] static inline uint32_t get_hash(uint64_t P, uint64_t O) {
            const uint16_t *p = (uint16_t *) &P;
            const uint16_t *o = (uint16_t *) &O;

            return
                    HASH_KEYS[0][p[0]] ^
                    HASH_KEYS[1][p[1]] ^
                    HASH_KEYS[2][p[2]] ^
                    HASH_KEYS[3][p[3]] ^
                    HASH_KEYS[4][o[0]] ^
                    HASH_KEYS[5][o[1]] ^
                    HASH_KEYS[6][o[2]] ^
                    HASH_KEYS[7][o[3]];
        }


        /** @brief Get hash code
         *
         * @param board board pointer
         * @return hash code
         */
        [[nodiscard]] static inline uint32_t get_hash(const Board *board) {
            auto b = (uint16_t *) board;
            return
                    HASH_KEYS[0][b[0]] ^
                    HASH_KEYS[1][b[1]] ^
                    HASH_KEYS[2][b[2]] ^
                    HASH_KEYS[3][b[3]] ^
                    HASH_KEYS[4][b[4]] ^
                    HASH_KEYS[5][b[5]] ^
                    HASH_KEYS[6][b[6]] ^
                    HASH_KEYS[7][b[7]];
        }

        static void init_hash_file(int numHashBits = HASH_BITS); // initialize hash keys
        static int init_hash(); // read hash keys from file
    private:
        static uint32_t HASH_KEYS[8][65536]; // random hash keys
        static const uint32_t HASH_MASK = (1UL << HASH_BITS) - 1UL;  // mask for the hash key
        HashEntry *table;                                            // pointer to the table
        uint8_t age = 0;                                             // age of the table
    };
} // engine

#endif //OTHELLO_TRANSPOSITIONTABLE_H

