/*
 * chessboard.h
 *
 * Defines the ChessBoard widget, which renders the current chess position,
 * handles user clicks to select squares, and displays hint highlights.
 * Integrates with ConfettiController for capture animations.
 *
 * @author  ESL Team
 * @date    2025-04-22
 */
#ifndef CHESSBOARD_H
#define CHESSBOARD_H

#include <QWidget>
#include <QImage>
#include <QMouseEvent>
#include <vector>
#include <QPainter>

class ConfettiController;

/**
 * ChessBoard
 *
 * Inherits QWidget to draw an 8×8 chess grid with piece images, hint
 * overlays, and confetti effects for captures. Emits squareSelected
 * when a user clicks a board square.
 */
class ChessBoard : public QWidget {
    Q_OBJECT

public:
    /**
     * Constructor
     *
     * @param parent Optional parent widget for ownership.
     */
    explicit ChessBoard(QWidget *parent = nullptr);

    /**
     * Paint event
     *
     * Called by Qt to repaint the board, pieces, hints, and confetti.
     *
     * @param event Paint event details (ignored).
     */
    void paintEvent(QPaintEvent *event) override;

    /**
     * setBoardState
     *
     * Update the internal board array from a raw 8×8 int matrix.
     *
     * @param pb 2D array of piece codes (positive white, negative black).
     */
    void setBoardState(int pb[8][8]);

    /**
     * setBoardState
     *
     * Update the internal board array from a vector-of-vectors.
     *
     * @param pb 8×8 vector of piece codes.
     */
    void setBoardState(const std::vector<std::vector<int>>& pb);

    /**
     * setConfettiController
     *
     * Attach a ConfettiController for capture/win animations.
     *
     * @param c Pointer to the controller.
     */
    void setConfettiController(ConfettiController* c) { confetti = c; }

    /**
     * getBoardState
     *
     * Retrieve the current board as an 8×8 vector of ints.
     *
     * @return 8×8 vector of piece codes.
     */
    std::vector<std::vector<int>> getBoardState() const;

    /**
     * setHintSquares
     *
     * Highlight both a source and destination square for a hint move.
     *
     * @param fromRow Row of source square.
     * @param fromCol Column of source square.
     * @param toRow   Row of destination square.
     * @param toCol   Column of destination square.
     */
    void setHintSquares(int fromRow, int fromCol,
                        int toRow,   int toCol);

    /**
     * clearHintMove
     *
     * Remove any currently drawn hint-move overlay.
     */
    void clearHintMove();

    /**
     * clearHint
     *
     * Remove any simple hint overlay (source only).
     */
    void clearHint();

private:
    bool   hasHintMove = false; ///< True if a two-square hint is shown
    bool   hasHint     = false; ///< True if a single-square hint is shown
    int    hintMoveFr;          ///< From-row for hint-move overlay
    int    hintMoveFc;          ///< From-col for hint-move overlay
    int    hintMoveTr;          ///< To-row for hint-move overlay
    int    hintMoveTc;          ///< To-col for hint-move overlay
    int    hintFr;              ///< Row for simple hint overlay
    int    hintFc;              ///< Col for simple hint overlay

    QImage image;         ///< Background board image
    QImage blackPawn;     ///< Black pawn image
    QImage blackRook;     ///< Black rook image
    QImage blackBishop;   ///< Black bishop image
    QImage blackKnight;   ///< Black knight image
    QImage blackQueen;    ///< Black queen image
    QImage blackKing;     ///< Black king image
    QImage whitePawn;     ///< White pawn image
    QImage whiteRook;     ///< White rook image
    QImage whiteBishop;   ///< White bishop image
    QImage whiteKnight;   ///< White knight image
    QImage whiteQueen;    ///< White queen image
    QImage whiteKing;     ///< White king image

    int puzzleBoard[8][8]; ///< Internal board state array

    /**
     * mousePressEvent
     *
     * Convert a click position into board coordinates and emit
     * squareSelected.
     *
     * @param event Mouse event with click position.
     */
    void mousePressEvent(QMouseEvent *event) override;

    bool selected{false}; ///< True if a piece is currently selected
    ConfettiController* confetti = nullptr; ///< Confetti animation controller

signals:
    /**
     * squareSelected
     *
     * Emitted when the user clicks on a valid board square.
     *
     * @param row Row index (0-7)
     * @param col Column index (0-7)
     */
    void squareSelected(int row, int col);
};

#endif // CHESSBOARD_H
