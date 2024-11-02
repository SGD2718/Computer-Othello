//
// Created by Benjamin Lee on 2/13/24.
//

#include "AiConfigWidget.h"

namespace gui {
    AIConfigWidget::AIConfigWidget(QWidget* parent) : QWidget(parent) {
        searchTimeSpinBox = new QSpinBox(this);
        searchTimeSpinBox->setRange(1, 100);
        searchTimeSpinBox->setValue(1);
        searchTimeSpinBox->setPrefix("Search Time: ");
        searchTimeSpinBox->setSuffix("s");

        blackAICheckBox = new QCheckBox("Black AI", this);
        whiteAICheckBox = new QCheckBox("White AI", this);
        startButton = new QPushButton("Start", this);

        auto* layout = new QGridLayout(this);
        layout->setAlignment(Qt::AlignCenter);
        layout->addWidget(searchTimeSpinBox, 0, 0, 1, 3);
        layout->addWidget(blackAICheckBox, 1, 0);
        layout->addWidget(whiteAICheckBox, 1, 2);
        layout->addWidget(startButton, 2, 0, 1, 3);
        layout->setSpacing(10);
        setLayout(layout);

        connect(startButton, &QPushButton::pressed, [this]() {emit start_pressed();});
    }

    bool AIConfigWidget::is_black_ai() const {
        return blackAICheckBox->isChecked();
    }

    bool AIConfigWidget::is_white_ai() const {
        return whiteAICheckBox->isChecked();
    }

    int AIConfigWidget::get_search_time() const {
        return searchTimeSpinBox->value();
    }
} // gui