//
// Created by Benjamin Lee on 2/25/24.
//

#ifndef OTHELLO_AIWORKER_H
#define OTHELLO_AIWORKER_H

#include "QtInclude.h"
#include "../Engine/Engine.h"

namespace gui {

    class AiWorker: public QObject {
        Q_OBJECT
    public:
        AiWorker() : cancelled(new bool(false)) {}
        explicit AiWorker(bool *cancelled) : cancelled(cancelled) {}

        void cancel();

    public slots:
        void wait_for_search(engine::SearchTask* searchTask);

    signals:
        void play_ai_move(int position, bool isAI = true);

    private:
        bool *cancelled;
    };

} // gui

#endif //OTHELLO_AIWORKER_H
