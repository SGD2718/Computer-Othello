//
// Created by Benjamin Lee on 2/12/24.
//

#include "ScoreDiscWidget.h"

namespace gui {
    ScoreDiscWidget::ScoreDiscWidget(QWidget *parent, QColor color) : QWidget(parent), color(color) {
        this->update();
    }

    void ScoreDiscWidget::paintEvent(QPaintEvent *event) {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.setBrush(this->color);
        painter.setPen(Qt::NoPen);

        auto padding = this->width() / 8;
        auto rect = this->rect().adjusted(padding, padding, -padding, -padding);

        painter.drawEllipse(rect);
    }
} // gui