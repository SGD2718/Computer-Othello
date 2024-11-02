//
// Created by Benjamin Lee on 1/16/24.
//

#include "Game.h"

Game::Game(const std::vector<Move>& moves) {
    this->board = Board();
    this->moves = {};
    this->play_moves(moves);
}

Game::Game(const std::string& moveSequence) {
    this->board = Board();
    this->moves = {};
    this->play_moves(moveSequence);
}

bool Game::play_move(const Move &move) {
    if (!this->is_move_legal(move)) {
        std::cerr << "Invalid move: " << move << std::endl;
        return false;
    }

    // delete redo history
    while (this->moves.size() > this->moveIndex) {
        this->moves.pop_back();
    }
    ++this->moveIndex;

    // play the move
    this->moves.emplace_back(move);

    if (this->moveIndex != this->moves.size())
        std::cerr << "your code broke: moves.size() = " << this->moves.size() << ", moveIndex = " << this->moveIndex << std::endl;

    if (!move.is_pass()) {
        this->board.play_move(move);
        return true;
    }

    this->board.pass();
    return true;
}

bool Game::play_move(uint8_t position) {
    if (position == I_PASS) return this->pass();

    if (!this->is_move_legal(position)) {
        std::cerr << "Invalid move: " << (int)position << " (" << char((position & 7) + 'a') << char((position >> 3) + '1') << ')' << std::endl;
        return false;
    }

    // delete redo history
    while (this->moves.size() > this->moveIndex) {
        this->moves.pop_back();
    }
    ++this->moveIndex;

    this->moves.emplace_back(this->board, position);
    this->board.play_move(moves.back());

    if (this->moveIndex != this->moves.size())
        std::cerr << "your code broke: moves.size() = " << this->moves.size() << ", moveIndex = " << this->moveIndex << std::endl;

    return true;
}

bool Game::play_move(uint_fast8_t col, uint_fast8_t row) {
    if (!this->is_move_legal(col, row)) {
        std::cerr << "Invalid move: " << (char)(col + 'a') << ", " << char(row + '1') << std::endl;
        return false;
    }

    // delete redo history
    while (this->moves.size() > this->moveIndex) {
        this->moves.pop_back();
    }
    ++this->moveIndex;

    this->moves.emplace_back(this->board, col, row);
    this->board.play_move(moves.back());

    if (this->moveIndex != this->moves.size())
        std::cerr << "your code broke: moves.size() = " << this->moves.size() << ", moveIndex = " << this->moveIndex << std::endl;
    return true;
}

bool Game::play_move(const std::string& coordinates) {
    if (!this->is_move_legal(coordinates)) {
        std::cerr << "Invalid move: " << coordinates << std::endl;
        return false;
    }

    // delete redo history
    while (this->moves.size() > this->moveIndex) {
        this->moves.pop_back();
    }
    ++this->moveIndex;

    if (coordinates == "PASS" || coordinates == "pass" || coordinates == "Pass" || coordinates.empty()) {
        return this->pass();
    }

    this->moves.emplace_back(this->board, coordinates);

    if (this->moveIndex != this->moves.size())
        std::cerr << "your code broke: moves.size() = " << this->moves.size() << ", moveIndex = " << this->moveIndex << std::endl;
    this->board.play_move(moves.back());
    return true;
}

/* play a sequence of moves read from a vector of strings.
 * returns false if the move sequence is invalid, true otherwise
 * @param moveSequence: a vector of Move objects. Passes are optional.
 */
bool Game::play_moves(const std::vector<Move>& moveSequence) {
    for (const auto& move : moveSequence) {
        if (!move.is_pass() && this->board.get_legal_moves() == 0) this->pass();
        if (!this->play_move(move)) return false;
    }
    return true;
}

/* play a sequence of moves read from a vector of strings.
 * returns false if the move sequence is invalid, true otherwise
 * @param moveSequence: a vector of moves as position indices (i.e., h1 = 0, h8 = 63). Passes will be detected automatically.
 */
bool Game::play_moves(const std::vector<uint8_t>& moveSequence) {
    for (const auto& move : moveSequence) {
        if (this->board.get_legal_moves() == 0) this->pass();
        if (!this->play_move(move)) return false;
    }
    return true;
}

/* play a sequence of moves read from a vector of strings.
 * returns false if the move sequence is invalid, true otherwise
 * @param moveSequence: a vector of moves as (column, row) pairs. Passes will be detected automatically.
 */
bool Game::play_moves(const std::vector<std::pair<uint8_t, uint8_t>>& moveSequence) {
    for (const auto& [col, row] : moveSequence) {
        if (this->board.get_legal_moves() == 0) this->pass();
        if (!this->play_move(col, row)) return false;
    }
    return true;
}

/* play a sequence of moves read from a vector of strings.
 * returns false if the move sequence is invalid, true otherwise
 * @param moveSequence: a vector of string moves in algebraic notation. For a pass, use "PASS", "pass", "Pass",
 * or "". Alternatively, you can just not include the pass.
 */
bool Game::play_moves(const std::vector<std::string>& moveSequence) {
    for (const auto& move : moveSequence) {
        if (std::tolower(move[0]) != 'p' && this->board.get_legal_moves() == 0) this->pass();
        if (!this->play_move(move)) return false;
    }
    return true;
}

/* play a sequence of moves read from a string.
 * returns false if the move sequence is invalid, true otherwise
 * @param moveSequence: a string of moves in algebraic notation. For a pass, use "PASS", "pass", "Pass",
 * or "". Alternatively, you can just not include the pass.
 */
bool Game::play_moves(std::string moveSequence) {
    // remove non-alphanumeric characters
    moveSequence.erase(remove_if(moveSequence.begin(),
                                 moveSequence.end(),
                                 [](char c) {
                                     return !isalnum(c);
                                 }),
                       moveSequence.end());

    // to lower case
    auto tmp = moveSequence;
    moveSequence = "";

    for (auto c : tmp)
        moveSequence += (char)std::tolower(c);

    std::cout << "moveSequence: " << moveSequence << std::endl;

    // remove "pass" from the string
    size_t pos = 0;
    while ((pos = moveSequence.find("pass", pos)) != std::string::npos) {
        moveSequence.erase(pos, 4);
    }

    // play the moves
    for (auto i = 0; i < moveSequence.size(); i += 2) {
        if (this->board.get_legal_moves() == 0) this->pass();
        if (!this->play_move(moveSequence.substr(i, 2))) return false;
    }
    return true;
}

std::string Game::to_string() const {
    bool isBlackTurn = !(this->moveIndex & 1);
    return this->board.to_string(isBlackTurn);
}

void Game::set_board(uint64_t black, uint64_t white, bool whiteToMove) {
    assert((black & white) == 0);
    this->moves.clear();
    this->moveIndex = 0;
    if (whiteToMove) this->board.set(white, black);
    else this->board.set(black, white);
}

bool Game::undo_move(bool skipPasses) {
    if (this->moveIndex <= 0) {
        this->moveIndex = 0;
        return false;
    }

    auto move = this->moves[--this->moveIndex];

    if (move.is_pass()) {
        this->board.pass();
        if (skipPasses && this->moveIndex > 0)
            move = this->moves[--this->moveIndex];
        else
            return true;
    }

    this->board.undo_move(move);

    return true;
}

bool Game::redo_move(bool skipPasses) {
    if (this->moveIndex >= moves.size()) {
        this->moveIndex = moves.size();
        return false;
    }

    auto move = this->moves[this->moveIndex++];

    if (move.is_pass())
        this->board.pass();
    else {
        this->board.play_move(move);
        if (moveIndex < moves.size() && skipPasses && this->moves[this->moveIndex].is_pass()) {
            ++this->moveIndex;
            this->board.pass();
        }
    }
    return true;
}

bool Game::pass() {
    if (this->board.get_legal_moves() != 0) {
        std::cerr << "Cannot pass when there are legal moves" << std::endl;
        return false;
    }
    this->board.pass();

    // delete redo history
    while (this->moves.size() > this->moveIndex) {
        this->moves.pop_back();
    }
    ++this->moveIndex;

    this->moves.emplace_back(PASS);
    return true;
}

void Game::set_move_index(int index) {
    if (index < 0 || index > this->moves.size()) {
        std::cerr << "move index out of range: " << index << std::endl;
        return;
    }

    // play moves or undo moves until the position is reached
    while (this->moveIndex < index) this->redo_move();
    while (this->moveIndex > index) this->undo_move();
}
