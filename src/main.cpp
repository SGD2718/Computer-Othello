#include <iostream>
#include "Game/Game.h"
#include "GUI/BoardWidget.h"
#include "GUI/OthelloGui.h"
#include "Engine/Evaluation/EvalBuilder.h"
#include "Engine/Engine.h"
#include "Init.h"

#if RUN_TRAINING_MODE
    int main(int argc, char *argv[]) {
        QApplication app(argc, argv);
        std::cout << "Starting..." << std::endl;
        engine::eval::EvalBuilder::init();
        //engine::eval::EvalBuilder::init_batches(8);
        //engine::eval::EvalBuilder::combine_batches(8, 213, 1);
        engine::eval::EvalBuilder::train("0000001.bin", 1000, 120000, true, true, 1);

        engine::eval::EvalBuilder::test(MODEL_NAME ".bin", "0000000.txt", "0");
        return QApplication::exec();
    }
#elif TUNE_PROBCUT
    int main() {
        init();
        auto e = engine::Engine();
        e.collect_prob_cut_data(1000, 6);
        return 0;
    }
#else
    int main(int argc, char *argv[]) {
        init();
        QApplication app(argc, argv);
        auto othelloGui = new gui::OthelloGUI();
        othelloGui->show();
        return QApplication::exec();
    }
#endif



