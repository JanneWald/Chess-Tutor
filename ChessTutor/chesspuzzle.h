/*
 * chesspuzzle.h
 *
 * Defines the ChessPuzzle class, which extends Chess to support
 * single-solution puzzles loaded from FEN+PGN CSV entries. It
 * provides hinting, peeking at the next move, and signaling when
 * puzzles are solved.
 *
 * @author  ESL Team
 * @date    2025-04-22
 */
#ifndef CHESSPUZZLE_H
#define CHESSPUZZLE_H

#include <QObject>
#include "chess.h"
#include <vector>
#include <string>

using std::vector;
using std::pair;
using std::string;

/**
 * ChessPuzzle
 *
 * Inherits Chess to represent a puzzle scenario with a pre-defined
 * move sequence. Loads initial position from FEN, applies opponent’s
 * first move, and then guides the user through the solution.
 */
class ChessPuzzle : public Chess {
    Q_OBJECT

private:
    /**
     * Full sequence of (fromSquare, toSquare) pairs,
     * including both opponent and player moves.
     */
    vector<pair<Square,Square>> solutionMoves;

    /**
     * Index of the next move to apply from solutionMoves.
     * Opponent moves first at index 0.
     */
    int currentStep = 0;

    /**
     * Elo rating assigned to this puzzle, used in scoring.
     */
    int puzzleElo = 0;

    /**
     * Split a delimited string into tokens.
     *
     * @param PGN Input string (e.g. CSV field)
     * @param delimiter Character to split on
     * @return Vector of substrings
     */
    vector<string> split(const string& PGN, char delimiter);

public:
    /**
     * Flag indicating whether a hint was used during this puzzle.
     */
    bool usedHint{false};

    /**
     * Construct from raw board state and solution moves.
     *
     * @param boardVec 8x8 integer matrix of piece codes
     * @param solMoves Sequence of solution move pairs
     */
    ChessPuzzle(const vector<vector<int>>& boardVec,
                const vector<pair<Square,Square>>& solMoves);

    /**
     * Construct from a single CSV line containing FEN and PGN moves.
     * Automatically parses FEN, loads board, and makes opponent’s first move.
     *
     * @param PGN CSV line from puzzle database
     */
    ChessPuzzle(const string& PGN);

    /**
     * Convert a FEN piece character to internal integer code.
     *
     * @param c Character in {KQRBNP kqrbnp}
     * @return Internal piece constant (positive white, negative black)
     */
    int toPiece(const char& c);

    /**
     * Load board layout and side-to-move from a FEN string.
     *
     * @param FEN FEN-encoded position field
     */
    void loadFEN(const string& FEN);

    /**
     * Parse space-separated PGN moves into solutionMoves.
     *
     * @param pgnMoves Moves string (e.g. "e2e4 e7e5 ...")
     */
    void loadMoves(const string& pgnMoves);

    /**
     * Override the solution move list and reset progress.
     *
     * @param solMoves New sequence of solution move pairs
     */
    void setSolutionMoves(const vector<pair<Square,Square>>& solMoves);

    /**
     * Attempt the user’s move guess. If it matches the next step,
     * apply it and return true.
     *
     * @param from Source square
     * @param to   Destination square
     * @return True if the guess was correct
     */
    bool makeGuess(Square from, Square to);

    /**
     * Execute the opponent’s next pre-defined move automatically.
     */
    void makeOpponentMove();

    /**
     * Check if all solution moves have been applied.
     *
     * @return True if puzzle is solved
     */
    bool isSolved() const;

    /**
     * Peek at the next move without advancing state.
     *
     * @return Pair (fromSquare, toSquare)
     */
    std::pair<Square,Square> peekNextMove() const;

    /**
     * Get the Elo rating assigned to this puzzle.
     *
     * @return Puzzle Elo
     */
    int getPuzzleElo() const { return puzzleElo; }

signals:
    /**
     * Emitted with both from- and to-square of the upcoming move.
     *
     * @param fromRow Row of source
     * @param fromCol Col of source
     * @param toRow   Row of destination
     * @param toCol   Col of destination
     */
    void hintMoveAvailable(int fromRow, int fromCol,
                           int toRow,   int toCol);

    /**
     * Emitted with only the from-square of the next move.
     *
     * @param fromRow Row of source
     * @param fromCol Col of source
     */
    void hintAvailable(int fromRow, int fromCol);

    /**
     * Emitted when the puzzle is fully solved.
     */
    void beatPuzzle();

public slots:
    /**
     * Request a full hint; emits hintMoveAvailable.
     */
    void requestHintMove();

    /**
     * Request a simple hint; emits hintAvailable.
     */
    void requestHint();

};

#endif // CHESSPUZZLE_H
