//
// Created by Benjamin Lee on 2/25/24.
//

#include "AiWorker.h"

namespace gui {
    void AiWorker::wait_for_search(engine::SearchTask* searchTask) {
        *this->cancelled = false;

        searchTask->qt_wait_for_completion();
        Move move = searchTask->get_result().move;

        std::cout << move << std::endl;

        if (!*this->cancelled)
            emit play_ai_move((int) move.x, true);
        else
            *this->cancelled = false;
    }

    void AiWorker::cancel() {
        *this->cancelled = true;
    }

} // gui