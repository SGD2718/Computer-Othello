//
// Created by Benjamin Lee on 2/12/24.
//

#ifndef OTHELLO_SCOREDISCWIDGET_H
#define OTHELLO_SCOREDISCWIDGET_H

#include "QtInclude.h"

namespace gui {

    class ScoreDiscWidget: public QWidget {
        Q_OBJECT
        public:
            ScoreDiscWidget(QWidget* parent, QColor color);

        protected:
            void paintEvent(QPaintEvent *event) override;

        private:
            QColor color;
    };

} // gui

#endif //OTHELLO_SCOREDISCWIDGET_H
