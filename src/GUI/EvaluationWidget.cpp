//
// Created by Benjamin Lee on 2/15/24.
//

#include "EvaluationWidget.h"

namespace gui {
    EvaluationWidget::EvaluationWidget(QWidget *parent, int barMax, int barWidth, int barHeight) :
            evaluations(
                    new EvaluationPoint[NUM_EVALUATIONS]), // not sure how many turns (counting passes) can happen in a game but definitely not 120
            evaluationIndex(new int(0)) {
        this->evaluationLabel = new QLabel(this);
        this->evaluationLabel->setAlignment(Qt::AlignTop);

        this->evaluationLabel->setStyleSheet("QLabel { color: white; }");

        this->evaluationBar = new QProgressBar(this);
        this->evaluationBar->setOrientation(Qt::Vertical);
        this->evaluationBar->setFixedSize(barWidth, barHeight);
        this->evaluationBar->setRange(0, barMax << 9);
        this->evaluationBar->setValue(barMax << 8);
        this->evaluationBar->setProperty("full", false);

        // evaluation bar should fill the entire widget width-wise. The progress color should be black, and the background color should be white. It also should not have the percentage text.
        this->evaluationBar->setTextVisible(false);
        this->evaluationBar->setStyleSheet(
                "QProgressBar { "
                "   border: 2px solid grey; "
                "   background-color: white; "
                "   border-radius: 10px"
                "}"
                "QProgressBar::chunk { "
                "   background-color: black; "
                "   border-bottom-left-radius: 8px;"
                "   border-bottom-right-radius: 8px;"
                "}"
                "QProgressBar[full=\"true\"]::chunk { "
                "   border-top-left-radius: 8px; "
                "   border-top-right-radius: 8px; "
                "}"
                "QProgressBar[full=\"false\"]::chunk { "
                "   border-top-left-radius: 0px; "
                "   border-top-right-radius: 0px; "
                "}"
        );

        this->scoreAnimation = new QPropertyAnimation(this->evaluationBar, "value");
        this->scoreAnimation->setDuration(500);
        this->scoreAnimation->setEasingCurve(QEasingCurve::OutCubic);

        auto layout = new QVBoxLayout(this);
        layout->addWidget(this->evaluationLabel);
        layout->addWidget(this->evaluationBar);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->setSpacing(0);
        this->setLayout(layout);

        // initialize evaluation worker and thread
        this->evaluationWorkerThread = new QThread;
        this->evaluationWorker = new EvaluationWorker;

        evaluationWorker->moveToThread(this->evaluationWorkerThread);
        connect(evaluationWorker, &EvaluationWorker::new_evaluation, this, &EvaluationWidget::set_evaluation_bar);
        connect(this, &EvaluationWidget::handle_start_evaluation_task, this->evaluationWorker,
                &EvaluationWorker::update_evaluation_task);
        this->evaluationWorkerThread->start();
    }

    void EvaluationWidget::set_evaluation_bar(EvaluationPoint evaluation) {
        if (this->evaluationIndex != nullptr) {
            auto pEvaluation = &this->evaluations[*(this->evaluationIndex)];

            // only update the evaluation if the new evaluation is deeper than the current one
            if (!isChangingIndex && evaluation.depth > pEvaluation->depth &&
                    (pEvaluation->depth == 0 || pEvaluation->discCount == 0 ||
                    pEvaluation->discCount == evaluation.discCount)) {
                *pEvaluation = evaluation;
                this->animate_bar(evaluation.value);
            }
        }
    }

    void EvaluationWidget::set_evaluation_index(int index) {
        if (index != *this->evaluationIndex && index >= 0 && index < NUM_EVALUATIONS) {
            this->isChangingIndex = true;
            // clear evaluations if a move was undone and a new move was tried.
            if (index == *this->evaluationIndex + 1) {
                for (int i = *this->evaluationIndex; i < NUM_EVALUATIONS; ++i) {
                    // stop clearing once we reach an empty evaluation
                    if (this->evaluations[i].discCount == 0)
                        break;
                    this->evaluations[i].value = 0;
                    this->evaluations[i].depth = 0;
                    this->evaluations[i].discCount = 0;
                }
            }

            *(this->evaluationIndex) = index;
            this->animate_bar(this->evaluations[index].value);
            this->isChangingIndex = false;
        }
    }

    void EvaluationWidget::reset() {
        this->set_evaluation_index(0);
    }

    void EvaluationWidget::start_evaluation_task(engine::SearchTask *searchTask) {
        emit handle_start_evaluation_task(searchTask, this->evaluationIndex);
    }

    void EvaluationWidget::animate_bar(int value) {
        value = std::clamp(value, -SCORE_MAX, SCORE_MAX);
        if (value >= WIN)
            value -= WIN;
        if (value <= LOSS)
            value -= LOSS;

        this->evaluationLabel->setText(QString::number(value));

        value <<= 8; // scale to fit the progress bar range for smooth animation
        auto bound = this->evaluationBar->maximum() / 2;
        if (value > bound)
            value = bound;
        else if (value < -bound)
            value = -bound;
        value += bound;

        this->evaluationBar->setProperty("full", value - this->evaluationBar->maximum() == 0);
        this->evaluationBar->style()->unpolish(this->evaluationBar);
        this->evaluationBar->style()->polish(this->evaluationBar);
        this->scoreAnimation->stop();
        this->scoreAnimation->setStartValue(this->evaluationBar->value());
        this->scoreAnimation->setEndValue(value);
        this->scoreAnimation->start();
    }

    [[noreturn]] void
    EvaluationWorker::update_evaluation_task(engine::SearchTask *searchTask, const int *evaluationIndex) {
        // even index -> black to move, odd index -> white to move (negate)
        int value;
        while (true) {
            if (searchTask->search != nullptr && searchTask->search->depth > 0) {
                value = *evaluationIndex & 1 ? -searchTask->search->value : searchTask->search->value;
                emit new_evaluation({value, searchTask->search->depth, searchTask->search->rootDiscCount});
            }
            QThread::msleep(1);
        }
    }
} // gui