//
// Created by Benjamin Lee on 2/12/24.
//

#include "CellWidget.h"

namespace gui {
    CellWidget::CellWidget(QWidget *parent, Game* pGame, int position, int borderSize):
            QWidget(parent), position(position), borderSize(borderSize) {
        this->update_disc(pGame->get_black_bitboard(), pGame->get_white_bitboard(), pGame->get_legal_moves(), 0, pGame->is_black_to_move());
    }

    void CellWidget::update_disc(uint64_t black, uint64_t white, uint64_t legal, uint64_t highlighted, bool blackToMove) {
        auto positionMask = 1ULL << this->position;

        if ( ( positionMask & (legal | black | white) ) == 0 )
            this->set_disc(DiscState::EMPTY);
        else {
            auto stateBits = ( (highlighted >> this->position & 1) << 3 );
            if (positionMask & legal) {
                stateBits |= (uint64_t) DiscState::LEGAL_BIT;
                stateBits |= blackToMove ? 1 : 2;
            } else {
                stateBits |= (black >> this->position) & 1;
                stateBits |= ((white >> this->position) & 1) << 1;
                if ((stateBits & 0b11) == 0b11)
                    std::cerr << "Invalid disc state: " << std::bitset<8>(stateBits) << ". Black = "
                              << std::bitset<64>(black) << ", White = " << std::bitset<64>(white) << std::endl;
            }
            this->set_disc((DiscState)stateBits);
        }
    }

    void CellWidget::set_position(int pos) {
        this->position = pos;
    }

    void CellWidget::set_disc(DiscState discState) {
        this->state = discState;
        this->update();
    }

    void CellWidget::draw_disc(QPainter &painter) {
        auto discPadding = this->width() / 8;
        auto discRect = rect().adjusted(discPadding, discPadding, -discPadding, -discPadding);

        switch ((DiscState)((char)this->state & ~(char)DiscState::HIGHLIGHT_BIT)) {
            case DiscState::BLACK:
                painter.setBrush(CellWidget::BLACK_COLOR);
                break;
            case DiscState::WHITE:
                painter.setBrush(CellWidget::WHITE_COLOR);
                break;
            case DiscState::BLACK_LEGAL:
                painter.setBrush(CellWidget::BLACK_LEGAL_COLOR);
                break;
            case DiscState::WHITE_LEGAL:
                painter.setBrush(CellWidget::WHITE_LEGAL_COLOR);
                break;
            case DiscState::EMPTY:
                return;
            default:
                std::cerr << "Invalid disc state: " << std::bitset<8>((char)this->state) << std::endl;
                return;
        }
        painter.drawEllipse(discRect);
    }

    void CellWidget::paintEvent(QPaintEvent *event) {
        Q_UNUSED(event)

        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing, true);

        // draw green background
        painter.setPen(QPen(Qt::black, this->borderSize));
        painter.setBrush((char)this->state & (char)DiscState::HIGHLIGHT_BIT ? CellWidget::HIGHLIGHT_COLOR : CellWidget::BG_COLOR);
        painter.drawRect(this->rect());
        painter.setPen(Qt::NoPen);

        this->draw_disc(painter);
    }

    void CellWidget::mousePressEvent(QMouseEvent *event) {
        if (event->button() == Qt::LeftButton) {
            emit cell_clicked(this->position, false);
        }
    }
} // gui