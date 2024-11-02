//
// Created by Benjamin Lee on 2/13/24.
//

#ifndef OTHELLO_AICONFIGWIDGET_H
#define OTHELLO_AICONFIGWIDGET_H

#include "QtInclude.h"

namespace gui {

    class AIConfigWidget: public QWidget {
        Q_OBJECT
    public:
        explicit AIConfigWidget(QWidget* parent);
        ~AIConfigWidget() override = default;

        [[nodiscard]] bool is_black_ai() const;
        [[nodiscard]] bool is_white_ai() const;
        [[nodiscard]] int get_search_time() const;

    signals:
        void start_pressed();

    private:
        QSpinBox* searchTimeSpinBox;
        QCheckBox* blackAICheckBox;
        QCheckBox* whiteAICheckBox;
        QPushButton* startButton;
    };

} // gui

#endif //OTHELLO_AICONFIGWIDGET_H
