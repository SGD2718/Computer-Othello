//
// Created by Benjamin Lee on 3/6/24.
//

#include "../Engine.h"

namespace engine {

    // From http://www.amy.hi-ho.ne.jp/okuhara/edaxopt.htm
    // lookup table for sorting empty squares by quadrant
    static const unsigned char parityCases[64] = {	/* Q0-Q3: x4x3x2x1 = */
            /*0000*/  0, /*0001*/  0, /*0010*/  1, /*0011*/  9, /*0100*/  2, /*0101*/ 10, /*0110*/ 11, /*0111*/  3,
            /*0002*/  0, /*0003*/  0, /*0012*/  0, /*0013*/  0, /*0102*/  4, /*0103*/  4, /*0112*/  5, /*0113*/  5,
            /*0020*/  1, /*0021*/  0, /*0030*/  1, /*0031*/  0, /*0120*/  6, /*0121*/  7, /*0130*/  6, /*0131*/  7,
            /*0022*/  9, /*0023*/  0, /*0032*/  0, /*0033*/  9, /*0122*/  8, /*0123*/  0, /*0132*/  0, /*0133*/  8,
            /*0200*/  2, /*0201*/  4, /*0210*/  6, /*0211*/  8, /*0300*/  2, /*0301*/  4, /*0310*/  6, /*0311*/  8,
            /*0202*/ 10, /*0203*/  4, /*0212*/  7, /*0213*/  0, /*0302*/  4, /*0303*/ 10, /*0312*/  0, /*0313*/  7,
            /*0220*/ 11, /*0221*/  5, /*0230*/  6, /*0231*/  0, /*0320*/  6, /*0321*/  0, /*0330*/ 11, /*0331*/  5,
            /*0222*/  3, /*0223*/  5, /*0232*/  7, /*0233*/  8, /*0322*/  8, /*0323*/  7, /*0332*/  5, /*0333*/  3
    };

    int Engine::last1(SearchNode *node, uint_fast8_t x, uint64_t P) {
        ++node->numNodes;

        int numFlipped = Board::count_n_flipped(P, x);
        int score = 2 * (__builtin_popcountll(P) + numFlipped) - 62;

        if (numFlipped == 0) { // invalid move
            numFlipped = Board::count_n_flipped(~P, x);
            if (numFlipped) // check whether the move would be valid for the opponent
                return score - 2 - numFlipped * 2; // opponent gets the empty square and the flipped discs
            return score - 1; // no one gets the empty square
        }

        return score;
    }

    int Engine::last2(SearchNode *node, int alpha, int beta, uint_fast8_t x1, uint_fast8_t x2, Board board) {
        ++node->numNodes;
        // check if the move would be legal for current player

        int value;
        Move move;

        if (eval::SURROUND_MASKS[x1] & board.O && board.calc_move(move, x1)) {
            value = -last1(node, x2, board.O ^ move.flip);

            if (value < beta && eval::SURROUND_MASKS[x2] & board.O && board.calc_move(move, x2))
                return std::max(value, -last1(node, x1, board.O ^ move.flip));
            return value;
        } else if (eval::SURROUND_MASKS[x2] & board.O && board.calc_move(move, x2)) {
            return -last1(node, x1, board.O ^ move.flip);
        } else { // pass
            board.pass();
            if (board.calc_move(move, x1)) {
                value = last1(node, x2, board.P ^ move.flip);

                if (value < -alpha && board.calc_move(move, x2))
                    return std::min(value, last1(node, x1, board.P ^ move.flip));
                return value;
            } else if (board.calc_move(move, x2)) {
                return last1(node, x1, board.P ^ move.flip);
            }
            return -board.get_end_value(64);
        }
    }

    int Engine::last3(SearchNode* node, int alpha, int beta, uint_fast8_t x1, uint_fast8_t x2, uint_fast8_t x3, int sort3, Board board) {

    }

    /**
     * Solve endgame with 4 empties with parity-based ordering
     *
     * Need to sort:
     * This board contains only 4 empty squares, so empty squares on each part will be:
     *      4 - 0 - 0 - 0
     *      3 - 1 - 0 - 0
     *      2 - 2 - 0 - 0
     *      2 - 1 - 1 - 0 > need to sort
     *      1 - 1 - 1 - 1
     * then the parities for squares will be:
     *      0 - 0 - 0 - 0
     *      1 - 1 - 0 - 0
     *      0 - 0 - 0 - 0
     *      1 - 1 - 0 - 0 > need to sort
     *      1 - 1 - 1 - 1
     *
     * @param node
     * @param alpha
     * @param beta
     * @return
     */
    int Engine::last4(SearchNode* node, int alpha, int beta) {
        auto empty = ~(node->board.P | node->board.O);
        uint_fast8_t x1 = bit::bitboard_to_coord(empty);
        uint_fast8_t x2 = bit::next_set_bit(empty);
        uint_fast8_t x3 = bit::next_set_bit(empty);
        uint_fast8_t x4 = bit::next_set_bit(empty);

        // I'll be honest i have no idea how this works.
        // I'm just going to copy it and hope it works.
        // 0x24 = 0b00100100 (set bits in the 4 and 32 places)
        // x3 & 0b00100100 will be nonzero iff x3 == 1, 2, 3, 4
        const int paritySort = parityCases[((x3 ^ x4) & 0x24) + ((((x2 ^ x4) & 0x24) * 2 + ((x1 ^ x4) & 0x24)) >> 2)];

        switch (paritySort) {
            case 8: // 2-2-1-1
                std::swap(x1, x3);
                std::swap(x2, x4);
                break;
            case 7: //
                std::swap(x1, x4);
                break;
            case 6: // 1-2-1-2
                std::swap(x1, x3);
                break;
            case 5:
                std::swap(x2, x4);
                break;
            case 4:
                std::swap(x2, x3);
                break;
                default:
            break;
                exit(1);
        }
        return 0;
    }
}