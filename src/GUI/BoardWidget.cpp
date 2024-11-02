//
// Created by Benjamin Lee on 2/12/24.
//

#include "BoardWidget.h"
#include <sstream>

namespace gui {
    BoardWidget::BoardWidget(QWidget *parent, int boardSize):
            BoardWidget(parent, Game(), boardSize) {}

    BoardWidget::BoardWidget(QWidget *parent, const Game& game, int boardSize) :
            QWidget(parent), Game(game), boardSize(boardSize), fontSize(boardSize / 20), borderSize(boardSize / 32),
            padding(boardSize / 24), cellSize(boardSize / 8), cells() {
        auto* layout = new QGridLayout(this);

        layout->setSpacing(0);
        layout->setAlignment(Qt::AlignCenter);

        // add cells
        for (int i = 0; i < 64; i++) {
            auto* cell = new CellWidget(this, this, i, 1);
            layout->addWidget(cell, (i >> 3) + 1, (i & 7) + 1);
            this->cells.emplace_back(cell);
            cell->setFixedSize(cellSize, cellSize);
            connect(cell, &CellWidget::cell_clicked, this, &BoardWidget::handle_cell_clicked);
        }

        // add coordinate labels
        auto coordinateLabelSize = fontSize + padding * 2;
        for (auto i = 0; i < 8; i++) {
            auto* rowLabel = new QLabel(QString::number(i + 1), this);
            rowLabel->setAlignment(Qt::AlignCenter);
            rowLabel->setFont(QFont("Arial", fontSize));
            rowLabel->setFixedSize(coordinateLabelSize, cellSize);
            layout->addWidget(rowLabel, i + 1, 0);

            auto* columnLabel = new QLabel(QString(QChar('A' + i)), this);
            columnLabel->setAlignment(Qt::AlignCenter);
            columnLabel->setFont(QFont("Arial", fontSize));
            columnLabel->setFixedSize(cellSize, coordinateLabelSize);
            layout->addWidget(columnLabel, 0, i + 1);
        }

        // add spacer
        auto* spacer = new QWidget(this);
        spacer->setFixedSize(coordinateLabelSize, padding);
        layout->addWidget(spacer, 9, 0);

        // add score widget
        this->scoreWidget = new ScoreWidget(this, cellSize, fontSize + fontSize / 2);
        this->scoreWidget->update_scores(this->get_black_score(), this->get_white_score());
        layout->addWidget(this->scoreWidget, 10, 1, 1, 8);

        // add spacer
        spacer = new QWidget(this);
        spacer->setFixedSize(coordinateLabelSize, padding);
        layout->addWidget(spacer, 11, 0);

        // add move history label
        this->moveHistoryLabel = new QLabel(this);
        this->moveHistoryLabel->setAlignment(Qt::AlignCenter);
        this->moveHistoryLabel->setFont(QFont("Arial", 2 * fontSize / 3));
        this->moveHistoryLabel->setFixedWidth(boardSize);
        this->moveHistoryLabel->setWordWrap(true);
        this->moveHistoryLabel->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
        layout->addWidget(this->moveHistoryLabel, 12, 1, 1, 8);

        this->setLayout(layout);
        this->show();
    }

    void BoardWidget::handle_cell_clicked(int position, bool isAI) {
        if ( (this->inputEnabled || isAI) && (this->get_legal_moves() >> position & 1)) {
            // play move and update board
            this->play_move((uint8_t) position);
            auto legalMoves = this->get_legal_moves();

            // if no legal moves, pass
            if (legalMoves == 0) {
                this->pass();
                legalMoves = this->get_legal_moves();
            }
            emit this->played_move(position, this);

            // if no legal moves after pass, game over
            if (legalMoves == 0) {
                this->undo_move();
                emit this->game_over(this->get_bitboard().get_disc_difference(), this);
            }

            this->update_display();
        } else {
            std::cout << "\033[31m YOU'RE NOT ALLOWED TO MOVE HERE: " << Move(this->get_bitboard(), position) << "\033[0m" << std::endl;
        }
    }

    void BoardWidget::update_display() {
        // update move history label
        std::stringstream ss;
        for (auto &move: this->get_move_history()) {
            if (!move.is_pass()) ss << move << ' ';
        }
        this->moveHistoryLabel->setText(QString(ss.str().c_str()));

        // update cells
        auto legalMoves = this->get_legal_moves();

        for (auto cell : this->cells) {
            cell->update_disc(this->get_black_bitboard(), this->get_white_bitboard(),
                              legalMoves, this->highlighted, this->is_black_to_move());
        }

        this->scoreWidget->update_scores(this->get_black_score(), this->get_white_score());

        this->update();
    }

    void BoardWidget::highlight_cell(int position) {
        if (position < 0 || position > 63) return;
        auto state = (DiscState)( (unsigned long long)DiscState::HIGHLIGHT_BIT | ( get_black_bitboard() >> position & 1 ) | ( get_white_bitboard() >> (position - 1) & 2 ) );
        this->highlighted |= 1ULL << position;
        this->cells[position]->set_disc(state);
    }

    void BoardWidget::preview_cell(int position) {
        if (position < 0 || position > 63) return;
        auto state = (DiscState)( (unsigned long long)DiscState::LEGAL_BIT & ( get_black_bitboard() >> position & 1 ) | ( get_white_bitboard() >> (position - 1) & 2 ) );
        this->cells[position]->set_disc(state);
    }

    void BoardWidget::set_input_enabled(bool inputEnabled) {
        this->inputEnabled = inputEnabled;
    }

    void BoardWidget::enable_input() {
        this->inputEnabled = true;
    }

    void BoardWidget::disable_input() {
        this->inputEnabled = false;
    }

    void BoardWidget::rehighlight_cells() {
        this->highlighted = 0;
        auto history = this->get_move_history();
        auto i = this->get_move_index() - 1;
        while (history[i].is_pass()) --i;
        auto parity = i & 1;
        while ((i & 1) == parity) {
            this->highlighted |= 1ULL << history[i].x;
            if (history[--i].is_pass()) --i;
        }
    }
} // gui