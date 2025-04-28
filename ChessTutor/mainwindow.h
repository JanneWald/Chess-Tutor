/*
 * mainwindow.h
 *
 * Defines the MainWindow class, which serves as the primary UI for both
 * standard chess games and puzzle mode. Manages board rendering, user input,
 * timers, ELO calculations, and confetti celebrations upon victories.
 *
 * @author  ESL Team
 * @date    2025-04-22
 */
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "chessboard.h"
#include <QMainWindow>
#include "confetticontroller.h"
#include "chess.h"
#include "chesspuzzle.h"
#include <vector>
#include <memory>
#include <QElapsedTimer>
#include <QTimer>
#include <QTimeEdit>
#include <QLabel>
#include <QPalette>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

/*
 * MainWindow
 *
 * Inherits QMainWindow to provide the main application window. Hosts
 * a ChessBoard widget, controls for hints, puzzles, and standard play,
 * and displays current ELO and timer.
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    /*
     * Constructor
     * @param parent Optional parent widget.
     */
    MainWindow(QWidget *parent = nullptr);

    /*
     * Destructor
     */
    ~MainWindow();

    /*
     * Pointer to the ChessBoard visualization widget.
     * Displays the current board state in both standard and puzzle modes.
     */
    ChessBoard *boardVisuals;

signals:
    /*
     * Emitted when a capture occurs, for confetti animation.
     * @param x World-coordinate X of the capture.
     * @param y World-coordinate Y of the capture.
     * @param count Number of confetti particles.
     */
    void captureAt(float x, float y, int count);

    /*
     * Emitted when a puzzle is completed, triggers full celebration.
     * @param count Number of confetti particles.
     */
    void puzzleComplete(int count);

private:
    /*
     * Auto-generated UI elements from mainwindow.ui.
     */
    Ui::MainWindow *ui;

    /*
     * True if a piece has been selected by the user for movement.
     */
    bool selected{false};

    /*
     * Stores the board coordinates of the selected piece.
     */
    Square selectedPiece{0, 0};

    /*
     * Active Chess instance for standard play. nullptr when in puzzle mode.
     */
    Chess* currentGame{nullptr};

    /*
     * Active ChessPuzzle instance for puzzle mode. nullptr when in standard play.
     */
    ChessPuzzle* currentPuzzle{nullptr};

    /*
     * Container for preloaded puzzles (if implemented), holds ownership.
     */
    std::vector<std::unique_ptr<ChessPuzzle>> puzzles;

    /*
     * Corresponding CSV lines for each puzzle in `puzzles`.
     */
    QVector<QString> puzzleCsvLines_;

    /*
     * Flag for light/dark theme toggle.
     */
    bool darkMode_{false};

    /*
     * Applies the currently selected UI theme (light or dark).
     */
    void applyTheme();

    /*
     * Timer used to measure puzzle solution time.
     */
    QElapsedTimer puzzleTimer;

    /*
     * Live update timer for the on-screen clock display.
     */
    QTimer *liveTimer = nullptr;

    /*
     * Current ELO rating of the player.
     */
    int currentElo{1200};

    /*
     * Base points awarded for fast puzzle solves.
     */
    static constexpr int kBasePoints = 100;

    /*
     * Rate of ELO decay per second in puzzle mode.
     */
    static constexpr double kDecayPerSecond = 1.0;

    /*
     * Adjusts the currentElo by the specified delta.
     * @param earned Points to add (or subtract if negative).
     */
    void adjustElo(int earned) { currentElo += earned; }

    /*
     * True once the user has used a hint in the current puzzle,
     * prevents ELO gain on hint-assisted solves.
     */
    bool hintUsed{false};

    /*
     * Calculates the ELO rating change based on opponent ELO and game result.
     * @param opponentElo The ELO rating of the opponent or puzzle.
     * @param win True if the user won, false otherwise.
     * @return ELO delta (positive for gain, negative for penalty).
     */
    int calculateEloChange(int opponentElo, bool win) const;

    /*
     * Updates the on-screen ELO display label.
     */
    void updateEloDisplay();

    /*
     * Loads puzzles into memory (not currently used).
     */
    void createPuzzles();

    /*
     * Utility: Reads lines from a CSV resource and returns one at random.
     * @param csvPath Resource path to the CSV file.
     * @return A random line from the file, or empty if none.
     */
    QString randomLineFromFile(const QString& csvPath) const;

    /*
     * Repeatedly samples lines until a valid puzzle line is found.
     * @param csvPath Resource path to the CSV file.
     * @return A valid puzzle line, or empty if none.
     */
    QString randomValidPuzzleLine(const QString& csvPath) const;

    /*
     * Validates the FEN and theme fields of a CSV puzzle line.
     * @param line A single comma-separated puzzle entry.
     * @return true if valid, false otherwise.
     */
    bool isValidPuzzleLine(const QString& line) const;

    /*
     * Index of the current puzzle in `puzzles` (not used).
     */
    int currentPuzzleIndex{0};

    /*
     * Initializes and displays a new random puzzle.
     */
    void makeNewPuzzle();

    /*
     * Assigns puzzle completion ELO based on time taken.
     */
    void assignElo();

    /*
     * Manages confetti controller for celebrations.
     */
    ConfettiController* m_confetti = nullptr;

public slots:
    /*
     * Handles user clicks on the board grid.
     * @param row Row index clicked (0-7).
     * @param col Column index clicked (0-7).
     */
    void onSquareSelected(int row, int col);

    /*
     * Highlights the from/to squares for a hint move.
     */
    void onHintMoveAvailable(int fromRow, int fromCol,
                             int toRow,   int toCol);

    /*
     * Highlights only the from-square for a simple hint.
     */
    void onHintAvailable(int fr, int fc);

    /*
     * Executes the next step when "Show Solution" is pressed.
     */
    void playSolutionStep();

private slots:
    /*
     * Loads puzzle mode (triggered by the Puzzle button).
     */
    void on_PuzzleButton_clicked();

    /*
     * Switches to standard board mode (triggered by the Board button).
     */
    void on_BoardButton_clicked();

    /*
     * Called when the current puzzle is solved for a full celebration.
     */
    void on_beat_puzzle();

    /*
     * Called when a normal chess game is won.
     */
    void on_game_won();

    /*
     * Triggered when the Hint-Move button is clicked.
     */
    void on_hintMoveButton_clicked();

    /*
     * Triggered when the Hint button is clicked.
     */
    void on_hintButton_clicked();

    /*
     * Refreshes the board display on update signals.
     */
    void on_update_Board();

    /*
     * Updates the UI label when the current player changes.
     */
    void on_set_player(Player player);

    /*
     * Triggered by ">>" on UI to reset the current puzzle.
     */
    void on_resetButton_clicked();

    /*
     * Triggered when the "Show Solution" button is clicked.
     */
    void on_solutionButton_clicked();
};

#endif // MAINWINDOW_H
