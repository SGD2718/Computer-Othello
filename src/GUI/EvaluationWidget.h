//
// Created by Benjamin Lee on 2/15/24.
//

#ifndef OTHELLO_EVALUATIONWIDGET_H
#define OTHELLO_EVALUATIONWIDGET_H

#include "../Engine/Search/SearchStructs.h"
#include "QtInclude.h"

namespace gui {

    struct EvaluationPoint {
        EvaluationPoint() = default;
        EvaluationPoint(int value, int depth, int discCount) :
            value(value), depth(depth), discCount(discCount) {}
        int value = 0;
        int depth = 0;
        int discCount = 0;
    };


    class EvaluationWorker: public QObject {
        Q_OBJECT
    public slots:
        [[noreturn]] void update_evaluation_task(engine::SearchTask* searchTask, const int* evaluationIndex);
    signals:
        void new_evaluation(EvaluationPoint evaluation);
    };

    class EvaluationWidget: public QWidget {
        Q_OBJECT
        QThread* evaluationWorkerThread;
    public:
        explicit EvaluationWidget(QWidget* parent, int barMax = 20, int barWidth = 30, int barHeight = 500);
        ~EvaluationWidget() override {
            delete this->evaluationIndex;
            delete this->evaluationWorker;
            delete this->evaluationWorkerThread;
            delete[] this->evaluations;
        }

        void set_evaluation_index(int index);

        void reset();
        void start_evaluation_task(engine::SearchTask* searchTask);

        static const int NUM_EVALUATIONS = 120;

    public slots:
        void set_evaluation_bar(EvaluationPoint evaluation);

    signals:
        void handle_start_evaluation_task(engine::SearchTask* searchTask, int* pEvaluationIndex);

    private:
        QLabel* evaluationLabel;
        QPropertyAnimation* scoreAnimation;
        QProgressBar* evaluationBar;

        EvaluationWorker* evaluationWorker;
        EvaluationPoint *evaluations;

        bool isChangingIndex = false;

        int* evaluationIndex = nullptr;

        void animate_bar(int value);
    };
} // gui

#endif //OTHELLO_EVALUATIONWIDGET_H
