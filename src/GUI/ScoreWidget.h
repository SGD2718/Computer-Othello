//
// Created by Benjamin Lee on 2/12/24.
//

#ifndef OTHELLO_SCOREWIDGET_H
#define OTHELLO_SCOREWIDGET_H

#include "QtInclude.h"
#include "CellWidget.h"

namespace gui {

    class ScoreWidget : public QWidget {
    Q_OBJECT

    public:
        ScoreWidget(QWidget* parent, int cellSize, int fontSize);
        void update_scores(int blackScore, int whiteScore);
    protected:

    private:
        int blackScore{};
        int whiteScore{};

        int fontSize;
        int cellSize;

        QLabel* scoreLabelBlack;
        QLabel* scoreLabelWhite;
        QWidget* scoreDiscBlack;
        QWidget* scoreDiscWhite;
    };
} // gui

#endif //OTHELLO_SCOREWIDGET_H
