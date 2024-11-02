//
// Created by Benjamin Lee on 2/14/24.
//

#include "SidePanelWidget.h"

namespace gui {
    SidePanelWidget::SidePanelWidget(QWidget *parent) : QWidget(parent) {
        setFixedSize(300, 800);
        setStyleSheet("background-color: #505050;");
        setLayout(new QVBoxLayout);
        auto frame = new QFrame(this);
        frame->setStyleSheet("background-color: #505050;");
        frame->setFrameStyle(QFrame::Panel | QFrame::Raised);
        frame->setLineWidth(2);
        layout()->addWidget(frame);
        layout()->setAlignment(Qt::AlignTop);
        layout()->setSpacing(10);
        layout()->setContentsMargins(0, 0, 0, 0);

        this->aiConfigWidget = new AIConfigWidget(this);
        layout()->addWidget(aiConfigWidget);

        this->undoButton = new QPushButton("Undo", this);
        this->redoButton = new QPushButton("Redo", this);
        this->restartButton = new QPushButton("Restart", this);

        layout()->addWidget(this->undoButton);
        layout()->addWidget(this->redoButton);
        layout()->addWidget(this->restartButton);

        connect(this->redoButton, &QPushButton::pressed, [this]() {emit this->redo_pressed();});
        connect(this->undoButton, &QPushButton::pressed, [this]() {emit this->undo_pressed();});
        connect(this->restartButton, &QPushButton::pressed, [this]() {emit this->restart_pressed();});
        connect(this->aiConfigWidget, &AIConfigWidget::start_pressed, [this]() {emit this->start_pressed();});
    }
}