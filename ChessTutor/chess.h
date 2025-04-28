/*
 * chess.h
 *
 * Defines the core Chess class and supporting types for move validation,
 * board state management, and game signals such as captures and wins.
 * Also provides Square for coordinate conversion.
 *
 * @author  ESL Team
 * @date    2025-04-22
 */
#ifndef CHESS_H
#define CHESS_H

#include <string>
#include <vector>
#include <QObject>

/**
 * Square
 *
 * Represents a position on the chessboard. Internally uses 0-7 for row/col
 * corresponding to ranks 1-8 and files a-h.
 */
struct Square{
    int row; ///< Row index (0-7), corresponds to ranks 1-8
    int col; ///< Column index (0-7), corresponds to files a-h

    /**
     * Construct from algebraic notation (e.g. "e4").
     * @param square Two-character string: file then rank.
     */
    Square(std::string square);

    /**
     * Construct from row and column indices.
     * @param row Row index (0-7)
     * @param col Column index (0-7)
     */
    Square(int row, int col);

    /**
     * Equality operator.
     * @return True if both row and column match.
     */
    bool operator==(const Square& other) const;

    /**
     * Inequality operator.
     * @return True if either row or column differ.
     */
    bool operator!=(const Square& other) const;

    /**
     * Stream output for debugging, prints in [file][rank] form.
     */
    friend std::ostream& operator<<(std::ostream& os, const Square& obj);
};

/**
 * Piece codes for board representation.
 */
enum Piece{
    PAWN = 1,  ///< Pawn
    ROOK,      ///< Rook
    KNIGHT,    ///< Knight
    BISHOP,    ///< Bishop
    QUEEN,     ///< Queen
    KING       ///< King
};

/**
 * Player color multiplier.
 */
enum Player{
    WHITE = 1, ///< White (positive piece codes)
    BLACK = -1 ///< Black (negative piece codes)
};

/**
 * Chess
 *
 * Core chess logic: board state, move validation, and event signaling.
 * Inherits QObject to emit signals for captures, board updates, and game results.
 */
class Chess : public QObject
{
    Q_OBJECT

public:
    /**
     * Constructor
     * @param parent Optional QObject parent for signal propagation.
     */
    explicit Chess(QObject* parent = nullptr);

    bool debugging{false}; ///< When true, prints board after each operation.

    /**
     * clearBoard
     *
     * Removes all pieces from the board (sets to 0).
     */
    void clearBoard();

    /**
     * loadDefaultBoard
     *
     * Sets up the initial FIDE starting position on the board.
     */
    void loadDefaultBoard();

    /**
     * loadBoard
     *
     * Replace the current board with a custom 8×8 integer array.
     * @param board 2D array of piece codes.
     */
    void loadBoard(int board[8][8]);

    /**
     * addPiece
     *
     * Places a piece on the board at the given square without validation.
     * @param player Player color multiplier (WHITE or BLACK)
     * @param piece  Piece type code
     * @param square Target position
     */
    void addPiece(const Player player, const Piece piece, const Square square);

    /**
     * movePiece
     *
     * Attempts to move a piece according to chess rules; validates turn,
     * destination, and move legality before calling movePieceUnconditionally.
     * @param oldSquare Starting square
     * @param newSquare Destination square
     */
    void movePiece(const Square oldSquare, const Square newSquare);

    /**
     * movePieceUnconditionally
     *
     * Moves a piece regardless of legality checks; used internally once
     * a move is validated.
     * @param oldSquare Starting square
     * @param newSquare Destination square
     */
    void movePieceUnconditionally(const Square oldSquare, const Square newSquare);

    /**
     * printBoard
     *
     * Outputs the board array to stdout for debugging.
     */
    void printBoard();

    /**
     * getPiece
     *
     * @return Integer code of the piece at the given square (0 if empty).
     */
    int getPiece(const Square square);

    /**
     * getBoardVector
     *
     * @return 8×8 vector of piece codes representing the board state.
     */
    std::vector<std::vector<int>> getBoardVector() const;

    Player currentPlayer = WHITE; ///< Whose turn it is (WHITE starts)

protected:
    int board[8][8]{}; ///< Internal board matrix of piece codes

    /**
     * isLegalKingMove
     *
     * Validates a single-square move for the king.
     */
    bool isLegalKingMove(const Square oldSquare, const Square newSquare);

    /**
     * isLegalQueenMove
     *
     * Validates rook- or bishop-like sliding moves for the queen.
     */
    bool isLegalQueenMove(const Square oldSquare, const Square newSquare);

    /**
     * isLegalBishopMove
     *
     * Validates diagonal sliding moves for the bishop.
     */
    bool isLegalBishopMove(const Square oldSquare, const Square newSquare);

    /**
     * isLegalKnightMove
     *
     * Validates L-shaped jumps for the knight.
     */
    bool isLegalKnightMove(const Square oldSquare, const Square newSquare);

    /**
     * isLegalRookMove
     *
     * Validates horizontal and vertical sliding moves for the rook.
     */
    bool isLegalRookMove(const Square oldSquare, const Square newSquare);

    /**
     * isLegalPawnMove
     *
     * Validates forward moves, captures, and two-square jumps for pawns.
     */
    bool isLegalPawnMove(const Square oldSquare, const Square newSquare);

    /**
     * isPieceInterrupting
     *
     * Checks for blocking pieces along a sliding path.
     * @param rowOffset Unit row step (-1,0,1)
     * @param colOffset Unit column step (-1,0,1)
     * @param oldSquare Start of path
     * @param newSquare End of path
     */
    bool isPieceInterrupting(int rowOffset, int colOffset,
                             const Square oldSquare,
                             const Square newSquare);

    /**
     * switchPlayer
     *
     * Toggles currentPlayer between WHITE and BLACK and emits set_player.
     */
    void switchPlayer();

public slots:
    /**
     * on_spawnAt
     *
     * Alias slot for capture events to spawn confetti (via capture_at).
     */
    // (No change needed here)

signals:
    /**
     * capture_at
     *
     * Emitted when a capture occurs; x,y in world coords, count for particles.
     */
    void capture_at(float x, float y, int count);

    /**
     * update_board
     *
     * Request the UI to refresh the board display.
     */
    void update_board();

    /**
     * set_player
     *
     * Notify the UI of a change in currentPlayer.
     */
    void set_player(Player player);

    /**
     * won_game
     *
     * Emitted when a king is captured, signaling game end.
     */
    void won_game();
};

#endif // CHESS_H
