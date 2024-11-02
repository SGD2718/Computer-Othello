//
// Created by Benjamin Lee on 2/12/24.
//

#include "ScoreWidget.h"
#include "ScoreDiscWidget.h"

namespace gui {
    ScoreWidget::ScoreWidget(QWidget* parent, int cellSize, int fontSize) :
            QWidget(parent), cellSize(cellSize), fontSize(fontSize) {

        scoreLabelBlack = new QLabel();
        scoreLabelWhite = new QLabel();

        scoreLabelBlack->setFont(QFont("Arial", fontSize));
        scoreLabelWhite->setFont(QFont("Arial", fontSize));

        auto layout = new QHBoxLayout();
        layout->setAlignment(Qt::AlignCenter);
        auto scoreLabelHyphen = new QLabel("-", layout->widget());
        scoreLabelHyphen->setFont(QFont("Arial", fontSize));

        this->scoreDiscBlack = new ScoreDiscWidget(layout->widget(), CellWidget::BLACK_COLOR);
        this->scoreDiscBlack->setFixedSize(cellSize, cellSize);
        this->scoreDiscWhite = new ScoreDiscWidget(layout->widget(), CellWidget::WHITE_COLOR);
        this->scoreDiscWhite->setFixedSize(cellSize, cellSize);

        layout->addWidget(scoreDiscBlack);
        layout->addWidget(this->scoreLabelBlack);
        layout->addWidget(scoreLabelHyphen);
        layout->addWidget(this->scoreLabelWhite);
        layout->addWidget(scoreDiscWhite);

        layout->setSpacing(cellSize / 2);
        layout->setContentsMargins(0, 0, 0, 0);

        this->setLayout(layout);
        this->update();
    }

    void ScoreWidget::update_scores(int blackScore, int whiteScore) {
        this->blackScore = blackScore;
        this->whiteScore = whiteScore;
        scoreLabelBlack->setText(QString::number(blackScore));
        scoreLabelWhite->setText(QString::number(whiteScore));
    }

    /*void ScoreWidget::paintEvent(QPaintEvent *event) {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing, true);

        auto discPadding = cellSize / 8;
        auto discRectBlack = scoreDiscBlack->rect().adjusted(discPadding, discPadding, -discPadding, -discPadding);
        auto discRectWhite = scoreDiscWhite->rect().adjusted(discPadding, discPadding, -discPadding, -discPadding);

        painter.setBrush(CellWidget::BLACK_COLOR);
        painter.drawEllipse(discRectBlack);

        painter.setBrush(CellWidget::WHITE_COLOR);
        painter.drawEllipse(discRectWhite);
    }*/
} // gui