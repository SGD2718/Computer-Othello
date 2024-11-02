//
// Created by Benjamin Lee on 4/13/24.
//

#ifndef OTHELLO_LINEARMODEL_H
#define OTHELLO_LINEARMODEL_H

#include <torch/torch.h>
#include "Evaluation.h"

namespace engine::eval {

    struct LinearModel: torch::nn::Module {
        torch::nn::Linear fc1{nullptr};

        explicit LinearModel(int64_t inputSize = NUM_PHASE_PARAMS, int64_t outputSize = 1) {
            this->fc1 = this->register_module("fc1", torch::nn::Linear(inputSize, outputSize));
        }

        inline torch::Tensor forward(torch::Tensor x) {
            x = this->fc1->forward(x);
            return x;
        }
    };

} // engine::eval

#endif //OTHELLO_LINEARMODEL_H
