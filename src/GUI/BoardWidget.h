//
// Created by Benjamin Lee on 2/12/24.
//

#ifndef OTHELLO_BOARDWIDGET_H
#define OTHELLO_BOARDWIDGET_H

#include "QtInclude.h"
#include "CellWidget.h"
#include "ScoreWidget.h"
#include "../Game/Game.h"


namespace gui {

    class BoardWidget: public QWidget, public Game {
        Q_OBJECT

    public:
        explicit BoardWidget(QWidget *parent, int boardSize = 500);
        BoardWidget(QWidget *parent, const Game& game, int boardSize = 500);

        void update_display();
        void disable_input();
        void enable_input();
        void set_input_enabled(bool inputEnabled);
        void highlight_cell(int position);
        void preview_cell(int position);
        void rehighlight_cells();

        [[nodiscard]] inline bool input_enabled() const {
            return this->inputEnabled;
        }

    signals:
        void played_move(int position, Game* pGame);
        void game_over(int value, Game* pGame);

    public slots:
        void handle_cell_clicked(int positions, bool isAI = false);

    private:
        QVector<CellWidget*> cells;
        ScoreWidget* scoreWidget;
        QLabel* moveHistoryLabel;

        int boardSize;
        int fontSize;
        int borderSize;
        int padding;
        int cellSize;

        uint64_t highlighted = 0;

        bool inputEnabled = true;
    };

} // gui

#endif //OTHELLO_BOARDWIDGET_H
