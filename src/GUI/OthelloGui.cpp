//
// Created by Benjamin Lee on 2/13/24.
//

#include "OthelloGui.h"

namespace gui {
    OthelloGUI::OthelloGUI(QWidget *parent) : QWidget(parent) {
        setStyleSheet("background-color: #404040; color: #ffffff; font-size: 20px; font-family: Arial; font-weight: bold;");
        auto* layout = new QHBoxLayout;
        setLayout(layout);
        layout->setAlignment(Qt::AlignTop);
        layout->setSpacing(10);

        this->boardWidget = new BoardWidget(this, 500);
        layout->addWidget(boardWidget);
        boardWidget->update_display();

        this->evaluationWidget = new EvaluationWidget(this, 16, 30, 495);
        auto boardLayout = (QGridLayout*)boardWidget->layout();
        boardLayout->addItem(new QSpacerItem(30,500), 0, 9, 9, 1);
        boardLayout->addWidget(evaluationWidget, 0,10,9,1);

        this->sidePanelWidget = new SidePanelWidget(this);
        layout->addWidget(sidePanelWidget);

        connect(sidePanelWidget, &SidePanelWidget::undo_pressed, this, &OthelloGUI::handle_undo_pressed);
        connect(sidePanelWidget, &SidePanelWidget::redo_pressed, this, &OthelloGUI::handle_redo_pressed);
        connect(sidePanelWidget, &SidePanelWidget::restart_pressed, this, &OthelloGUI::handle_restart_pressed);
        connect(sidePanelWidget, &SidePanelWidget::start_pressed, this, &OthelloGUI::play_ai_move);
        connect(boardWidget, &BoardWidget::played_move, this, &OthelloGUI::handle_played_move);

        // initialize AI worker and thread
        this->aiWorker = new AiWorker;
        this->aiWorkerThread = new QThread;
        this->aiWorker->moveToThread(this->aiWorkerThread);
        connect(this, &OthelloGUI::get_ai_move, this->aiWorker, &AiWorker::wait_for_search);
        connect(this->aiWorker, &AiWorker::play_ai_move, this->boardWidget, &BoardWidget::handle_cell_clicked);
        this->aiWorkerThread->start();

        // initialize search task and evaluation thread for AI
        this->searchTask = this->engine.search_task(*this->boardWidget);
        this->evaluationWidget->start_evaluation_task(this->searchTask);
    }

    void OthelloGUI::handle_redo_pressed() {
        if (this->boardWidget->redo_move(true)) {
            this->aiWorker->cancel();
            this->searchTask->stop();
            this->searchTask->qt_wait_for_completion();
            this->evaluationWidget->set_evaluation_index(this->boardWidget->get_move_index());
            this->searchTask->set_board(this->boardWidget->get_bitboard());
            this->engine.continue_search_task(this->searchTask, this->boardWidget->get_last_move().is_pass());
            this->boardWidget->rehighlight_cells();
            this->boardWidget->enable_input();
        }
        this->boardWidget->update_display();
    }

    void OthelloGUI::handle_undo_pressed() {
        if (this->boardWidget->undo_move(true)) {
            this->aiWorker->cancel();
            this->searchTask->stop();
            this->searchTask->qt_wait_for_completion();
            this->evaluationWidget->set_evaluation_index(this->boardWidget->get_move_index());
            this->searchTask->set_board(this->boardWidget->get_bitboard());
            this->engine.continue_search_task(this->searchTask, this->boardWidget->get_last_move().is_pass());
            this->boardWidget->rehighlight_cells();
            this->boardWidget->enable_input();
        }
        this->boardWidget->update_display();
    }

    void OthelloGUI::handle_restart_pressed() {
        if (this->boardWidget->get_move_index() > 0) {
            this->boardWidget->reset();
            this->boardWidget->rehighlight_cells();
            this->boardWidget->update_display();
            this->boardWidget->enable_input();
            this->aiWorker->cancel();
            this->searchTask->stop();
            this->searchTask->qt_wait_for_completion();
            this->searchTask->set_board(this->boardWidget->get_bitboard());
            this->evaluationWidget->reset();
            this->engine.continue_search_task(this->searchTask, this->boardWidget->get_last_move().is_pass());
        }
    }

    void OthelloGUI::handle_played_move(int position) {
        this->searchTask->stop();
        this->boardWidget->rehighlight_cells();
        this->boardWidget->update_display();
        this->searchTask->qt_wait_for_completion();
        this->evaluationWidget->set_evaluation_index(this->boardWidget->get_move_index());
        this->searchTask->set_board(this->boardWidget->get_bitboard());

        this->boardWidget->enable_input();
        if (this->is_ai_move()) {
            this->play_ai_move();
        }
        else {
            this->engine.continue_search_task(this->searchTask, this->boardWidget->get_last_move().is_pass());
        }
    }

    bool OthelloGUI::is_ai_move() {
        return !this->boardWidget->is_terminal() &&
               ((this->boardWidget->is_black_to_move() && this->sidePanelWidget->is_black_ai()) ||
                (this->boardWidget->is_white_to_move() && this->sidePanelWidget->is_white_ai()));
    }

    void OthelloGUI::play_ai_move() {
        if (is_ai_move() && this->boardWidget->input_enabled()) {
            this->boardWidget->disable_input();
            // set endgame flag if needed
            if (this->boardWidget->get_disc_count() < 64 - PERFECT_SEARCH_DEPTH) {
                // make timer thread for mid-game search
                engine::Engine::make_timer_thread((double)this->sidePanelWidget->get_search_time(),
                                                  this->searchTask->running, false).detach();
            }
            // restart search task if needed
            if (!*this->searchTask->running) {
                this->engine.continue_search_task(this->searchTask, this->boardWidget->get_last_move().is_pass());
            }
            emit get_ai_move(this->searchTask);
        }
    }
} // gui