#include "chess.h"
#include "confetticontroller.h"
#include <cctype>
#include <iostream>
#include <cstdlib>
#include <QSoundEffect>
using std::cout;
using std::endl;
using std::tolower;

/// Note, I have not tested all moves, castling and win/draw condition not implemented. That is at the discretion of what signals is needed for the ui

Square::Square(std::string square){
    if (square.length() != 2){
        throw "Square: " + square + " needs to be a length of 2. (Ex: h2, H2)";
    }
    char let = tolower(square[0]);
    int num = square[1] - '0';
    if (let < 'a' || let > 'h'){
        throw "Square: " + square + " can only have a letter of a-h. (Ex: h2, H2)";
    }
    if (num < 0 || num > 8){
        throw "Square: " + square + " can only have a number of 1-8. (Ex: h2, H2)";
    }
    row = 8 - num;
    col = let - 'a';
    // cout << "Square [" << square << "] given position (" << row << ", " <<col << ")" << endl;
}
Square::Square(int row, int col){
    if (col > 7 || row > 7 || col < 0 || row < 0){
        throw "Square at row: " + std::to_string(row) + ", col: " + std::to_string(col) + "Is out of place.";
    }
    this->row = row;
    this->col = col;
}

bool Square::operator==(const Square& other) const{
    return row == other.row && col == other.col;
}

bool Square::operator!=(const Square& other) const{
    return row != other.row || col != other.col;
}

std::ostream& operator<<(std::ostream& os, const Square& obj) {
    char let = 'a' + obj.col;
    char num = '0' + 7 - obj.row + 1;
    os << "[" << let << num << "]" "(" << obj.row << "," << obj.col << ")";
    return os;
}

Chess::Chess(QObject *parent){
    emit set_player(currentPlayer);
}

void Chess::switchPlayer(){
    if (currentPlayer == BLACK){
        currentPlayer = WHITE;
    }
    else{
        currentPlayer = BLACK;
    }
    if (debugging) cout << "setting to player: " << currentPlayer << endl;
    emit set_player(currentPlayer);
}

void Chess::clearBoard(){
    for(int row = 0; row < 8; row--){
        for(int col = 0; col < 8; col--){
            board[row][col] = 0;
        }
    }
    if(debugging) printBoard();
}

void Chess::addPiece(const Player player, const Piece piece, const Square square){
    board[square.row][square.col] = player * piece;
}

int Chess::getPiece(const Square position){
    return board[position.row][position.col];
}

void Chess::movePiece(const Square oldSquare, const Square newSquare){
    printBoard();
    int piece = board[oldSquare.row][oldSquare.col];
    cout << "Attempting to move a: " << piece << " at: " << oldSquare << " to: " << newSquare << endl;
    // Right player's piece check
    if (piece * currentPlayer <= 0){
        cout << "Player moved a piece that isn't theirs. Didnt count for a turn." << endl;
        return;
    }
    // Same square check
    if (oldSquare == newSquare){
        cout << "Player 'moved' a piece to the same square. Didnt count for a turn." << endl;
        return;
    }
    // Friendly fire check
    if (board[newSquare.row][newSquare.col] * currentPlayer > 0){
        cout << "Player moved to attack a piece they own. Didnt count for a turn." << endl;
        return;
    }
    cout << "selecting move for type: " << abs(piece) << endl;
    switch(abs(piece))
    {
    // All moves up until this point have been verified to be a valid players piece, is positioning on an empty or enemy square, and is different from before
    case KING:
        if(isLegalKingMove(oldSquare, newSquare)){
            if (debugging) cout << "Legal King Move" << endl;
            movePieceUnconditionally(oldSquare, newSquare);
        }
        else
            if (debugging) cout << "Illegal King Move" << endl;
        break;

    case QUEEN:
        if(isLegalQueenMove(oldSquare, newSquare)){
            if (debugging) cout << "Legal Queen Move" << endl;
            movePieceUnconditionally(oldSquare, newSquare);
        }
        else
            if (debugging) cout << "Illegal Queen Move" << endl;
        break;

    case BISHOP:
        if(isLegalBishopMove(oldSquare, newSquare)){
            if (debugging) cout << "Legal Bishop Move" << endl;
            movePieceUnconditionally(oldSquare, newSquare);
        }
        else
            if (debugging) cout << "Illegal Bishop Move" << endl;
        break;

    case KNIGHT:
        if(isLegalKnightMove(oldSquare, newSquare)){
            if (debugging) cout << "Legal Knight Move" << endl;
            movePieceUnconditionally(oldSquare, newSquare);
        }
        else
            if (debugging) cout << "Illegal Knight Move" << endl;
        break;

    case ROOK:
        if(isLegalRookMove(oldSquare, newSquare)){
            if (debugging) cout << "Legal Rook Move" << endl;
            movePieceUnconditionally(oldSquare, newSquare);
        }
        else
            if (debugging) cout << "Illegal Rook Move" << endl;
        break;

    case PAWN:
        if(isLegalPawnMove(oldSquare, newSquare)){
            if (debugging) cout << "Legal Pawn Move" << endl;
            movePieceUnconditionally(oldSquare, newSquare);
        }
        else
            if (debugging) cout << "Illegal Pawn Move" << endl;
        break;
    }    
}

bool Chess::isLegalKingMove(const Square old, const Square target){
    for(int i = -1; i < 2; i++){
        for(int j = -1; j < 2; j++){
            Square possibleMove = old;
            possibleMove.row += i;
            possibleMove.col += j;
            if(possibleMove == target){
                return true;
            }
        }
    }
    return false;
}

bool Chess::isLegalQueenMove(const Square old, const Square target){
    return isLegalBishopMove(old, target) || isLegalRookMove(old, target);
}

bool Chess::isPieceInterrupting(const int rowOffset, const int colOffset, const Square old, const Square target){
    Square current = old;
    current.row += rowOffset;
    current.col += colOffset;

    int i{};
    while(current != target){
        cout << i;
        if(board[current.row][current.col]){
            cout << "Piece hit another piece before target @" << current.row << ", " <<current.col << "with val of: " << board[current.row][current.col] << endl;
            return true;
        }
        current.row += rowOffset;
        current.col += colOffset;
        i++;
    }
    return false;
}

bool Chess::isLegalBishopMove(const Square old, const Square target){
    int rowOffset = target.row - old.row;
    int colOffset = target.col - old.col;

    // Does piece move diagonally evenly
    if(abs(rowOffset) - abs(colOffset) == 0){
        rowOffset = rowOffset/abs(rowOffset);
        colOffset = colOffset/abs(colOffset);
    }
    else{
        return false;
    }
    return !isPieceInterrupting(rowOffset, colOffset, old, target);
}

bool Chess::isLegalRookMove(const Square old, const Square target){
    int rowOffset = target.row - old.row;
    int colOffset = target.col - old.col;

    // Does piece moved cardinally
    if(rowOffset == 0 ^ colOffset == 0){
        if(rowOffset != 0){
            rowOffset = rowOffset/abs(rowOffset);
        }
        else if(colOffset != 0){
            colOffset = colOffset/abs(colOffset);
        }
    }
    else{
        return false;
    }
    return !isPieceInterrupting(rowOffset, colOffset, old, target);
}

bool Chess::isLegalKnightMove(const Square old, const Square target){
    int rowOffsets[8]{-2, -2, -1, -1, 1, 1, 2, 2};
    int colOffsets[8]{1, -1, 2, -2, -2, 2, 1, -1};

    for(int i = 0; i < 8; i++){
        Square possibleJump = old;
        possibleJump.row += rowOffsets[i];
        possibleJump.col += colOffsets[i];
        if(possibleJump == target){
            return true;
        }
    }
    return false;
}

bool Chess::isLegalPawnMove(const Square old, const Square target){
    int rowOffset = target.row - old.row;
    int colOffset = target.col - old.col;
    int direction = board[old.row][old.col];

    // Piece must move 'up'
    if(rowOffset * direction > 0){
        cout << "Pawn attempted to move backwards" << endl;
        return false;
    }
    // If attacking
    if(abs(colOffset) == 1 && rowOffset * direction == -1){
        return board[target.row][target.col] * direction <= 0;
    }
    // If first move jump
    else if(colOffset == 0 && abs(rowOffset) == 2){
        if((direction == WHITE && old.row != 6) || (direction == BLACK && old.row != 1)){
            cout << "Pawn attempted leap outside of starting turn" << endl;
            return false;
        }
        return !isPieceInterrupting(-direction, 0, old, target);
    }
    else if(colOffset == 0 and abs(rowOffset) == 1){
        return true;
    }
    else{
        return false;
    }
}

void Chess::movePieceUnconditionally(const Square old, const Square target){
    // Extra Taking logic
    bool isKingTaken = abs(board[target.row][target.col]) == 6;
    if(board[target.row][target.col] != 0 && board[old.row][old.col] != 0){
        float wx = target.col + 0.5f;
        float wy = 8.0f - target.row - 0.5f;
        emit capture_at(wx, wy, 30);

        if (debugging) cout << "Tango Down, load the 'fetti' launcher" << endl;
    }

    board[target.row][target.col] = board[old.row][old.col];
    board[old.row][old.col] = 0;
    if(debugging) printBoard();
    switchPlayer();

    // play sound effect
    QSoundEffect *effect = new QSoundEffect;
    effect->setSource(QUrl("qrc:/Assets/Assets/ChessMove.wav"));
    effect->setVolume(1);
    effect->play();

    if (isKingTaken){
        emit won_game();
        effect->setSource(QUrl("qrc:/Assets/Assets/Confetti.wav"));
        effect->setVolume(1);
        effect->play();
    }
    emit update_board();
}

void Chess::loadDefaultBoard(){
    int defaultBoard[8][8]{
        {-ROOK,-KNIGHT,-BISHOP,-QUEEN,-KING,-BISHOP,-KNIGHT,-ROOK}, // Black on Top
        {-PAWN,-PAWN,-PAWN,-PAWN,-PAWN,-PAWN,-PAWN,-PAWN},
        {0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0},
        {PAWN, PAWN, PAWN, PAWN, PAWN, PAWN, PAWN, PAWN},
        {ROOK, KNIGHT, BISHOP, QUEEN, KING, BISHOP, KNIGHT, ROOK}, // White on Bottom
    };
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            board[row][col] = defaultBoard[row][col];
        }
    }
    if(debugging) printBoard();
}

void Chess::loadBoard(int newBoard[8][8]){
    // For performance could probably use pointer or ref
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            board[row][col] = newBoard[row][col];
        }
    }
}

void Chess::printBoard(){
    cout << " ===== Board ===== " << endl;

    cout << "   "; // Empty Space
    for(char c = 'a'; c < 'i'; c++){
        cout << c << "  ";
    }
    cout << endl;

    for(int i = 0; i < 25; i++) cout << "-";
    cout << endl;

    for(int x = 0; x < 8; x++){
        cout << 8 - x << "|";
        for(int y = 0; y < 8; y++){
            if(board[x][y] >= 0) cout << " ";
            cout << board[x][y] << " ";
        }
        cout << endl;
    }
}

std::vector<std::vector<int>> Chess::getBoardVector() const {
    std::vector<std::vector<int>> v(8, std::vector<int>(8));
    for(int i = 0; i < 8; ++i)
        for(int j = 0; j < 8; ++j)
            v[i][j] = board[i][j];
    return v;
}
