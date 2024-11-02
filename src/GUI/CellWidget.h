//
// Created by Benjamin Lee on 2/12/24.
//

#ifndef OTHELLO_CELLWIDGET_H
#define OTHELLO_CELLWIDGET_H

#include "QtInclude.h"
#include "../Game/Game.h"

namespace gui {
    enum class DiscState : char {
        EMPTY = 0b0000,
        BLACK = 0b0001,
        WHITE = 0b0010,
        BLACK_LEGAL = 0b0101,
        WHITE_LEGAL = 0b0110,
        EMPTY_HIGHLIGHT = 0b1000,
        BLACK_HIGHLIGHT = 0b1001,
        WHITE_HIGHLIGHT = 0b1010,
        BLACK_LEGAL_HIGHLIGHT = 0b1101,
        WHITE_LEGAL_HIGHLIGHT = 0b1110,
        HIGHLIGHT_BIT = 0b1000,
        LEGAL_BIT = 0b0100
    };

    class CellWidget : public QWidget {
    Q_OBJECT

    public:
        explicit CellWidget(QWidget *parent = nullptr, Game *pGame = nullptr, int position = 0, int borderSize = 2);

        void update_disc(uint64_t black, uint64_t white, uint64_t legal, uint64_t highlighted, bool blackToMove);

        void set_position(int pos);

        void set_disc(DiscState discState);

        constexpr static const QColor BG_COLOR = QColor(26, 184, 99);
        constexpr static const QColor HIGHLIGHT_COLOR = QColor(240, 100, 49);
        constexpr static const QColor BLACK_COLOR = QColor(15, 15, 15);
        constexpr static const QColor WHITE_COLOR = QColor(245, 245, 245);
        constexpr static const QColor BLACK_LEGAL_COLOR = QColor(15, 15, 15, 64);
        constexpr static const QColor WHITE_LEGAL_COLOR = QColor(245, 245, 245, 64);

    signals:
        void cell_clicked(int position, bool isAI = false);

    protected:
        void paintEvent(QPaintEvent *event) override;

        void mousePressEvent(QMouseEvent *event) override;

    private:
        void draw_disc(QPainter &painter);

        int position;
        DiscState state;

        int borderSize;
    };
}

#endif //OTHELLO_CELLWIDGET_H
