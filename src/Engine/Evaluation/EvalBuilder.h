//
// Created by Benjamin Lee on 10/25/23.
//

#ifndef OTHELLO_EVALBUILDER_H
#define OTHELLO_EVALBUILDER_H

#include "../../Game/Board.h"
#include "../../Util.h"
#include "TernaryIndices.h"
#include "Evaluation.h"
#include <utility>

#ifdef slots
#undef slots
#endif
#include "LinearModel.h"
#include <torch/torch.h>
#ifdef slots
#define slots Q_SLOTS
#endif

namespace engine::eval {

    struct GameFeatures {
        GameFeatures() = default;
        GameFeatures(std::vector<std::array<uint16_t, NUM_FEATURES>> indices, int8_t value) : indices(std::move(indices)), value(value) {}
        std::vector<std::array<uint16_t, NUM_FEATURES>> indices{};
        int8_t value{};
        uint8_t numPositions{0};
    };

    class EvalBuilder {
    public:
        static void init();
        static void preprocess_transcripts(int maxThreads, int numFilesPerBatch, bool hasLogbook = false);
        static void train(const std::string& datasetFilename, int numEpochs = 1000, int batchSize = 120000, bool loadWeights = false, bool verbose = true, int maxThreads = 4);
        static void test(const std::string& weightFile, const std::string& gamesFile, const std::string& windowTitle = "Test Results");

        static void load(float* weights, const std::string& filename);
        static void load_short(short* weights, const std::string& filename);
        static void save(const float* weights, const std::string& filename, bool round = true);
        static void save_from_model(const std::string& filename, const std::string& modelFilename);

        static void post_process(float* weights);
        static void generate_eval_constants();

        static void reformat_logbook();
        static void reformat_transcripts();

        static void print_feature_permutation(const Feature* feature, int index);
        [[nodiscard]] static Board feature_to_board(const Feature* feature, int index);
        [[nodiscard]] static int evaluate(Board board, const short* weights);

        [[nodiscard]] static int count_transcripts();
        static void init_batches(int maxThreads, int numBatches=-1);
        static void combine_batches(int maxThreads, int numBatchesPerBatch, int firstNewBatchIndex);

    private:
        struct GameData {
            GameData() = default;
            GameData(std::vector<Board> positions, int8_t value) : positions(std::move(positions)), value(value) {}
            std::vector<Board> positions = {};
            int8_t value{};
        };

        enum MirrorType {
            NONE = 0,       // asymmetric
            VERTICAL = 1,   // mirror vertical
            HORIZONTAL = 2, // mirror horizontal
            DIAGONAL7 = 4,  // mirror diagonal 7
            DIAGONAL9 = 8,  // mirror diagonal 9
            ALL = 16,       // mirror vertical and horizontal
        };

        [[nodiscard]] static int get_mirrored_index(int index, int patternIdx, int numPatternDiscs);
        static void compute_feature_indices(int *indices, const Board &board, int phase);

        static uint64_t feature_to_bitboard(const Feature *featureCoords);
        static MirrorType get_mirror_type(const Feature *featureCoords);

        static void mirror_feature(const Feature *featureCoords, Feature *output = nullptr);
        static void mirror_feature_d7(const Feature *featureCoords, Feature *output = nullptr);
        static void mirror_feature_d9(const Feature *featureCoords, Feature *output = nullptr);
        static void mirror_feature_h(const Feature *featureCoords, Feature *output = nullptr);
        static void mirror_feature_v(const Feature *featureCoords, Feature *output = nullptr);
        static void rotate_feature_90(const Feature *featureCoords, Feature *output = nullptr);
        static void rotate_feature_180(const Feature *featureCoords, Feature *output = nullptr);
        static void rotate_feature_270(const Feature *featureCoords, Feature *output = nullptr);

        static void generate_feature_coords(Feature *featureCoords, int *featureToPattern);
        static void generate_coord_features(Feature *featureCoords, CoordFeatures *coordFeatures, int lastFeature = NUM_PATTERN_SYMMETRIES);
        static void print_feature_coords(Feature *featureCoords, const int *featureToPattern, const std::string& name);
        static void print_coord_features(CoordFeatures *coordFeatures, const std::string& name);
        static void print_feature_to_pattern(const int *featureToPattern, const std::string& name);

        [[maybe_unused]] static void test_mirrors();
        [[maybe_unused]] static void test_features();

        static long compute_game_features(GameFeatures& gameFeatures, const GameData& gameData, long *numPhaseEntries, long *numPhaseObservations);
        static unsigned int parse_game(GameData& gameData, const std::string& transcript);
        static unsigned int parse_games(std::vector<GameData>& games, const std::string& filename, bool verbose = false);
        static void make_combined_batch(int firstBatchIndex, int numBatches, int newBatchIndex);
        static void write_batch_data(int batchIndex);

        static void add_training_observation(std::vector<int64_t> *matrixIndices, std::vector<float> *matrixValues, std::vector<float> *labels, int64_t& row, int* featureIndices, float value);
        static void train_phase(const std::string& datasetFilename, int phaseIndex, int batchSize, int numEpochs, bool loadWeights, bool verbose, float learningRate = 0.01, float lossSlopeThreshold = 0.001);
        [[nodiscard]] static bool has_plateaued(const std::vector<std::pair<int, float>>& data, int sampleSize, float threshold = 0.003f);
        [[nodiscard]] static std::vector<std::tuple<torch::Tensor, torch::Tensor>> load_phase_data(int phaseIndex, int batchSize, const std::string& filename);

        static void interpolate_weights(float* weights);
        static void mirror_weights(float* weights);

        static inline int get_phase_start_disc_count(int phase) {
            return phase * NUM_PHASE_DISCS + 5;
        }

        static inline int get_phase_end_disc_count(int phase) {
            return (phase + 1) * NUM_PHASE_DISCS + 4;
        }

        static void pt_to_weights(float* weights, const std::string& filename);

        static Feature MIRRORED_FEATURES[NUM_PATTERN_SYMMETRIES];
        static int MIRRORED_ORDER[NUM_PATTERNS][MAX_FEATURE_SIZE];
        static MirrorType MIRROR_TYPES[NUM_PATTERN_SYMMETRIES];
        static uint_fast8_t ARRAY_TO_BITBOARD_COORD[64];
        static int OFFSETS[NUM_FEATURES];

        static Board IGNORE; // used to indicate that the current player passed so that it's left out of the training/testing data
        static bool initialized;
    };
}
#endif //OTHELLO_EVALBUILDER_H
