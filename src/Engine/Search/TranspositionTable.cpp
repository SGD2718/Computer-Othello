//
// Created by Benjamin Lee on 2/28/24.
//
#include "TranspositionTable.h"
#include <random>
#include <fstream>
#include <sstream>

namespace engine {
    uint32_t TranspositionTable::HASH_KEYS[8][65536] = {0};

    TranspositionTable::TranspositionTable() {
        this->table = new HashEntry[1ULL << HASH_BITS];
    }

    int TranspositionTable::init_hash() {
        // read hash keys from file
        std::ifstream file(HASH_FILE);
        if (!file.is_open()) {
            std::cerr << "Error: could not open file " << HASH_FILE << std::endl;
            exit(1);
        }

        std::string str;
        for (auto &row : TranspositionTable::HASH_KEYS) {
            for (auto &key : row) {
                std::getline(file, str, ',');
                key = static_cast<uint32_t>(std::stoul(str)) & HASH_MASK;
            }
        }

        return 0;
    }

    void TranspositionTable::init_hash_file(int numHashBits) {
        std::random_device rd; // obtain a random number from hardware
        std::mt19937 gen(rd()); // Mersenne Twister random number engine (32-bit)
        std::uniform_int_distribution<uint32_t> dis(0,std::numeric_limits<uint32_t>::max()); // Uniform distribution

        std::array<uint32_t, 524288> keys{0};

        int minSetBits = numHashBits / 6;
        auto hashMask = (1UL << numHashBits) - 1UL;

        // generate initial hash keys
        std::cout << "generating hash keys..." << std::endl;
        for (auto &key : keys) {
            do {
                key = dis(gen) & hashMask;
            } while (__builtin_popcount(key) < minSetBits);
        }

        // ensure that the hash keys are unique
        bool done = false;
        for (int i = 0; !done; ++i) {
            std::cout << "sorting hash keys..." << std::endl;
            std::sort(keys.begin(), keys.end());

            done = true;
            int numReplaced = 0;
            for (int j = 0; j < 524287; ++j) {
                if (keys[j] == keys[j + 1]) {
                    do {
                        keys[j] = dis(gen) & hashMask;
                    } while (__builtin_popcount(keys[j]) < minSetBits || keys[j] == keys[j + 1]);
                    ++numReplaced;
                    done = false;
                }
            }

            std::cout << "Iteration " << i << ": " << numReplaced << " duplicates found." << std::endl;
        }

        std::cout << "shuffling..." << std::endl;
        std::shuffle(keys.begin(), keys.end(), gen);
        std::ofstream file(HASH_FILE);
        std::cout << "writing to file..." << std::endl;
        for (auto &key : keys) {
            file << key << ",";
        }
        file.close();
        std::cout << "Done" << std::endl;
    }
} // engine