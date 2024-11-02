//
// Created by Benjamin Lee on 3/27/24.
//

#ifndef OTHELLO_INIT_H
#define OTHELLO_INIT_H

#include "Engine/Engine.h"

void init() {
    engine::eval::EvaluationFeatures::eval_init(WEIGHT_FILEPATH);
    engine::TranspositionTable::init_hash();
    engine::Engine::probcut_init();
}


#endif //OTHELLO_INIT_H
