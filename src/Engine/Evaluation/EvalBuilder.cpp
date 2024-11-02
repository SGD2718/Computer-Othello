#include "EvalBuilder.h"
#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <random>
#include "qcustomplot.h"

constexpr unsigned short FEATURES_PASS = -1;

namespace engine::eval {
    Board EvalBuilder::IGNORE = Board();
    int EvalBuilder::MIRRORED_ORDER[NUM_PATTERNS][MAX_FEATURE_SIZE]{};
    Feature EvalBuilder::MIRRORED_FEATURES[NUM_PATTERN_SYMMETRIES]{};
    uint_fast8_t EvalBuilder::ARRAY_TO_BITBOARD_COORD[64]{};
    int EvalBuilder::OFFSETS[NUM_FEATURES]{};
    EvalBuilder::MirrorType EvalBuilder::MIRROR_TYPES[NUM_PATTERN_SYMMETRIES]{};
    bool EvalBuilder::initialized = false;

    void EvalBuilder::generate_coord_features(Feature *featureCoords, CoordFeatures *coordFeatures, int lastFeature) {
        for (int featureIdx = 0; featureIdx < lastFeature; ++featureIdx) {
            auto feature = &featureCoords[featureIdx];
            int pow3 = 1;

            // iterate through cells in reverse
            for (int i = (feature->size)-1; i >= 0; --i) {
                auto &cf = coordFeatures[ARRAY_TO_BITBOARD_COORD[feature->cells[i]]];
                cf.features[cf.numFeatures].feature = featureIdx;
                cf.features[cf.numFeatures++].offset = pow3;
                pow3 *= 3;
            }
        }
    }

    void EvalBuilder::print_coord_features(CoordFeatures *coordFeatures, const std::string& name) {
        std::cout << "constexpr CoordFeatures " << name << "[64] = {" << std::endl;
        for (int i = 0; i < 64; ++i) {
            std::cout << "\t{" << (uint)coordFeatures[i].numFeatures << ", {";
            for (int j = 0; j < coordFeatures[i].numFeatures; ++j) {
                std::cout << "{" << (uint)coordFeatures[i].features[j].feature << ", "
                          << (uint)coordFeatures[i].features[j].offset << "}";
                if (j < coordFeatures[i].numFeatures - 1)
                    std::cout << ", ";
            }
            if (i == 63) std::cout << "}}" << std::endl;
            else std::cout << "}}, " << std::endl;
        }
        std::cout << "};" << std::endl;
    }

    void EvalBuilder::print_feature_to_pattern(const int *featureToPattern, const std::string &name) {
        std::cout << "constexpr int " << name << "[NUM_PATTERN_SYMMETRIES] = {";
        int patternIdx = -1;
        for (int i = 0; i < NUM_PATTERN_SYMMETRIES; ++i) {
            if (patternIdx != featureToPattern[i]) {
                patternIdx = featureToPattern[i];
                std::cout << "\n\t";
            }
            std::cout << patternIdx;
            if (i + 1 < NUM_PATTERN_SYMMETRIES)
                std::cout << ", ";
        }
        std::cout << "\n};" << std::endl;
    }

    void EvalBuilder::generate_feature_coords(Feature *featureCoords, int *featureToPattern) {
        int index = 0;
        int patternIdx = 0;

        for (auto &p: PATTERNS) {
            auto mirrorType = EvalBuilder::get_mirror_type(&p);
            switch (mirrorType) {
                case NONE:
                    // 8 mirrors
                    featureToPattern[index] = patternIdx;
                    featureCoords[index++] = p;
                    featureToPattern[index] = patternIdx;
                    mirror_feature_h(&p, &featureCoords[index++]);
                    // rotate 90 degrees for the other 6 mirrors
                    for (int i = 0; i < 6; ++i) {
                        rotate_feature_90(&featureCoords[index - 2], &featureCoords[index]);
                        featureToPattern[index] = patternIdx;
                        ++index;
                    }
                    break;
                case ALL:
                    // 2 mirrors
                    featureToPattern[index] = patternIdx;
                    featureCoords[index++] = p;
                    featureToPattern[index] = patternIdx;
                    rotate_feature_90(&p, &featureCoords[index++]);
                    break;
                default:
                    // 4 mirrors
                    featureToPattern[index] = patternIdx;
                    featureCoords[index++] = p;
                    // rotate 90 degrees for the other 3 mirrors
                    for (int i = 0; i < 3; ++i) {
                        rotate_feature_90(&featureCoords[index-1], &featureCoords[index]);
                        featureToPattern[index] = patternIdx;
                        ++index;
                    }
                    break;
            }
            ++patternIdx;
        }
    }

    void EvalBuilder::print_feature_coords(Feature featureCoords[], const int *featureToPattern, const std::string& name) {
        std::cout << "constexpr Feature " << name << "[NUM_PATTERN_SYMMETRIES] = {" << std::endl;
        int patternIdx = -1;
        for (int i = 0; i < NUM_PATTERN_SYMMETRIES; ++i) {
            if (featureToPattern[i] != patternIdx) {
                patternIdx = featureToPattern[i];
                std::cout << "\t// " << patternIdx << " " << PATTERN_NAMES[patternIdx] << std::endl;
            }
            std::cout << "\t{" << (int)featureCoords[i].size << ", {";
            for (int j = 0; j < featureCoords[i].size; ++j) {
                std::cout << "T_" << (char)((featureCoords[i].cells[j] & 0b111) + 'A') << (char)((featureCoords[i].cells[j] >> 3) + '1');
                if (j < featureCoords[i].size - 1)
                    std::cout << ", ";
            }
            if (i == NUM_PATTERN_SYMMETRIES - 1) std::cout << "}}" << std::endl;
            else std::cout << "}}, " << std::endl;
        }
        std::cout << "};" << std::endl;
    }

    void EvalBuilder::generate_eval_constants() {

        if (!EvalBuilder::initialized) {
            EvalBuilder::init();
            initialized = true;
        }

        Feature featureCoords[NUM_PATTERN_SYMMETRIES] = {0};
        CoordFeatures coordFeatures[64] = {0};
        CoordFeatures coordFeaturesEnd[64] = {0};
        int featureToPattern[NUM_FEATURES] = {0};

        generate_feature_coords(featureCoords, featureToPattern);
        generate_coord_features(featureCoords, coordFeatures);
        generate_coord_features(featureCoords, coordFeaturesEnd, NUM_PATTERN_SYMMETRIES_END);

        // count the number of features
        int numPatternConfigs = 0;
        for (auto &pattern: PATTERNS)
            numPatternConfigs += POW3[pattern.size];

        // determine maximum features that a position can have
        int maxNumFeatures = 0;
        for (auto &cf: coordFeatures)
            if (cf.numFeatures > maxNumFeatures)
                maxNumFeatures = cf.numFeatures;

        // determine maximum features that a position can have
        int maxNumFeaturesEnd = 0;
        for (auto &cf: coordFeaturesEnd)
            if (cf.numFeatures > maxNumFeaturesEnd)
                maxNumFeaturesEnd = cf.numFeatures;

        std::cout << "constexpr int NUM_PATTERN_PERMUTATIONS = " << numPatternConfigs << ";\n"
                  << "constexpr int MAX_COORD_FEATURES = " << maxNumFeatures << ";\n"
                  << "constexpr int MAX_COORD_FEATURES_END = " << maxNumFeaturesEnd << ";\n"
                  << std::endl;

        print_feature_coords(featureCoords, featureToPattern, "FEATURES");
        std::cout << std::endl;
        print_coord_features(coordFeatures, "COORD_FEATURES");
        std::cout << std::endl;
        print_coord_features(coordFeaturesEnd, "COORD_FEATURES_END");
        std::cout << std::endl;
        print_feature_to_pattern(featureToPattern, "FEATURE_TO_PATTERN");

    }

    // INITIALIZATION

    void EvalBuilder::init() {
        if (initialized)
            return;
        // init array to bitboard conversion
        for (uint_fast8_t i = 0; i < 64; ++i) {
            ARRAY_TO_BITBOARD_COORD[i] = 0b111000 & (i << 3) | (i >> 3);
        }

        for (int i = 0; i < NUM_PATTERN_SYMMETRIES; ++i)
            EvalBuilder::mirror_feature(&FEATURES[i], &EvalBuilder::MIRRORED_FEATURES[i]);

        int lastPatternIdx = -1;
        for (int i = 0; i < NUM_PATTERN_SYMMETRIES; ++i) {
            if (FEATURE_TO_PATTERN[i] != lastPatternIdx) {
                lastPatternIdx = FEATURE_TO_PATTERN[i];

                for (int j = 0; j < FEATURES[i].size; ++j) {
                    auto coord = FEATURES[i].cells[j];
                    for (int mirrorIdx = 0; mirrorIdx < FEATURES[i].size; ++mirrorIdx) {
                        if (MIRRORED_FEATURES[i].cells[mirrorIdx] == coord) {
                            MIRRORED_ORDER[lastPatternIdx][j] = mirrorIdx;
                            break;
                        }
                    }
                }
            }
        }

        for (int i = 0; i < NUM_PATTERN_SYMMETRIES; ++i)
            MIRROR_TYPES[i] = get_mirror_type(&FEATURES[i]);

        int offset = 0;
        int patternIdx = 0;
        int i;
        for (i = 0; i < NUM_PATTERN_SYMMETRIES; ++i) {
            OFFSETS[i] = offset;
            if (i + 1 == NUM_PATTERN_SYMMETRIES || FEATURE_TO_PATTERN[i+1] != patternIdx) {
                ++patternIdx;
                offset += POW3[FEATURES[i].size];
            }
        }
        #if TUNE_MODE_MIDGAME
            OFFSETS[i] = offset;
            OFFSETS[++i] = offset + SCORE_RANGE * SCORE_RANGE;
        #endif
    }

    // COMPUTING FEATURES

    int EvalBuilder::get_mirrored_index(int index, int patternIdx, int numPatternDiscs) {
        std::vector<int> cellConfig(numPatternDiscs, 0);

        // extract ternary digits of index in forward order
        for (int i = 1; i <= numPatternDiscs; ++i) {
            cellConfig[numPatternDiscs - i] = index % 3;
            index /= 3;
        }

        // get mirror index
        auto mi = 0;

        for (int i = 0; i < numPatternDiscs; ++i) {
            mi *= 3;
            mi += cellConfig[MIRRORED_ORDER[patternIdx][i]];
        }

        return mi;
    }

    // ternary-encode features to use as indices of the weights
    void EvalBuilder::compute_feature_indices(int *indices, const Board& board, int phase) {
        uint_fast8_t boardArr[64];
        board.to_array(boardArr);

        const auto phaseOffset = phase * NUM_PHASE_PARAMS;
        int featureIdx;
        for (featureIdx = 0; featureIdx < NUM_PATTERN_SYMMETRIES; ++featureIdx) {
            // compute index and it's mirror
            auto *cells = FEATURES[featureIdx].cells;
            auto *cellsMirrored = MIRRORED_FEATURES[featureIdx].cells;
            auto size = (int) FEATURES[featureIdx].size;
            auto idx = 0, mirrorIdx = 0;

            for (int i = 0; i < size; ++i) {
                idx *= 3;
                mirrorIdx *= 3;
                idx += boardArr[cells[i]];
                mirrorIdx += boardArr[cellsMirrored[i]];
            }

            // choose the smaller index between idx and it's mirror
            indices[featureIdx] = std::min(idx, mirrorIdx) + OFFSETS[featureIdx] + phaseOffset;
        }
        #if TUNE_MODE_MIDGAME
            indices[NUM_FEATURES - 2] =
                    (board.get_player_score() * SCORE_RANGE + board.get_opponent_score()) + OFFSETS[NUM_FEATURES - 2] +
                    phaseOffset; // score
            indices[NUM_FEATURES - 1] = eval::get_potential_mobility(board.P, board.O) * MAX_SURROUND +
                                        eval::get_potential_mobility(board.O, board.P) + OFFSETS[NUM_FEATURES - 1] +
                                        phaseOffset;  // surround
        #endif
        std::sort(indices, indices + NUM_FEATURES);
    }

    // PARSING GAME FILES

    int EvalBuilder::count_transcripts() {
        auto dirIter = std::filesystem::directory_iterator(TRANSCRIPT_DIRECTORY);
        return (int)std::count_if(
                begin(dirIter),
                end(dirIter),
                [](auto& entry) { return entry.is_regular_file(); }
        ) - 1;
    }

    void EvalBuilder::reformat_logbook() {
        // get the number of files in the transcript directory to avoid overwriting
        auto fileNumber = EvalBuilder::count_transcripts();

        // shuffle the logbook file
        std::ifstream logbook(LOGBOOK_FILEPATH);
        std::vector<std::string> lines;
        lines.reserve(121123);
        std::string line;

        int numGames = 0;
        while (std::getline(logbook, line)) {
            if (!line.empty()) {
                lines.push_back(line);
                ++numGames;
            }
        }

        std::random_device rd;
        std::mt19937 g(rd());

        for (int i = 0; i < 5; ++i)
            std::shuffle(lines.begin(), lines.end(), g);

        logbook.close();

        // write the shuffled lines to new transcripts
        std::ostringstream transcriptFilepath("");
        transcriptFilepath << TRANSCRIPT_DIRECTORY << std::setfill('0') << std::setw(7) << fileNumber << ".txt";

        std::ofstream outfile(transcriptFilepath.str());
        std::cout << transcriptFilepath.str() << std::endl;
        int numGamesInFile = 0;

        for (auto &game: lines) {
            for (char c: game) {
                if (c == ':')
                    break;
                if (c != '+' && c != '-')
                    outfile << c;
            }
            outfile << std::endl;

            if (++numGamesInFile == 10000) {
                outfile.close();
                // generate new file name
                transcriptFilepath.str("");
                transcriptFilepath << TRANSCRIPT_DIRECTORY << std::setfill('0') << std::setw(7) << ++fileNumber << ".txt";
                // create new file
                outfile.open(transcriptFilepath.str());
                std::cout << transcriptFilepath.str() << std::endl;
                numGamesInFile = 0;
            }
        }
        if (outfile.is_open())
            outfile.close();
    }

    void EvalBuilder::reformat_transcripts() {
        auto numTranscripts = EvalBuilder::count_transcripts();
        std::string tmpFilepath = TRANSCRIPT_DIRECTORY "tmp.txt";
        std::ostringstream fileName;
        std::ifstream inFile;
        std::ofstream outFile;

        std::string line;

        for (int i = 0; i < numTranscripts; ++i) {
            fileName.str("");
            fileName << TRANSCRIPT_DIRECTORY << std::setfill('0') << std::setw(7) << i << ".txt";
            inFile.open(fileName.str());

            // count number of lines
            int lineCount = 0;
            while (std::getline(inFile, line))
                ++lineCount;

            // reset cursor in input file
            inFile.clear();
            inFile.seekg(0, std::ios::beg);

            // copy the file to a temporary file
            outFile.open(tmpFilepath);
            outFile << lineCount << std::endl;
            outFile << inFile.rdbuf();

            inFile.close();
            outFile.close();

            // replace the original file with the temporary file
            std::remove(fileName.str().c_str());
            std::rename(tmpFilepath.c_str(), fileName.str().c_str());

            std::cout << "finished " << i << " of " << numTranscripts << std::endl;
        }
    }

    /**
     * @brief parse a game from a transcript
     * @param gameData object to store game data
     * @param transcript game transcript
     * @return number of training examples this will add
     */
    unsigned int EvalBuilder::parse_game(EvalBuilder::GameData& gameData, const std::string& transcript) {
        // parse moves
        Board board;
        unsigned int numPositions = 0;
        int phase;

        for (int i = 0, numDiscs = 5; i+1 < transcript.size(); i += 2, ++numDiscs) {
            // pass if no legal moves so that we can keep track of who's turn it is based on number of turns
            if (board.get_legal_moves() == 0) {
                gameData.positions.push_back(EvalBuilder::IGNORE);
                board.pass();
            }
            auto move = (transcript[i] - 'a') + ((transcript[i + 1] - '1') << 3);

            board.play_move(static_cast<uint8_t>(move));

            gameData.positions.push_back(board);

            // count number of positions

            #if TUNE_MODE_MIDGAME
                phase = get_phase(numDiscs);
                if (phase == NUM_PHASES - 1 || phase == 0)
                    numPositions += 3;
                else if (phase == NUM_PHASES - 2 || phase == 1)
                    numPositions += 4;
                else
                    numPositions += 5;
                assert(phase < NUM_PHASES);
            #else
                ++numPositions;
            #endif
        }

        // calculate value
        if (gameData.positions.size() & 1) {
            // odd number of positions -> current player is white, negate
            gameData.value = static_cast<int8_t>( -board.get_disc_difference() );
        } else {
            // even number of positions -> current player is black, don't negate
            gameData.value = static_cast<int8_t>( +board.get_disc_difference() );
        }

        return numPositions;
    }

    /**
     * @brief parse games from a transcript file
     * @param games vector to store game data
     * @param filename name of transcript file
     * @param verbose whether to print progress
     * @return number of training examples this can make
     */
    unsigned int EvalBuilder::parse_games(std::vector<EvalBuilder::GameData>& games, const std::string& filename, bool verbose) {
        std::ifstream file(TRANSCRIPT_DIRECTORY + filename);

        std::string line, word;

        // first line has the number of games in the file
        std::getline(file, line);
        const int numGames = std::stoi(line);

        util::ProgressBar progressBar(numGames, "Parsing games from '" + filename + '\'');
        if (verbose)
            progressBar.start_timer();

        games.resize(numGames);

        unsigned int numPositions = 0;

        if (file.is_open()) {
            for (int i = 0; i < numGames; ++i) {
                if (!std::getline(file, line)) {
                    std::cerr << "Error reading game file \"" << filename << "\": expected " << numGames
                              << " games, but only found " << i << std::endl;
                    std::exit(1);
                }

                numPositions += parse_game(games[i], line);

                // verbose
                if (verbose)
                    progressBar.update(i+1);
            }
        } else {
            std::cerr << "\nError opening game file \"" << filename << '"' << std::endl;
            std::exit(1);
        }

        file.close();

        return numPositions;
    }

    /**
     * @brief compute features for a game
     * @param gameFeatures object to store game features
     * @param gameData object containing game data
     * @return number of features
     */
    long EvalBuilder::compute_game_features(GameFeatures& gameFeatures, const GameData& gameData, long *numPhaseEntries, long *numPhaseObservations) {
        gameFeatures.value = gameData.value;
        gameFeatures.indices.resize(gameData.positions.size());

        int previous;
        int phase;
        long numEntries = 0;
        int numDiscs = 4;
        assert(!gameData.positions.empty());
        for (int i = 0; i < gameData.positions.size(); ++i) {
            // handle pass moves
            if (gameData.positions[i] == IGNORE) {
                gameFeatures.indices[i][0] = -1;
                continue;
            }

            ++gameFeatures.numPositions;

            // compute the feature indices for the position.
            // this is necessary since the features must be sorted.
            int indices[NUM_FEATURES];
            compute_feature_indices(indices, gameData.positions[i], 0);

            // add the indices to the game features
            previous = -2;

            #if TUNE_MODE_MIDGAME
                phase = get_phase(++numDiscs);
                const int s1 = std::max(0, phase - 2);
                const int s5 = std::min(NUM_PHASES - 1, phase + 2);
                for (int s = s1; s <= s5; ++s)
                    ++numPhaseObservations[s];
            #endif

            for (int f = 0; f < NUM_FEATURES; ++f) {
                int index = indices[f] - OFFSETS[f];
                gameFeatures.indices[i][f] = static_cast<unsigned short>(index);
                if (f < NUM_PATTERN_SYMMETRIES)
                    assert(index < POW3[FEATURES[f].size]);

                // count the number of features
                if (indices[f] != previous) {
                    #if TUNE_MODE_MIDGAME
                        if (phase == 0 || phase == NUM_PHASES - 1)
                            numEntries += 3;
                        else if (phase == 1 || phase == NUM_PHASES - 2)
                            numEntries += 4;
                        else
                            numEntries += 5;
                        for (int s = s1; s <= s5; ++s)
                            ++numPhaseEntries[s];
                    #else
                        ++numPhaseEntries[0];
                        ++numEntries;
                    #endif
                }
                previous = indices[f];
            }
        }
        assert(gameFeatures.numPositions > 0);
        return numEntries;
    }

    void EvalBuilder::init_batches(int maxThreads, int numBatches) {
        const int numTranscripts = count_transcripts();
        if (numBatches == -1 || numBatches > numTranscripts)
            numBatches = numTranscripts;

        util::ProgressBar progressBar(numBatches, "Initializing batch data ");
        std::vector<std::thread> threads;
        std::atomic<int> batchIndex = 0;
        std::atomic<int> numComplete = 0;
        std::mutex mtx;
        std::atomic<bool> printLock = false;
        threads.reserve(maxThreads);
        progressBar.start_timer();
        for (int i = 0; i < maxThreads; ++i) {
            threads.emplace_back([&batchIndex, &numComplete, &mtx, &printLock, &progressBar, numBatches]() {
                // get the batch index and increment
                mtx.lock();
                int index = batchIndex++;
                mtx.unlock();

                while (index < numBatches) {
                    write_batch_data(index);
                    // print progress
                    ++numComplete;
                    if (!printLock) {
                        printLock = true;
                        progressBar.update(numComplete);
                        printLock = false;
                    }

                    // get the next batch index and increment
                    mtx.lock();
                    index = batchIndex++;
                    mtx.unlock();
                }
            });
        }

        // wait for threads to finish
        for (auto &thread : threads)
            thread.join();
    }

    void EvalBuilder::write_batch_data(int batchIndex) {
        std::ostringstream filename;
        filename.str("");
        filename << std::setfill('0') << std::setw(7) << batchIndex;

        std::vector<GameData> games;
        long numObservations = parse_games(games, filename.str() + ".txt", false);
        unsigned int numGames = games.size();
        long numEntries = 0;
        long numPhaseEntries[NUM_PHASES] = {0};
        long numPhaseObservations[NUM_PHASES] = {0};

        std::ofstream file(BINARY_DATASET_DIRECTORY + filename.str() + ".bin", std::ios::binary);

        // write the number of games, training examples, and entries
        file.write(reinterpret_cast<const char *>(&numGames), sizeof(unsigned int));
        file.write(reinterpret_cast<const char *>(&numObservations), sizeof(long));
        file.write(reinterpret_cast<const char *>(&numEntries), sizeof(long));
        file.write(reinterpret_cast<const char *>(numPhaseEntries), sizeof(long) * NUM_PHASES);
        file.write(reinterpret_cast<const char *>(numPhaseObservations), sizeof(long) * NUM_PHASES);

        // write the game features
        for (auto &game: games) {
            GameFeatures gameFeatures;
            auto entriesInGame = compute_game_features(gameFeatures, game, numPhaseEntries, numPhaseObservations);
            assert(numEntries + entriesInGame >= numEntries);
            numEntries += entriesInGame;

            // header: number of positions (8-bit integer), value (8-bit integer)
            file.write(reinterpret_cast<const char *>(&gameFeatures.numPositions), sizeof(uint8_t));
            file.write(reinterpret_cast<const char *>(&gameFeatures.value), sizeof(int8_t));

            // write the feature indices (each as a 16-bit integer)

            char negate = 0;
            for (auto &indices: gameFeatures.indices) {
                negate ^= '\1';
                if (indices[0] == FEATURES_PASS)
                    continue;
                file.write(reinterpret_cast<const char*>(&negate), sizeof(char));
                file.write(reinterpret_cast<const char*>(indices.data()), sizeof(short) * NUM_FEATURES);
            }
        }

        // update the number of entries
        file.seekp(sizeof(int) + sizeof(long), std::ios::beg);
        file.write(reinterpret_cast<const char *>(&numEntries), sizeof(long));
        file.write(reinterpret_cast<const char *>(numPhaseEntries), sizeof(long) * NUM_PHASES);
        file.write(reinterpret_cast<const char *>(numPhaseObservations), sizeof(long) * NUM_PHASES);

        file.close();
    }

    void EvalBuilder::combine_batches(int maxThreads, int numBatchesPerBatch, int firstNewBatchIndex) {
        const int numTranscripts = count_transcripts();
        const int numBatches = 1 + (numTranscripts - 1) / numBatchesPerBatch; // ceil(numTranscripts / numBatchesPerBatch)
        const int lastBatchIndex = firstNewBatchIndex + numBatches;
        maxThreads = std::min(maxThreads, numBatches);

        util::ProgressBar progressBar(numBatches, "Combining batches into groups of " + std::to_string(numBatchesPerBatch) + "");
        progressBar.print();
        std::vector<std::thread> threads;
        std::atomic<int> batchIndex = firstNewBatchIndex;
        std::atomic<int> numComplete = 0;
        std::mutex mtx;
        std::atomic<bool> printLock = false;
        threads.reserve(maxThreads);
        progressBar.start_timer();

        for (int i = 0; i < maxThreads; ++i) {
            threads.emplace_back(
                    [&batchIndex, &numComplete, &mtx, &printLock, &progressBar, firstNewBatchIndex, numTranscripts, lastBatchIndex, numBatchesPerBatch]() {

                        int firstIndex, newIndex;
                        // get the batch index and increment
                        mtx.lock();
                        newIndex = batchIndex++;
                        mtx.unlock();

                        while (newIndex < lastBatchIndex) {
                            firstIndex = (newIndex - firstNewBatchIndex) * numBatchesPerBatch;
                            EvalBuilder::make_combined_batch(firstIndex,
                                                             std::min(numBatchesPerBatch,
                                                                      numTranscripts - firstIndex - 1),
                                                             newIndex);
                            // print progress
                            ++numComplete;
                            if (!printLock) {
                                printLock = true;
                                progressBar.update(numComplete);
                                printLock = false;
                            }

                            // get the next batch index and increment
                            mtx.lock();
                            newIndex = batchIndex++;
                            mtx.unlock();
                        }
                    });
        }

        // wait for threads to finish
        for (auto &thread : threads)
            thread.join();
    }

    void EvalBuilder::make_combined_batch(int firstBatchIndex, int numBatches, int newBatchIndex) {
        std::ostringstream oss;
        oss << COMBINED_DATASET_DIRECTORY << std::setfill('0') << std::setw(7) << newBatchIndex << ".bin";
        std::ofstream file(oss.str(), std::ios::binary);

        unsigned int numGames = 0;
        long numObservations = 0;
        long numEntries = 0;
        long numPhaseEntries[NUM_PHASES] = {0};
        long numPhaseObservations[NUM_PHASES] = {0};

        unsigned int numGamesInBatch;
        long numObservationsInBatch, numEntriesInBatch, numPhaseEntriesInBatch[NUM_PHASES], numPhaseObservationsInBatch[NUM_PHASES];

        // write the number of games, training examples, and entries
        file.write(reinterpret_cast<const char *>(&numGames), sizeof(int));
        file.write(reinterpret_cast<const char *>(&numObservations), sizeof(long));
        file.write(reinterpret_cast<const char *>(&numEntries), sizeof(long));
        file.write(reinterpret_cast<const char *>(numPhaseEntries), sizeof(long) * NUM_PHASES);
        file.write(reinterpret_cast<const char *>(numPhaseObservations), sizeof(long) * NUM_PHASES);

        std::vector<int> batchIndices;
        for (int i = firstBatchIndex; i < firstBatchIndex + numBatches; ++i)
            batchIndices.push_back(i);

        // shuffle the batch indices
        std::random_device rd;
        std::mt19937 g(rd());
        std::shuffle(batchIndices.begin(), batchIndices.end(), g);

        // combine the batch data
        for (auto index: batchIndices) {
            std::ostringstream batchFilename;
            batchFilename << BINARY_DATASET_DIRECTORY << std::setfill('0') << std::setw(7) << index << ".bin";
            std::ifstream batchFile(batchFilename.str(), std::ios::binary);
            if (!batchFile.is_open()) {
                std::cerr << "Error opening file " << BINARY_DATASET_DIRECTORY << std::setfill('0') << std::setw(7) << index << ".bin" << std::endl;
                std::exit(1);
            }

            batchFile.read(reinterpret_cast<char *>(&numGamesInBatch), sizeof(int));
            batchFile.read(reinterpret_cast<char *>(&numObservationsInBatch), sizeof(long));
            batchFile.read(reinterpret_cast<char *>(&numEntriesInBatch), sizeof(long));
            batchFile.read(reinterpret_cast<char *>(numPhaseEntriesInBatch), sizeof(long) * NUM_PHASES);
            batchFile.read(reinterpret_cast<char *>(numPhaseObservationsInBatch), sizeof(long) * NUM_PHASES);

            numGames += numGamesInBatch;
            numObservations += numObservationsInBatch;
            numEntries += numEntriesInBatch;

            for (int i = 0; i < NUM_PHASES; ++i) {
                numPhaseEntries[i] += numPhaseEntriesInBatch[i];
                numPhaseObservations[i] += numPhaseObservationsInBatch[i];
            }

            // copy the batch data
            file << batchFile.rdbuf();
            batchFile.close();
        }

        // update the number of games, training examples, and entries
        file.seekp(0, std::ios::beg);
        file.write(reinterpret_cast<const char *>(&numGames), sizeof(int));
        file.write(reinterpret_cast<const char *>(&numObservations), sizeof(long));
        file.write(reinterpret_cast<const char *>(&numEntries), sizeof(long));
        file.write(reinterpret_cast<const char *>(numPhaseEntries), sizeof(long) * NUM_PHASES);
        file.write(reinterpret_cast<const char *>(numPhaseObservations), sizeof(long) * NUM_PHASES);

        file.close();
    }

    void EvalBuilder::preprocess_transcripts(int maxThreads, int numFilesPerBatch, bool hasLogbook) {
        if (hasLogbook)
            reformat_logbook();
        reformat_transcripts();
        init_batches(maxThreads);
        combine_batches(maxThreads, numFilesPerBatch, 0);
    }

    void EvalBuilder::add_training_observation(std::vector<int64_t> *matrixIndices, std::vector<float> *matrixValues, std::vector<float> *labels, int64_t &row, int* featureIndices, float value) {
        // set the label
        labels->push_back(value);

        int occurences = 1;
        for (int i = 0; i < NUM_FEATURES; ++i) {
            // check if the next feature is the same
            if (i+1 < NUM_FEATURES && featureIndices[i] == featureIndices[i + 1]) {
                ++occurences;
            } else {
                matrixIndices->emplace_back(row);
                matrixIndices->emplace_back((long)featureIndices[i]);
                matrixValues->emplace_back((float)occurences);

                if (featureIndices[i] >= NUM_PHASE_PARAMS)
                    std::cerr << "col = " << featureIndices[i] << " >= " << NUM_PHASE_PARAMS
                              << ", pattern = " << PATTERN_NAMES[FEATURE_TO_PATTERN[i]]
                              << std::endl;
                if (row >= labels->size())
                    std::cerr << "row = " << row << " >= " << labels->size() << std::endl;
                occurences = 1;
            }
        }
        ++row;
    }

    std::vector<std::tuple<torch::Tensor, torch::Tensor>> EvalBuilder::load_phase_data(int phaseIndex, int batchSize, const std::string& filename) {
        unsigned int numGames;
        long numObservations;
        long numEntries;
        long numPhaseEntries[NUM_PHASES];
        long numPhaseObservations[NUM_PHASES];

        std::ifstream file(COMBINED_DATASET_DIRECTORY + filename, std::ios::binary);

        if (!file.is_open()) {
            std::cerr << "Error opening file " << filename << std::endl;
            std::exit(1);
        }

        // Read header to get the number of observations and entries
        file.read(reinterpret_cast<char *>(&numGames), sizeof(int));
        file.read(reinterpret_cast<char *>(&numObservations), sizeof(long));
        file.read(reinterpret_cast<char *>(&numEntries), sizeof(long));
        file.read(reinterpret_cast<char *>(numPhaseEntries), sizeof(long) * NUM_PHASES);
        file.read(reinterpret_cast<char *>(numPhaseObservations), sizeof(long) * NUM_PHASES);
        #if (TUNE_MODE_MIDGAME)
            numEntries = numPhaseEntries[phaseIndex];
            numObservations = numPhaseObservations[phaseIndex];
        #endif

        // initialize progress bar
        util::ProgressBar progressBar((int) numGames,
                                      "Loading phase " + std::to_string(phaseIndex) + '/' +
                                      std::to_string(NUM_PHASES - 1),
                                      util::FRACTION);
        progressBar.print();

        // initialize the linear regression system
        auto *matrixIndices = new std::vector<int64_t>;
        auto *matrixValues = new std::vector<float>;
        auto *labels = new std::vector<float>;

        matrixIndices->reserve(batchSize * NUM_FEATURES * 2);
        matrixValues->reserve(batchSize * NUM_FEATURES);
        labels->reserve(batchSize);

        std::vector<std::tuple<torch::Tensor, torch::Tensor>> dataset;
        dataset.reserve(1 + (numObservations - 1) / batchSize);

        // Read the game features
        int g, f, i;
        int64_t row = 0;
        float value;
        uint8_t numPositions;
        int8_t v;

        // get the depth when the phase data starts and ends
        #if TUNE_MODE_MIDGAME
            int startDepth = get_phase_start_disc_count(std::max(phaseIndex - 2, 0)) - 5;
            int endDepth = get_phase_end_disc_count(std::min(phaseIndex + 2, NUM_PHASES - 1)) - 4;
        #else
            int startDepth = 59 - END_SEARCH_DEPTH;
            int endDepth = 60;
        #endif
        const unsigned int stepSize = sizeof(short) * NUM_FEATURES + sizeof(char);
        const unsigned int startOffset = stepSize * startDepth;
        int actualEndDepth;
        unsigned short idx;
        char negate;

        int indices[NUM_FEATURES];

        for (g = 0; g < numGames; ++g) {
            // read the number of positions and the value
            file.read(reinterpret_cast<char *>(&numPositions), sizeof(uint8_t));
            file.read(reinterpret_cast<char *>(&v), sizeof(int8_t));

            if (numPositions < startDepth) {
                file.seekg(numPositions * stepSize, std::ios::cur);
                continue;
            }

            // skip to the start offset
            file.seekg(startOffset, std::ios::cur);

            value = static_cast<float>(v);

            actualEndDepth = std::min((int)numPositions, endDepth);

            // read the feature indices
            for (i = startDepth; i < actualEndDepth; ++i) {
                // determine if we negate the value for this position
                file.read(reinterpret_cast<char *>(&negate), sizeof(char));

                // read the rest of the indices
                for (f = 0; f < NUM_FEATURES; ++f) {
                    file.read(reinterpret_cast<char *>(&idx), sizeof(short));
                    indices[f] = (int)(idx) + OFFSETS[f];
                }

                add_training_observation(matrixIndices, matrixValues, labels, row, indices, negate ? -value : value);

                // add batch to the dataset
                if (row >= batchSize) {
                    matrixIndices->shrink_to_fit();
                    matrixValues->shrink_to_fit();
                    labels->shrink_to_fit();
                    auto nnz = static_cast<int64_t>(matrixValues->size());
                    auto torchIndices = torch::from_blob(
                            matrixIndices->data(),
                            {2, nnz},
                            {1, 2},
                            [matrixIndices](void*) { delete matrixIndices; },
                            torch::kInt64
                    );
                    auto torchValues = torch::from_blob(
                            matrixValues->data(),
                            {nnz},
                            [matrixValues](void*) { delete matrixValues; },
                            torch::kFloat32
                    );
                    auto torchLabels = torch::from_blob(
                            labels->data(),
                            {static_cast<int64_t>(labels->size())},
                            [labels](void*) { delete labels; },
                            torch::kFloat32
                    );
                    auto torchData = torch::_sparse_coo_tensor_unsafe(
                            torchIndices,
                            torchValues,
                            {batchSize, NUM_PHASE_PARAMS},
                            torch::kFloat32
                    );
                    dataset.emplace_back(torchData, torchLabels);

                    // reset the batch
                    matrixIndices = new std::vector<int64_t>;
                    matrixValues = new std::vector<float>;
                    labels = new std::vector<float>;
                    matrixIndices->reserve(batchSize * NUM_FEATURES * 2);
                    matrixValues->reserve(batchSize * NUM_FEATURES);
                    labels->reserve(batchSize);
                    row = 0;
                }
            }
            if (i != numPositions) {
                file.seekg((numPositions - i) * stepSize, std::ios::cur);
            }
            progressBar.update(g + 1);
        }
        file.close();

        // add the last batch
        if (row > 0) {
            matrixIndices->shrink_to_fit();
            matrixValues->shrink_to_fit();
            labels->shrink_to_fit();
            auto nnz = static_cast<int64_t>(matrixValues->size());
            auto torchIndices = torch::from_blob(
                    matrixIndices->data(),
                    {2, nnz},
                    {1, 2},
                    [matrixIndices](void*) { delete matrixIndices; },
                    torch::kInt64
            );
            auto torchValues = torch::from_blob(
                    matrixValues->data(),
                    {nnz},
                    [matrixValues](void*) { delete matrixValues; },
                    torch::kFloat32
            );
            auto torchLabels = torch::from_blob(
                    labels->data(),
                    {row},
                    [labels](void*) { delete labels; },
                    torch::kFloat32
            );
            auto torchData = torch::_sparse_coo_tensor_unsafe(
                    torchIndices,
                    torchValues,
                    {row, NUM_PHASE_PARAMS},
                    torch::kFloat32
            );
            dataset.emplace_back(torchData, torchLabels);
        }

        return std::move(dataset);
    }

    bool EvalBuilder::has_plateaued(const std::vector<std::pair<int, float>>& data, int sampleSize, float threshold) {
        auto end = (int)data.size() - 1;

        if (end + 1 < sampleSize)
            return false;

        float meanLoss = 0;
        float meanEpoch = 0;
        for (int i = 0; i < sampleSize; ++i) {
            meanEpoch += (float)data[end - i].first;
            meanLoss += data[end - i].second;
        }
        meanLoss /= (float)sampleSize;
        meanEpoch /= (float)sampleSize;

        float rise = 0;
        float run = 0;

        for (int i = 0; i < sampleSize; ++i) {
            rise += ((float)data[end - i].first - meanEpoch) * (data[end - i].second - meanLoss);
            run += ((float)data[end - i].first - meanEpoch) * ((float)data[end - i].first - meanEpoch);
        }
        float slope = rise / run;

        return slope > -threshold;
    }

    void EvalBuilder::train_phase(const std::string &datasetFilename, int phaseIndex, int batchSize, int numEpochs,
                                  bool loadWeights, bool verbose, float learningRate, float lossSlopeThreshold) {
        std::random_device rd;
        std::mt19937 g(rd());

        auto modelFilepath = TORCH_MODEL_DIRECTORY MODEL_NAME " model" + std::to_string(phaseIndex) + ".pt";
        auto optimFilepath = TORCH_MODEL_DIRECTORY MODEL_NAME " optim" + std::to_string(phaseIndex) + ".pt";

        // initialize the model and optimizer
        auto model = LinearModel(NUM_PHASE_PARAMS, 1);
        auto optimizer = torch::optim::Adam(model.parameters(), torch::optim::AdamOptions(learningRate));
        auto criterion = torch::nn::MSELoss(torch::nn::MSELossOptions().reduction(torch::kSum));
        auto deviceType = torch::cuda::is_available() ? torch::kCUDA : torch::kCPU;

        if (loadWeights) {
            torch::serialize::InputArchive inputArchive;
            inputArchive.load_from(modelFilepath);
            model.load(inputArchive);
            inputArchive.load_from(optimFilepath);
            optimizer.load(inputArchive);
            for (auto &group: optimizer.param_groups())
                group.options().set_lr(learningRate);
        } else {
            // initialize the weights as zeros
            torch::NoGradGuard noGrad;
            for (auto &p: model.parameters())
                p.data().zero_();
        }
        model.to(deviceType);
        model.train();

        {
            auto data = load_phase_data(phaseIndex, batchSize, datasetFilename);
            std::vector<std::pair<int, float>> losses;
            losses.reserve(numEpochs);

            // get number of observations'
            unsigned int numObservations = 0;
            for (auto &[X, y]: data) {
                numObservations += y.size(0);
                X.to(deviceType);
                y.to(deviceType);
            }
            auto numObservationsFloat = (float) numObservations;

            std::cout << "\033[33;1mPHASE " << std::to_string(phaseIndex) << "/" + std::to_string(NUM_PHASES - 1)
                      << "\033[0m" << std::endl;
            int epoch;
            for (epoch = 1; epoch <= numEpochs; ++epoch) {
                std::shuffle(data.begin(), data.end(), g);
                util::ProgressBar progressBar(
                        (int) data.size(),
                        "\033[32;1mPhase " + std::to_string(phaseIndex) + " Epoch " + std::to_string(epoch) + "/" +
                        std::to_string(numEpochs) + "\033[0m",
                        util::FRACTION,
                        false);
                if (verbose)
                    progressBar.print();
                float meanLoss = 0;
                for (auto &[X, y]: data) {
                    optimizer.zero_grad();
                    auto output = model.forward(X).squeeze(1);
                    auto loss = criterion(output, y);
                    loss.backward();
                    optimizer.step();
                    meanLoss += loss.item().toFloat();
                    if (verbose)
                        progressBar.update();
                }

                // print loss
                meanLoss /= numObservationsFloat;
                losses.emplace_back(epoch, meanLoss);

                if (verbose)
                    std::cout << "  \033[1;31mLoss = " << meanLoss << "\033[0m" << std::endl;
                else {
                    progressBar.finish();
                    std::cout << "  \033[1;31mLoss = " << meanLoss << "\033[0m" << std::endl;
                }

                // check for plateau in last 10 losses using regression analysis
                if (has_plateaued(losses, 10, lossSlopeThreshold))
                    break;

                // save checkpoint every 10 epochs
                if (epoch != numEpochs && epoch % 10 == 0) {
                    std::cout << "\033[1;34mSaving checkpoint at " << epoch << " epochs...\033[0m" << std::endl;
                    torch::serialize::OutputArchive outputArchive;
                    model.save(outputArchive);
                    outputArchive.save_to(modelFilepath);
                    optimizer.save(outputArchive);
                    outputArchive.save_to(optimFilepath);
                }
            }

            // save model
            std::cout << "\033[1;36mPhase " << phaseIndex << " complete after " << epoch << " epochs.\033[0m"
                      << std::endl;
            torch::serialize::OutputArchive outputArchive;
            model.save(outputArchive);
            outputArchive.save_to(modelFilepath);
            optimizer.save(outputArchive);
            outputArchive.save_to(optimFilepath);

            // save loss history
            std::ofstream lossFile(LOSS_DIRECTORY "loss" + std::to_string(phaseIndex) + ".txt");
            for (auto [ep, loss]: losses)
                lossFile << ep << ':' << loss << std::endl;
            lossFile.close();
        }
    }

    /**
     * @brief train the weights using the given number of epochs and games
     * @param datasetFilename the name of the file with the dataset
     * @param numEpochs the number of epochs to train for
     * @param batchSize the batch size
     * @param loadWeights whether to load the weights from the file
     * @param verbose whether to print verbose output
     * @param maxThreads the maximum number of threads to use
     */
    void EvalBuilder::train(const std::string& datasetFilename, int numEpochs, int batchSize, bool loadWeights, bool verbose, int maxThreads) {
        if (!initialized)
            init();
        // train the weights for each phase
        maxThreads = std::min(maxThreads, NUM_PHASES);

        if (maxThreads > 1) {
            std::atomic<int> phaseIndex = 0;
            std::mutex mtx;
            std::vector<std::thread> threads;
            threads.reserve(maxThreads);

            for (int i = 0; i < maxThreads; ++i) {
                threads.emplace_back(
                        [&phaseIndex, &mtx, maxThreads, datasetFilename, batchSize, numEpochs, loadWeights, verbose]() {
                            int phase;
                            while (phaseIndex < NUM_PHASES) {
                                mtx.lock();
                                phase = phaseIndex++;
                                mtx.unlock();
                                if (phase < NUM_PHASES)
                                    train_phase(datasetFilename, phase, batchSize, numEpochs, loadWeights,
                                                verbose && maxThreads == 1, 0.01, 0.001);
                            }
                        });
            }
            for (auto &thread : threads)
                thread.join();
        } else if (maxThreads == 1) {
            for (int i = 0; i < NUM_PHASES; ++i) {
                train_phase(datasetFilename, i, batchSize, numEpochs, loadWeights, verbose, 0.01, 0.0001);
            }
        } else {
            std::cerr << "Invalid number of threads: " << maxThreads << std::endl;
            std::exit(1);
        }

        std::cout << "\033[1;32mTraining complete.\033[0m" << std::endl;
        std::cout << "\033[1;33mPost Processing...\033[0m" << std::endl;

        // load the weights from the saved tensorflow models and save them
        save_from_model(MODEL_NAME, MODEL_NAME " model");
    }

    // POST PROCESSING

    void EvalBuilder::interpolate_weights(float* weights) {
        const auto oldWeights = weights;

        // extrapolate weights for each stage
        for (auto i = 0; i < NUM_EVAL_PARAMS; ++i) {
            if (weights[i] == 0) {
                // find the nearest non-zero weights
                auto lowerIndex = i;
                auto upperIndex = i;

                // find the nearest non-zero weights in an earlier and later phase
                while (lowerIndex > 0 && oldWeights[lowerIndex] == 0)
                    lowerIndex -= NUM_PHASE_PARAMS;

                while (upperIndex < NUM_EVAL_PARAMS && oldWeights[upperIndex] == 0)
                    upperIndex += NUM_PHASE_PARAMS;

                // if no weight for a given bound is found, set to maximum phase
                if (lowerIndex < 0)
                    lowerIndex = i % NUM_PHASE_PARAMS;
                if (upperIndex >= NUM_EVAL_PARAMS)
                    upperIndex = i % NUM_PHASE_PARAMS + (NUM_PHASES - 1) * NUM_PHASE_PARAMS;

                auto lowerWeight = oldWeights[lowerIndex];
                auto upperWeight = oldWeights[upperIndex];

                // interpolate: c = a + (b - a)t -> t = (c - a) / (b - a)
                auto t = (float)(i - lowerIndex) / (float) (upperIndex - lowerIndex);
                weights[i] = lowerWeight + (upperWeight - lowerWeight) * t;
            }
        }
    }

    void EvalBuilder::mirror_weights(float* weights) {
        const auto oldWeights = weights;

        int offset = 0;
        int patternIdx = 0;
        int numPatternDiscs = PATTERNS[0].size;
        int numPatternPermutations = POW3[numPatternDiscs];

        // mirror weights
        for (int i = 0; i < NUM_PATTERN_PERMUTATIONS; ++i) {
            // move to next pattern if necessary
            if (i >= offset + numPatternPermutations) {
                offset += numPatternPermutations;
                numPatternDiscs = PATTERNS[++patternIdx].size;
                numPatternPermutations = POW3[numPatternDiscs];
            }

            auto idx = i - offset;
            auto mIdx = get_mirrored_index(idx, patternIdx, numPatternDiscs);
            auto mi = mIdx + offset;

            assert(mi >= offset);
            assert(patternIdx + 1 == NUM_PATTERNS || mi < offset + numPatternPermutations);

            for (int phase = 0; phase < NUM_PHASES; ++phase) {
                auto phaseOffset = phase * NUM_PHASE_PARAMS;
                auto weight = oldWeights[i + phaseOffset];
                auto mirrorWeight = oldWeights[mi + phaseOffset];
                if (weight != mirrorWeight && mirrorWeight != 0 && weight != 0) {
                    if (i < mi)
                        std::cerr << "Mirrored weight is non-zero (Mirror index is larger). " << std::endl;
                    if (i > mi)
                        std::cerr << "Mirrored weight is non-zero (Regular index is larger). " << std::endl;
                    std::cerr << "index = " << i << ", mirror = " << mi << ", weights[index] = " << weight
                              << ", weights[mirror] = " << mirrorWeight
                              << std::endl;
                    print_feature_permutation(&PATTERNS[patternIdx], idx);
                    print_feature_permutation(&PATTERNS[patternIdx], mIdx);
                    std::exit(1);
                }
                else if (i < mi) {
                    weights[mi + phaseOffset] = weight;
                    assert(mirrorWeight == 0);
                }
            }
        }
    }

    /**
     * @brief Mirror weights and interpolate them between phases.
     * @param weights the weights to post-process
     */
    void EvalBuilder::post_process(float* weights) {
        if (!initialized)
            init();

        //remove_weights_that_i_forgot_to_zero(weights);
        mirror_weights(weights);
        #if TUNE_MODE_MIDGAME
            interpolate_weights(weights);
        #endif
        std::cout << "\033[35;1mPost-processing complete.\033[0m" << std::endl;
    }

    // TESTING

    /**
     * @brief evaluate a board position
     * @param board the board to evaluate
     * @param weights the weights to use
     * @return the evaluation of the board
     */
    int EvalBuilder::evaluate(Board board, const short* weights) {
        if (!initialized)
            init();
        int phase = get_phase(board.get_disc_count());
        int indices[NUM_FEATURES];
        compute_feature_indices(indices, board, phase);
        int value = 0;

        for (int index : indices) {
            value += (int)weights[index];
        }

        return value;
    }

    /**
     * @brief test the evaluation function for accuracy
     * @param weightFile the name of the file containing the weights
     * @param gamesFile the name of the file containing the games
     * @param windowTitle
     */
    void EvalBuilder::test(const std::string& weightFile, const std::string& gamesFile, const std::string& windowTitle) {
        if (!initialized)
            init();
        std::vector<GameData> games;
        auto* weights = new short[NUM_EVAL_PARAMS];
        load_short(weights, weightFile);
        parse_games(games, gamesFile);
        auto numGames = games.size();

        std::array<QVector<double>, NUM_PHASES> pred{};
        std::array<QVector<double>, NUM_PHASES> actual{};

        util::ProgressBar progressBar((int)numGames, "Testing evaluation function ");
        progressBar.start_timer();

        QVector<double> rmse(NUM_PHASES, 0);
        QVector<double> absError(NUM_PHASES, 0);
        QVector<int> counts(NUM_PHASES, 0);

        // compute predictions and actual values
        for (auto i = 0; i < numGames; ++i) {
            auto &[positions, v] = games[i];
            auto value = static_cast<double>(v);

            int phase, numDiscs = 4;
            for (auto &board : positions) {
                value = -value;

                if (board == IGNORE)
                    continue; // skip pass moves

                #if !TUNE_MODE_MIDGAME
                    if (numDiscs++ < 64 - NUM_PHASE_DISCS)
                        continue;
                #endif

                // update phase and numDiscs
                phase = get_phase(++numDiscs);

                // add data point
                auto x = value;
                auto y = evaluate(board, weights) * EVAL_TO_DOUBLE;
                pred[phase].emplace_back(y);
                actual[phase].emplace_back(x);

                // update squared error calculations
                rmse[phase] += (y - x) * (y - x);
                absError[phase] += std::fabs(y - x);
                ++counts[phase];
            }
            progressBar.update(i+1);
        }

        // compute root mean squared error
        for (auto i = 0; i < NUM_PHASES; ++i) {
            rmse[i] = std::sqrt(rmse[i] / counts[i]);
            absError[i] /= counts[i];
        }

        // print results
        std::cout << "Phase\tRMSE\t\tAbsolute Error" << std::endl;
        for (auto i = 0; i < NUM_PHASES; ++i) {
            std::cout << i << "\t\t" << rmse[i] << "\t\t" << absError[i] << std::endl;
        }

        // plot results
        auto* window = new QMainWindow();
        window->setWindowTitle(windowTitle.c_str());
        window->setMinimumSize(640, 480);

        auto *centralWidget = new QWidget(window);
        auto *mainLayout = new QHBoxLayout(centralWidget);
        auto *plot = new QCustomPlot(centralWidget);
        auto *checkBoxLayout = new QVBoxLayout;

        checkBoxLayout->setSizeConstraint(QLayout::SetMinimumSize);

        // configure plot
        int alpha = 50;
        const QColor colors[] = {QColor(255, 0, 0, alpha),
                                 QColor(255, 128, 0, alpha),
                                 QColor(128, 128, 0, alpha),
                                 QColor(128, 255, 0, alpha),
                                 QColor(0, 255, 0, alpha),
                                 QColor(0, 255, 128, alpha),
                                 QColor(0, 255, 255, alpha),
                                 QColor(0, 128, 255, alpha),
                                 QColor(0, 0, 255, alpha),
                                 QColor(128, 0, 255, alpha),
                                 QColor(255, 0, 255, alpha),
                                 QColor(255, 0, 128, alpha),
                                 QColor(128, 128, 128, alpha),
                                 QColor(0, 0, 0, alpha),
                                 QColor(255, 128, 128, alpha),
                                 QColor(128, 255, 128, alpha)
        };

        for (auto i = 0; i < NUM_PHASES; ++i) {
            plot->addGraph();
            plot->graph(i)->setData(actual[i], pred[i]);
            plot->graph(i)->setName(QString("Phase " + QString::number(i)));
            plot->graph(i)->setLineStyle(QCPGraph::lsNone);
            plot->graph(i)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, colors[i], 5));
            plot->graph(i)->setVisible(false);
        }

        plot->xAxis->setLabel("Actual Value");
        plot->xAxis->setRange(-64, 64);
        plot->yAxis->setLabel("Evaluation");
        plot->yAxis->setRange(-64, 64);
        plot->legend->setVisible(true);
        plot->legend->setBrush(QBrush(QColor(255, 255, 255, 150)));
        plot->legend->setBorderPen(Qt::NoPen);
        plot->legend->setWrap(3);
        plot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables | QCP::iSelectLegend | QCP::iSelectAxes | QCP::iMultiSelect | QCP::iSelectItems | QCP::iSelectOther);
        plot->setWindowTitle("Evaluation vs. Actual Value");
        plot->replot();

        // Create checkboxes for each graph
        for (int i = 0; i < NUM_PHASES; ++i) {
            auto *checkBox = new QCheckBox("Phase " + QString::number(i), centralWidget);
            checkBox->setFixedWidth(100);
            checkBoxLayout->addWidget(checkBox);
            QMainWindow::connect(checkBox, &QCheckBox::stateChanged, [plot, i](int state) {
                plot->graph(i)->setVisible(state == Qt::Checked);
                plot->replot();
            });
        }

        mainLayout->addWidget(plot);
        mainLayout->addLayout(checkBoxLayout);
        centralWidget->setLayout(mainLayout);
        window->setCentralWidget(centralWidget);
        window->show();
    }

    // SAVING AND LOADING

    void EvalBuilder::save_from_model(const std::string& filename, const std::string& modelFilename) {
        auto* weights = new float[NUM_EVAL_PARAMS];
        for (int phase = 0; phase < NUM_PHASES; ++phase) {
            auto filepath = modelFilename + std::to_string(phase) + ".pt";
            pt_to_weights(weights + phase * NUM_PHASE_PARAMS, filepath);
        }

        save(weights, filename + " raw.bin", false);

        // post processing
        post_process(weights);
        save(weights, filename + ".bin", true);
        delete[] weights;
        std::cout << "Save complete." << std::endl;
    }

    void EvalBuilder::pt_to_weights(float* weights, const std::string& filename) {
        auto filepath = TORCH_MODEL_DIRECTORY + filename;

        LinearModel model(NUM_PHASE_PARAMS, 1);
        torch::serialize::InputArchive inputArchive;
        inputArchive.load_from(filepath);
        model.load(inputArchive);
        std::copy(model.fc1->weight.data_ptr<float>(), model.fc1->weight.data_ptr<float>() + NUM_PHASE_PARAMS, weights);

        // Scale the weights
        for (int i = 0; i < NUM_PHASE_PARAMS; ++i) {
            weights[i] *= (float)EVAL_SCALE;
        }
    }

    void EvalBuilder::save(const float* weights, const std::string& filename, bool round) {
        std::ofstream outFile(WEIGHT_DIRECTORY + filename, std::ios::binary);

        if (round) {
            for (int i = 0; i < NUM_EVAL_PARAMS; ++i) {
                // Write the weights
                auto w = (short)std::roundf(weights[i]);
                outFile.write(reinterpret_cast<const char *>(&w), sizeof(short));
            }
        } else {
            outFile.write(reinterpret_cast<const char *>(weights), sizeof(float) * NUM_EVAL_PARAMS);
        }

        outFile.close();
        std::cout << "Weights saved to " << filename << std::endl;
    }

    void EvalBuilder::load(float* weights, const std::string& filename) {
        std::ifstream file(WEIGHT_DIRECTORY + filename, std::ios::binary);

        if (!file.is_open()) {
            std::cout << "Error opening file: " << filename << std::endl;
            std::exit(1);
        }

        file.read(reinterpret_cast<char *>(weights), sizeof(float) * NUM_EVAL_PARAMS);

        file.close();
        std::cout << "Weights loaded from " << filename << std::endl;
    }

    void EvalBuilder::load_short(short* weights, const std::string& filename) {
        std::ifstream file(WEIGHT_DIRECTORY + filename, std::ios::binary);

        if (!file.is_open()) {
            std::cout << "Error opening file: " << filename << std::endl;
            std::exit(1);
        }
        if (filename.find("raw") != std::string::npos) {
            for (auto i = 0; i < NUM_EVAL_PARAMS; ++i) {
                // Write the weights
                float wf; file.read(reinterpret_cast<char *>(&wf), sizeof(float));
                weights[i] = (short)std::roundf(wf * (float)EVAL_SCALE);
            }
        }
        else
            file.read(reinterpret_cast<char *>(weights), sizeof(short) * NUM_EVAL_PARAMS);

        file.close();
        std::cout << "Weights loaded from " << filename << std::endl;
    }
} // engine::eval