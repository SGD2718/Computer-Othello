//
// Created by Benjamin Lee on 3/21/24.
//

#include "Evaluation.h"
#include "EvalBuilder.h"

namespace engine::eval {

    EvalBuilder::MirrorType EvalBuilder::get_mirror_type(const engine::eval::Feature *featureCoords) {
        auto bitboard = feature_to_bitboard(featureCoords);

        auto v = bit::mirror_vertical(bitboard) == bitboard;
        auto h = bit::mirror_horizontal(bitboard) == bitboard;
        auto d7 = bit::mirror_d7(bitboard) == bitboard;
        auto d9 = bit::mirror_d9(bitboard) == bitboard;

        if (d7 && d9)
            return ALL;
        if (h)
            return HORIZONTAL;
        if (v)
            return VERTICAL;
        if (d7)
            return DIAGONAL7;
        if (d9)
            return DIAGONAL9;
        return NONE;
    }

    uint64_t EvalBuilder::feature_to_bitboard(const Feature *featureCoords) {
        uint64_t res = 0;
        for (int i = 0; i < featureCoords->size; ++i)
            res |= 1ULL << ARRAY_TO_BITBOARD_COORD[featureCoords->cells[i]];
        return res;
    }

    void EvalBuilder::mirror_feature(const engine::eval::Feature *featureCoords, Feature *output) {
        auto mirrorType = get_mirror_type(featureCoords);
        switch (mirrorType) {
            case HORIZONTAL:
                mirror_feature_h(featureCoords, output);
                break;
            case VERTICAL:
                mirror_feature_v(featureCoords, output);
                break;
            case DIAGONAL7:
                mirror_feature_d7(featureCoords, output);
                break;
            case DIAGONAL9:
                mirror_feature_d9(featureCoords, output);
                break;
            case ALL:
                rotate_feature_180(featureCoords, output);
                break;
            default:
                output->size = featureCoords->size;
                std::copy(featureCoords->cells, featureCoords->cells + MAX_FEATURE_SIZE, output->cells);
        }
    }

    void EvalBuilder::mirror_feature_d7(const Feature *featureCoords, Feature *output) {
        output->size = featureCoords->size;
        for (int i = 0; i < featureCoords->size; ++i) {
            auto pos = featureCoords->cells[i];
            // swap row and column
            output->cells[i] = (uint_fast8_t) ((pos & 0b111) << 3) + (pos >> 3);
        }
    }
    void EvalBuilder::mirror_feature_d9(const Feature *featureCoords, Feature *output) {
        output->size = featureCoords->size;
        for (int i = 0; i < featureCoords->size; ++i) {
            auto pos = featureCoords->cells[i];
            // swap row and column and reverse the column (reversedColumn = 7 - column)
            output->cells[i] = (uint_fast8_t) ((0b111 - (pos & 0b111)) << 3) + 0b111 - (pos >> 3);
        }
    }

    void EvalBuilder::mirror_feature_h(const Feature *featureCoords, Feature *output) {
        output->size = featureCoords->size;
        for (int i = 0; i < featureCoords->size; ++i) {
            // reverse the column
            output->cells[i] = featureCoords->cells[i] ^ 56;
        }
    }

    void EvalBuilder::mirror_feature_v(const Feature *featureCoords, Feature *output) {
        output->size = featureCoords->size;
        for (int i = 0; i < featureCoords->size; ++i) {
            // reverse the row
            output->cells[i] = featureCoords->cells[i] ^ 7;
        }
    }

    void EvalBuilder::rotate_feature_180(const Feature *featureCoords, Feature *output) {
        output->size = featureCoords->size;
        for (int i = 0; i < featureCoords->size; ++i) {
            // reverse the row and column
            output->cells[i] = featureCoords->cells[i] ^ 63;
        }
    }

    void EvalBuilder::rotate_feature_90(const engine::eval::Feature *featureCoords,
                                        engine::eval::Feature *output) {
        output->size = featureCoords->size;
        for (int i = 0; i < featureCoords->size; ++i) {
            // rotate 90 degrees
            auto x = featureCoords->cells[i];
            output->cells[i] = (((x >> 3) | (x << 3)) & 63) ^ 56;
        }
    }

    void EvalBuilder::rotate_feature_270(const engine::eval::Feature *featureCoords,
                                         engine::eval::Feature *output) {
        output->size = featureCoords->size;
        for (int i = 0; i < featureCoords->size; ++i) {
            auto x = featureCoords->cells[i];
            output->cells[i] = (((x >> 3) | (x << 3)) & 63) ^ 7;
        }
    }
}