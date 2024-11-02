//
// Created by Benjamin Lee on 2/13/24.
//

#ifndef OTHELLO_OTHELLOGUI_H
#define OTHELLO_OTHELLOGUI_H

#include "../Engine/Engine.h"
#include "QtInclude.h"
#include "CellWidget.h"
#include "SidePanelWidget.h"
#include "BoardWidget.h"
#include "EvaluationWidget.h"
#include "AiWorker.h"

namespace gui {

    class OthelloGUI: public QWidget {
    Q_OBJECT
    public:
        OthelloGUI(QWidget* parent = nullptr);
        ~OthelloGUI() override = default;

    public slots:
        void handle_played_move(int position);
        void handle_undo_pressed();
        void handle_redo_pressed();
        void handle_restart_pressed();
        void play_ai_move();

    signals:
        void get_ai_move(engine::SearchTask* searchTask);

    private:
        BoardWidget* boardWidget;
        SidePanelWidget* sidePanelWidget;
        EvaluationWidget* evaluationWidget;
        QThread* aiWorkerThread;
        AiWorker* aiWorker;
        engine::Engine engine;
        engine::SearchTask* searchTask;
        int lastAIDepth = 0;

        bool is_ai_move();
    };

} // gui

#endif //OTHELLO_OTHELLOGUI_H
