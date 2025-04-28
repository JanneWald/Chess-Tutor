#include "chessboard.h"
#include "qapplication.h"
#include "confetticontroller.h"
#include <QPainter>
#include <QDir>
#include <iostream>
#include "chess.h"

using std::endl;

ChessBoard::ChessBoard(QWidget *parent)
    : QWidget(parent) {

    image.load(":/Assets/Assets/Board.png");

    blackPawn.load(":/Assets/Assets/BlackPawn.png");
    blackRook.load(":/Assets/Assets/BlackRook.png");
    blackBishop.load(":/Assets/Assets/BlackBishop.png");
    blackKnight.load(":/Assets/Assets/BlackKnight.png");
    blackQueen.load(":/Assets/Assets/BlackQueen.png");
    blackKing.load(":/Assets/Assets/BlackKing.png");

    whitePawn.load(":/Assets/Assets/WhitePawn.png");
    whiteRook.load(":/Assets/Assets/WhiteRook.png");
    whiteBishop.load(":/Assets/Assets/WhiteBishop.png");
    whiteKnight.load(":/Assets/Assets/WhiteKnight.png");
    whiteQueen.load(":/Assets/Assets/WhiteQueen.png");
    whiteKing.load(":/Assets/Assets/WhiteKing.png");



    setFixedSize(520, 520);
}

void ChessBoard::mousePressEvent(QMouseEvent *event){
    int col{event->pos().x() / 50};
    int row{event->pos().y() / 50};

    emit squareSelected(row, col);
}

void ChessBoard::setHintSquares(int fr, int fc, int tr, int tc) {
    hintMoveFr = fr; hintMoveFc = fc;
    hintMoveTr = tr; hintMoveTc = tc;
    hasHintMove = true;
    update();
}

void ChessBoard::clearHintMove() {
    hasHintMove = false;
    update();
}

void ChessBoard::clearHint() {
    hasHint = false;
    update();
}


void ChessBoard::paintEvent(QPaintEvent *event) {
    QPainter painter(this);
    int x = 0;     // X coordinate
    int y = 0;      // Y coordinate
    int w = 520;     // Width of the board
    int h = 520;     // Height of the board

    // Draw the image scaled to size at the specified coordinates
    painter.drawImage(QRect(x, y, 400, 400), image);

    for (int i = 0; i < 8; ++i){
        for (int j = 0; j < 8; ++j){
            switch (puzzleBoard[i][j]){
                case PAWN:
                    painter.drawImage(QRect(x  + 50*j, y  + 50*i, 50, 50), whitePawn);
                    break;
                case ROOK:
                    painter.drawImage(QRect(x  + 50*j, y  + 50*i, 50, 50), whiteRook);
                    break;
                case KNIGHT:
                    painter.drawImage(QRect(x  + 50*j, y  + 50*i, 50, 50), whiteKnight);
                    break;
                case BISHOP:
                    painter.drawImage(QRect(x  + 50*j, y  + 50*i, 50, 50), whiteBishop);
                    break;
                case QUEEN:
                    painter.drawImage(QRect(x  + 50*j, y  + 50*i, 50, 50), whiteQueen);
                    break;
                case KING:
                    painter.drawImage(QRect(x  + 50*j, y  + 50*i, 50, 50), whiteKing);
                    break;

                // Black
                case -PAWN:
                    painter.drawImage(QRect(x  + 50*j, y  + 50*i, 50, 50), blackPawn);
                    break;
                case -ROOK:
                    painter.drawImage(QRect(x  + 50*j, y  + 50*i, 50, 50), blackRook);
                    break;
                case -KNIGHT:
                    painter.drawImage(QRect(x  + 50*j, y  + 50*i, 50, 50), blackKnight);
                    break;
                case -BISHOP:
                    painter.drawImage(QRect(x  + 50*j, y  + 50*i, 50, 50), blackBishop);
                    break;
                case -QUEEN:
                    painter.drawImage(QRect(x  + 50*j, y  + 50*i, 50, 50), blackQueen);
                    break;
                case -KING:
                    painter.drawImage(QRect(x  + 50*j, y  + 50*i, 50, 50), blackKing);
                    break;
            }
        }
    }

    if (hasHintMove) {
        QColor overlay(255,255,0,100);
        painter.setBrush(overlay);
        painter.setPen(Qt::NoPen);
        // draw from‐square
        painter.drawRect(hintMoveFc*50, hintMoveFr*50, 50, 50);
        // draw to‐square
        painter.drawRect(hintMoveTc*50, hintMoveTr*50, 50, 50);
    }

    if (hasHint) {
        QColor overlay(255,255,0,100);
        painter.setBrush(overlay);
        painter.setPen(Qt::NoPen);
        // draw from‐square
        painter.drawRect(hintFc*50, hintFr*50, 50, 50);
    }

    if (confetti) {
        confetti->draw(painter);
    }


}

void ChessBoard::setBoardState(int pb[8][8]){
    for (int i = 0; i < 8; ++i)
        for (int j = 0; j < 8; ++j)
            puzzleBoard[i][j] = pb[i][j];
}

std::vector<std::vector<int>> ChessBoard::getBoardState() const {
    std::vector<std::vector<int>> out(8, std::vector<int>(8));
    for(int i=0; i<8; ++i)
        for(int j=0; j<8; ++j)
            out[i][j] = puzzleBoard[i][j];
    return out;
}

void ChessBoard::setBoardState(const std::vector<std::vector<int>>& pb) {
    // assume pb.size()==8 && pb[i].size()==8
    for(int i = 0; i < 8; ++i) {
        for(int j = 0; j < 8; ++j) {
            puzzleBoard[i][j] = pb[i][j];
        }
    }
    update();  // trigger repaint
}
