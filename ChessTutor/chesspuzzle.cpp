#include "chesspuzzle.h"
#include <iostream>
#include <sstream>
#include <QTimer>
#include <QSoundEffect>

using std::endl;
using std::cout;
ChessPuzzle::ChessPuzzle(const std::vector<std::vector<int>>& boardVec,
                         const std::vector<std::pair<Square,Square>>& solMoves)
    : solutionMoves(solMoves), currentStep(0)
{
    int raw[8][8];
    for (int r = 0; r < 8; ++r)
        for (int c = 0; c < 8; ++c)
            raw[r][c] = boardVec[r][c];
    loadBoard(raw);
}

vector<string> ChessPuzzle::split(const string& PGN, char delimiter){
    vector<string> tokens;
    std::stringstream ss(PGN);
    string item;
    while(std::getline(ss, item, delimiter)){
        tokens.push_back(item);
    }
    return tokens;
}

ChessPuzzle::ChessPuzzle(const std::string& PGN){
    debugging = true;

    if (debugging) cout << "PGN for this puzzle is: " << PGN << endl;

    auto parts = split(PGN, ',');
    if (parts.size() < 9){
        if (debugging) cout << "CSV cannot be shorter than 9 segments" << endl;
        return;
    }
    string fen = parts[1];
    string moves = parts[2];
    // Elo rating is in column 4 if we want to use
    string themes = parts[7];

    puzzleElo = std::stoi(parts[3]);
    loadFEN(fen);
    if(debugging){
        cout << "New board from FEN is: " << endl;
        printBoard();
    }
    loadMoves(moves);
    // Make first move
    makeOpponentMove();
    emit set_player(currentPlayer);
}

int ChessPuzzle::toPiece(const char& c){
    switch(c){
    // Black pieces
    case 'k':
        return -KING;
    case 'q':
        return -QUEEN;
    case 'n':
        return -KNIGHT;
    case 'b':
        return -BISHOP;
    case 'r':
        return -ROOK;
    case 'p':
        return -PAWN;
    // White pieces
    case 'K':
        return KING;
    case 'Q':
        return QUEEN;
    case 'N':
        return KNIGHT;
    case 'B':
        return BISHOP;
    case 'R':
        return ROOK;
    case 'P':
        return PAWN;
    default:
        return 0;
    }
}

void ChessPuzzle::loadFEN(const string& FEN){
    auto parts = split(FEN, ' ');
    string startingPositions = parts[0];
    string startingPlayer = parts[1];
    // Our board is upsidedown so we can read from left to righ and just ++
    int row = 0;
    int col = 0;
    for(char c : startingPositions){
        if(c == '/'){
            row++;
            col = 0;
        }
        else if(std::isdigit(c)){
            col += c - '0';
        }
        else{
            board[row][col] = toPiece(c);
            col++;
        }
    }
    if (startingPlayer == "b"){
        currentPlayer = BLACK;
    }
    else{
        currentPlayer = WHITE;
    }
    if (debugging) cout << "First move goes to" << currentPlayer << endl;
    return;
}

void ChessPuzzle::requestHintMove() {
    usedHint = true;
    if (currentStep < solutionMoves.size()) {
        const auto& mv = solutionMoves[currentStep];
        // mv.first and mv.second are Squares
        emit hintMoveAvailable(
            mv.first.row,  mv.first.col,
            mv.second.row, mv.second.col
            );
    }
}

void ChessPuzzle::requestHint() {
    usedHint = true;
    if (currentStep < solutionMoves.size()) {
        const auto& mv = solutionMoves[currentStep];
        // emit only the “from” square
            emit hintAvailable(
                mv.first.row,
                mv.first.col
                );
    }
}

void ChessPuzzle::loadMoves(const string& pgnMoves){
    auto moves = split(pgnMoves, ' ');
    for(string move : moves){
        Square piece{move.substr(0, 2)};
        Square location{move.substr(2, 2)};
        solutionMoves.push_back({piece, location});
    }
}

void ChessPuzzle::makeOpponentMove(){
    const std::pair<Square, Square>& currentMove = solutionMoves[currentStep];
    Square piece{currentMove.first};
    Square move{currentMove.second};

    if (debugging) cout << "Opponent will move " << piece << " to " << move << endl;

    movePieceUnconditionally(piece, move);
    currentStep++;
}

void ChessPuzzle::setSolutionMoves(const std::vector<std::pair<Square,Square>>& solMoves) {
    solutionMoves = solMoves;
    currentStep   = 0;
}

bool ChessPuzzle::makeGuess(Square from, Square to) {
    if (currentStep >= solutionMoves.size()) {
        if (debugging) std::cout << "Puzzle already solved.\n";
        return false;
    }

    auto [expFrom, expTo] = solutionMoves[currentStep];
    if (from == expFrom && to == expTo) {
        if (debugging) std::cout << "Correct move " << currentStep/2 << " of " << solutionMoves.size() / 2 << std::endl;
        movePieceUnconditionally(from, to);
        currentStep++;

        cout << "current step: " << currentStep << endl;
        if(currentStep == solutionMoves.size()){
            if (debugging) cout << "Beat the puzzle" << endl;
            // play sound effect
            emit beatPuzzle();
            QSoundEffect *effect = new QSoundEffect;
            effect->setSource(QUrl("qrc:/Assets/Assets/Confetti.wav"));
            effect->setVolume(1);
            effect->play();
            return true;
        }
        else{
            if (debugging) cout << "Correct move, opponent will now move" << endl;
            QTimer::singleShot(1000, this, &ChessPuzzle::makeOpponentMove);
            return true;
        }
    }
    else {
        if (debugging) std::cout << "Wrong move. Expected: " << expFrom << " -> " << expTo << std::endl;
        return false;
    }
}

bool ChessPuzzle::isSolved() const {
    return currentStep >= solutionMoves.size();
}

std::pair<Square,Square> ChessPuzzle::peekNextMove() const {
    return solutionMoves[currentStep];
}

