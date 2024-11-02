//
// Created by Benjamin Lee on 3/25/24.
//

#include "EvalBuilder.h"

namespace engine::eval {
    void EvalBuilder::print_feature_permutation(const Feature* feature, int index) {
        auto b = feature_to_board(feature, index);
        b.print(true, feature_to_bitboard(feature));
    }

    Board EvalBuilder::feature_to_board(const Feature* feature, int index) {
        std::vector<int> cellConfig(feature->size, 0);

        // extract ternary digits of index in forward order
        for (int i = 1; i <= feature->size; ++i) {
            cellConfig[feature->size - i] = index % 3;
            index /= 3;
        }

        Board b(0, 0);

        for (int i = 0; i < feature->size; ++i) {
            switch (cellConfig[i]) {
                case 0:
                    b.P |= 1ULL << ARRAY_TO_BITBOARD_COORD[feature->cells[i]];
                    break;
                case 1:
                    b.O |= 1ULL << ARRAY_TO_BITBOARD_COORD[feature->cells[i]];
                    break;
            }
        }
        return b;
    }

    void EvalBuilder::test_features() {
        for (auto &f : FEATURES) {
            std::vector<uint64_t> bitboards;
            bitboards.push_back(feature_to_bitboard(&f));
            for (auto c: f.cells) {
                bitboards.push_back(1ULL << ARRAY_TO_BITBOARD_COORD[c]);
            }
            util::print_bitboards(bitboards);
        }

        int featureIdx;
        std::vector<GameData> games;
        parse_games(games, "0000000.txt");

        util::ProgressBar bar(10000, "Testing features... ", util::ProgressMode::FRACTION);
        bar.print();
        uint_fast8_t boardArr[64], boardArrH[64], boardArrV[64], boardArr180[64], boardArrD7[64], boardArrD9[64];
        int x = 0;
        Board boardMirror;
        for (auto &game: games) {
            for (auto &board: game.positions) {
                if (board == IGNORE)
                    continue;

                // test features
                auto boardH = Board(bit::mirror_horizontal(board.P), bit::mirror_horizontal(board.O));
                auto boardV = Board(bit::mirror_vertical(board.P), bit::mirror_vertical(board.O));
                auto board180 = Board(bit::rotate_180(board.P), bit::rotate_180(board.O));
                auto boardD7 = Board(bit::mirror_d7(board.P), bit::mirror_d7(board.O));
                auto boardD9 = Board(bit::mirror_d9(board.P), bit::mirror_d9(board.O));

                board.to_array(boardArr);
                boardH.to_array(boardArrH);
                boardV.to_array(boardArrV);
                board180.to_array(boardArr180);
                boardD7.to_array(boardArrD7);
                boardD9.to_array(boardArrD9);

                for (featureIdx = 0; featureIdx < NUM_PATTERN_SYMMETRIES; ++featureIdx) {
                    const auto* cells = FEATURES[featureIdx].cells;
                    const auto* cellsMirrored = EvalBuilder::MIRRORED_FEATURES[featureIdx].cells;
                    auto size = (int)FEATURES[featureIdx].size;

                    uint_fast8_t* boardArrMirror;

                    switch (MIRROR_TYPES[featureIdx]) {
                        case NONE:
                            boardArrMirror = boardArr;
                            boardMirror = board;
                            break;
                        case VERTICAL:
                            boardArrMirror = boardArrV;
                            boardMirror = boardV;
                            break;
                        case HORIZONTAL:
                            boardArrMirror = boardArrH;
                            boardMirror = boardH;
                            break;
                        case DIAGONAL7:
                            boardArrMirror = boardArrD7;
                            boardMirror = boardD7;
                            break;
                        case DIAGONAL9:
                            boardArrMirror = boardArrD9;
                            boardMirror = boardD9;
                            break;
                        case ALL:
                            boardArrMirror = boardArr180;
                            boardMirror = board180;
                            break;
                    }

                    int idx = 0;
                    int mirrorIdx = 0;
                    int idxMirror = 0;
                    int mirrorIdxMirror = 0;

                    for (int i = 0; i < size; ++i) {
                        idx *= 3;                               mirrorIdx *= 3;
                        idx += boardArr[cells[i]];              mirrorIdx += boardArr[cellsMirrored[i]];

                        idxMirror *= 3;                         mirrorIdxMirror *= 3;
                        idxMirror += boardArrMirror[cells[i]];  mirrorIdxMirror += boardArrMirror[cellsMirrored[i]];
                    }

                    if (idx != mirrorIdxMirror) {
                        board.print();
                        boardMirror.print();

                        util::print_bitboard(feature_to_bitboard(&FEATURES[featureIdx]));
                        util::print_bitboard(feature_to_bitboard(&MIRRORED_FEATURES[featureIdx]));
                        assert(1==0);
                    }
                    assert(mirrorIdx == idxMirror);
                }
            }
            bar.update(++x);
        }

        std::cout << "feature tests successful" << std::endl;
    }

    [[maybe_unused]] void EvalBuilder::test_mirrors() {
        std::cout << "testing mirrors...  0% complete" << std::flush;
        int offset = 0;
        int numPatternDiscs;
        for (int i = 0; i < NUM_PATTERN_SYMMETRIES;) {
            auto p = FEATURE_TO_PATTERN[i];
            std::vector<int> mirrors = {};

            while (i < NUM_PATTERN_SYMMETRIES && FEATURE_TO_PATTERN[i] == p) {
                if (mirrors.empty()) {
                    numPatternDiscs = PATTERNS[p].size;
                    for (int f = 0; f < POW3[numPatternDiscs]; ++f) {
                        auto mf = get_mirrored_index(f, p, numPatternDiscs);
                        assert(get_mirrored_index(mf, p, numPatternDiscs) == f);
                        mirrors.push_back(mf);
                    }
                } else {
                    for (int f = 0; f < POW3[numPatternDiscs]; ++f) {
                        auto mf = get_mirrored_index(f, p, numPatternDiscs);
                        assert(get_mirrored_index(mf, p, numPatternDiscs) == f);
                        if (mf != mirrors[f]) {
                            std::cerr << "mirror error: pattern=" << (int) p << ", feature=" << i << ", f=" << f
                                      << ", mf="
                                      << mf << ", mirror=" << mirrors[f] << std::endl;
                            exit(1);
                        }
                    }
                }

                util::verbose(++i, NUM_PATTERN_SYMMETRIES, 1);
            }

            offset += POW3[PATTERNS[p].size];

            std::sort(mirrors.begin(), mirrors.end());

            for (int j = 1; j < mirrors.size(); ++j) {
                assert(mirrors[j - 1] != mirrors[j]);
            }
        }
    }
}