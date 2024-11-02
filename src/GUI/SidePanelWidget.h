//
// Created by Benjamin Lee on 2/14/24.
//

#ifndef OTHELLO_SIDEPANELWIDGET_H
#define OTHELLO_SIDEPANELWIDGET_H

#include "QtInclude.h"
#include "AiConfigWidget.h"


namespace gui {
    class SidePanelWidget : public QWidget {
    Q_OBJECT

    public:
        SidePanelWidget(QWidget *parent = nullptr);
        ~SidePanelWidget() = default;

        [[nodiscard]] inline bool is_black_ai() const {return aiConfigWidget->is_black_ai();}
        [[nodiscard]] inline bool is_white_ai() const {return aiConfigWidget->is_white_ai();}
        [[nodiscard]] inline int get_search_time() const {return aiConfigWidget->get_search_time();}

    signals:
        void undo_pressed();
        void redo_pressed();
        void restart_pressed();
        void start_pressed();

    public slots:

    protected:
        AIConfigWidget* aiConfigWidget;
        QPushButton* undoButton;
        QPushButton* redoButton;
        QPushButton* restartButton;
    };
}

#endif //OTHELLO_SIDEPANELWIDGET_H
