#include "mainwindow.h"
#include "qboxlayout.h"
#include "confetticontroller.h"
#include "ui_mainwindow.h"
#include "chess.h"
#include "chesspuzzle.h"
#include <iostream>
#include <QDir>
#include <QTime>
#include <QTimer>
#include <QFile>
#include <QTextStream>
#include <QCoreApplication>
#include <QRandomGenerator>
#include <QMessageBox>
#include <cmath>

using std::cout;
using std::endl;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{

    ui->setupUi(this);

    ui->timeEdit->setDisplayFormat("mm:ss");
    ui->timeEdit->setReadOnly(true);

    liveTimer = new QTimer(this);
    connect(liveTimer, &QTimer::timeout, this, [this]() {
        qint64 ms    = puzzleTimer.elapsed();
        int    secs  = int(ms / 1000);
        int    mins  = secs / 60;
        secs %= 60;
        int    milli = int(ms % 1000);
        ui->timeEdit->setTime(QTime{0, mins, secs, milli});
    });

    // Setup Elo label in status bar
    ui->eloLabel->setText(QString("Elo: %1").arg(currentElo));

    boardVisuals = new ChessBoard(ui->boardFrame);
    m_confetti = new ConfettiController(boardVisuals, this);
    boardVisuals->setConfettiController(m_confetti);
    Chess blank{};
    boardVisuals->setBoardState(blank.getBoardVector());

    connect(boardVisuals, &ChessBoard::squareSelected, this, &MainWindow::onSquareSelected);
    connect(ui->hintMoveButton, &QPushButton::clicked, this,&MainWindow::on_hintMoveButton_clicked);
    connect(ui->hintButton, &QPushButton::clicked, this,&MainWindow::on_hintButton_clicked);
    connect(ui->solutionButton, &QPushButton::clicked, this, &MainWindow::on_solutionButton_clicked);

    // wire the Next Puzzle button
    connect(ui->nextPuzzleButton, &QPushButton::clicked,
            this, &MainWindow::makeNewPuzzle);

    // Asset loading setup
    QDir dir(QApplication::applicationDirPath());
    dir.cdUp();
    dir.cdUp();
    dir.cdUp();

    // Loads title icon
    QString titlePath = dir.filePath(":/Assets/Assets/Title.png");
    QImage title{titlePath};
    QPixmap titleMap = QPixmap::fromImage(title);
    ui->Title->setPixmap(titleMap);
    ui->Title->show();

    // Loads standard game icon
    QSize iconSize{140, 140};
    QString standardPath = dir.filePath(":/Assets/Assets/Standard.png");
    QIcon standardIcon{standardPath};
    ui->BoardButton->setText("");
    ui->BoardButton->setIcon(standardIcon);
    ui->BoardButton->setIconSize(iconSize);

    // Loads Puzzle Icon
    QString puzzlePath = dir.filePath(":/Assets/Assets/Puzzle.png");
    QIcon puzzleIcon{puzzlePath};
    ui->PuzzleButton->setText("");
    ui->PuzzleButton->setIcon(puzzleIcon);
    ui->PuzzleButton->setIconSize(iconSize);

}

// Check FEN castling field is exactly "-" and themes lack "enPassant"
bool MainWindow::isValidPuzzleLine(const QString& line) const {
    auto parts = line.split(',', Qt::KeepEmptyParts);
    if (parts.size() < 8)
        return false;

    // parts[1] is the FEN; split on spaces and ensure castling field (index 2) == "-"
    auto fenFields = parts[1].split(' ', Qt::SkipEmptyParts);
    if (fenFields.size() < 3 || fenFields[2] != "-")
        return false;

    // parts[7] is themes; reject if "enPassant" appears
    if (parts[7].contains("enPassant", Qt::CaseInsensitive))
        return false;

    return true;
}

// Read all non‑empty lines, pick one at uniform random
QString MainWindow::randomLineFromFile(const QString& csvPath) const {
    QFile f(csvPath);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
        return {};

    QStringList lines;
    QTextStream in(&f);
    while (!in.atEnd()) {
        QString ln = in.readLine().trimmed();
        if (!ln.isEmpty()) lines.append(ln);
    }
    if (lines.isEmpty())
        return {};

    int idx = QRandomGenerator::global()->bounded(lines.size());
    return lines.at(idx);
}

// Loop until we find a valid one (or give up after MaxTries)
QString MainWindow::randomValidPuzzleLine(const QString& csvPath) const {
    constexpr int MaxTries = 500;
    for (int i = 0; i < MaxTries; ++i) {
        QString cand = randomLineFromFile(csvPath);
        if (cand.isEmpty()) break;
        if (isValidPuzzleLine(cand))
            return cand;
    }
    return {};
}

void MainWindow::onHintMoveAvailable(int fr, int fc, int tr, int tc) {
    boardVisuals->setHintSquares(fr, fc, tr, tc);
}

void MainWindow::onHintAvailable(int fr, int fc) {
    boardVisuals->setHintSquares(fr, fc, fr, fc);
}

void MainWindow::on_hintMoveButton_clicked() {
    hintUsed = true;
    if (!currentPuzzle) {
        // no puzzle loaded yet
        return;
    }
    // ask the puzzle to emit hintMoveAvailable(...)
    currentPuzzle->requestHintMove();
}

void MainWindow::on_hintButton_clicked() {
    hintUsed = true;
    if (!currentPuzzle) {
        // no puzzle loaded yet
        return;
    }
    // ask the puzzle to emit hintAvailable(...)
    currentPuzzle->requestHint();
}

void MainWindow::on_resetButton_clicked() {
    // rebuild & reload the same puzzle so state, timer, etc. all go back to start
    // We should probably save the current PGN so that we can reconstruct currentPuzzle from past pgn!
}

void MainWindow::on_solutionButton_clicked() {
    if (!currentPuzzle) return;

    // disable user controls
    ui->hintMoveButton->setEnabled(false);
    ui->hintButton->setEnabled(false);
    ui->solutionButton->setEnabled(false);
    ui->BoardButton->setEnabled(false);
    ui->PuzzleButton->setEnabled(false);
    boardVisuals->clearHintMove();
    boardVisuals->clearHint();

    // start the recursive stepping
    playSolutionStep();
}

void MainWindow::playSolutionStep() {
    // done?
    if (!currentPuzzle || currentPuzzle->isSolved()) {
        QTimer::singleShot(2500, this, [this]() {
            ui->solutionButton->setEnabled(true);
            ui->BoardButton->setEnabled(true);
            ui->PuzzleButton->setEnabled(true);
        });

        return;
    }

    // peek at the next move
    auto [fr, fc] = currentPuzzle->peekNextMove().first;
    auto [tr, tc] = currentPuzzle->peekNextMove().second;

    // highlight it
    boardVisuals->setHintSquares(fr, fc, tr, tc);

    // after 1s, actually perform the move
    QTimer::singleShot(1000, this, [this, fr, fc, tr, tc]() {
        // execute that move
        currentPuzzle->makeGuess({fr,fc}, {tr,tc});
        boardVisuals->setBoardState(currentPuzzle->getBoardVector());
        boardVisuals->clearHintMove();
        boardVisuals->clearHint();

        // short pause, then step again
        QTimer::singleShot(500, this, &MainWindow::playSolutionStep);
    });
}


void MainWindow::onSquareSelected(int row, int col){
    Square square{row, col};

    if(!currentGame && !currentPuzzle)
        return;

    // toggle select/deselect
    selected = !selected;
    if(selected){
        selectedPiece = square;
        cout << "Selected: " << square << endl;
        return;
    }
    cout << "Moving to: " << square << endl;

    // Normal chess mode
    if(currentGame){
        currentGame->movePiece(selectedPiece, square);
        boardVisuals->setBoardState(currentGame->getBoardVector());
        boardVisuals->update();
        return;
    }

    // Puzzle mode
    if(currentPuzzle){
        boardVisuals->clearHintMove();
        boardVisuals->clearHint();
        ui->hintButton->setEnabled(false);
        ui->hintMoveButton->setEnabled(false);
        auto oldBoard = boardVisuals->getBoardState();

        cout << "making guess" << endl;
        bool correct = currentPuzzle->makeGuess(selectedPiece, square);

        if(correct){
            ui->statusbar->showMessage("Correct! Next move…", 1500);
            QTimer::singleShot(1000, this, [this]() {
                ui->hintMoveButton->setEnabled(true),
                    ui->hintButton->setEnabled(true);
            });

        }
        else {
            ui->statusbar->showMessage("Wrong try again!", 1500);

            selected = false;
            QTimer::singleShot(1000, this, [this]() {
                ui->hintMoveButton->setEnabled(true);
                ui->hintButton->setEnabled(true);
            });
            return;
        }
    }
}

void MainWindow::makeNewPuzzle(){
    // Chess logic
    selected = false;
    currentPuzzle = nullptr; // Clean it
    hintUsed = false;
    QString line = randomValidPuzzleLine(":/Data/lichess_db_puzzle_sample_50.csv");
    currentPuzzle = new ChessPuzzle(line.toStdString());
    Player currentPlayer = currentPuzzle->currentPlayer;

    // Signal/slot connections
    connect(currentPuzzle, &ChessPuzzle::hintMoveAvailable, this, &MainWindow::onHintMoveAvailable);
    connect(currentPuzzle, &ChessPuzzle::hintAvailable, this, &MainWindow::onHintAvailable);
    connect(currentPuzzle, &Chess::update_board, this, &MainWindow::on_update_Board);
    connect(currentPuzzle, &ChessPuzzle::beatPuzzle, this, &MainWindow::on_beat_puzzle);
    connect(currentPuzzle, &Chess::set_player, this, &MainWindow::on_set_player);
    connect(currentPuzzle, &Chess::capture_at, m_confetti, &ConfettiController::onSpawnAt);
    connect(currentPuzzle, &Chess::won_game, this, &MainWindow::on_game_won);

    connect(ui->hintMoveButton, &QPushButton::clicked, this, &MainWindow::on_hintMoveButton_clicked);
    connect(currentPuzzle, &ChessPuzzle::hintMoveAvailable, this, &MainWindow::onHintMoveAvailable);
    connect(ui->hintButton, &QPushButton::clicked, this, &MainWindow::on_hintButton_clicked);
    connect(currentPuzzle, &ChessPuzzle::hintAvailable, this, &MainWindow::onHintAvailable);
    on_set_player(currentPlayer);

    // Visuals
    boardVisuals->setBoardState(currentPuzzle->getBoardVector());
    boardVisuals->update();
    boardVisuals->clearHintMove();
    boardVisuals->clearHint();
    ui->hintMoveButton->setEnabled(true);
    ui->hintButton->setEnabled(true);

    // Timer
    puzzleTimer.restart();
    liveTimer->start(50);
    ui->nextPuzzleButton->setEnabled(false);
    statusBar()->showMessage("Loaded random puzzle!, Solve me!", 1500);
}

void MainWindow::assignElo(){
    qint64 ms   = puzzleTimer.elapsed();
    double secs = ms / 1000.0;
    int earned  = 100 - int(secs);          // base 100 minus 1pt/sec
    earned      = std::max(0, earned);
    if(currentPuzzle->usedHint){
        earned = calculateEloChange(currentPuzzle->getPuzzleElo(), false);
    }
    else{
        earned = calculateEloChange(currentPuzzle->getPuzzleElo(), true);
    }
    cout << earned << endl;
    currentElo += earned;
    updateEloDisplay();

    ui->statusbar->showMessage(
        QString("Solved in %1s  •  +%2 Elo  (total %3)")
            .arg(secs,0,'f',1)
            .arg(earned)
            .arg(currentElo),
        3000
        );

    ui->nextPuzzleButton->setEnabled(true);
}

void MainWindow::on_PuzzleButton_clicked() {
    if (currentGame) {
        delete currentGame;
        currentGame = nullptr;
    }
    currentGame = nullptr;
    makeNewPuzzle();

    statusBar()->showMessage("Puzzle mode", 1000);

    ui->hintMoveButton->setEnabled(true);
    ui->hintButton->setEnabled(true);
    boardVisuals->clearHintMove();
    boardVisuals->clearHint();
}

void MainWindow::on_BoardButton_clicked() {
    // Switch into “standard board” mode
    puzzleTimer.restart();
    liveTimer->start(50);
    currentGame = new Chess;
    currentGame->loadDefaultBoard();
    connect(currentGame, &Chess::capture_at, m_confetti, &ConfettiController::onSpawnAt);
    connect(currentGame, &Chess::won_game, this, &MainWindow::on_game_won);
    connect(currentGame, &Chess::set_player, this, &MainWindow::on_set_player);

    // Clear any puzzle state
    currentPuzzle = nullptr;

    // Paint the standard board
    boardVisuals->setBoardState(currentGame->getBoardVector());
    statusBar()->showMessage("Standard board mode", 1000);
    ui->hintMoveButton->setEnabled(false);
    ui->hintButton->setEnabled(false);
    boardVisuals->clearHintMove();
    boardVisuals->clearHint();
}

void MainWindow:: on_update_Board(){
    cout << "updating from signal" << endl;

    if(currentPuzzle){
        boardVisuals->setBoardState(currentPuzzle->getBoardVector());
    }
    boardVisuals->update();
}

void MainWindow::on_beat_puzzle(){
    cout << "beat puzzle from obj, making new puzzle" << endl;

    assignElo();
    m_confetti->spawn(300);

    QTimer::singleShot(3000, this, &MainWindow::makeNewPuzzle);
}

void MainWindow::on_set_player(Player player){
    cout << "Plare has been set to ";
    if(player == WHITE){
        cout << "WHITE";
        ui->player->setText("White Move");
    }
    else{
        cout << "BLACK";
        ui->player->setText("Black Move");
    }
    cout << endl;
}

int MainWindow::calculateEloChange(int opponentElo, bool win) const {
    static constexpr int K = 32;
    double expected = 1.0 / (1.0 + std::pow(10.0, (opponentElo - currentElo) / 400.0));
    double score    = win ? 1.0 : 0.0;
    return int(std::round(K * (score - expected)));
}

void MainWindow::updateEloDisplay() {
    ui->eloLabel->setText(QString("Elo: %1").arg(currentElo));
}

void MainWindow::on_game_won() {
    if(currentGame){
        m_confetti->spawn(300);
        QTimer::singleShot(3000, this, &MainWindow::on_BoardButton_clicked);
    }
}

MainWindow::~MainWindow()
{
    delete ui;
    if(currentPuzzle) delete currentPuzzle;
    if(currentGame) delete currentGame;
}


