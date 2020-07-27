#include <vector>
#include <memory.h>
#include "types.h"
#include "board.h"
#include "evaluate.h"
#include "evaluate.cpp"
#include <iostream>

using namespace types;
using namespace std;

const short LOSING = -30000;
const short WINNING = 30000;
const short NO_SCORE = 0b1000000010000000;
const int POS_SIZE = 20;

_move hashedMoves[HASH_SIZE];
short hashedMovesDepth[HASH_SIZE];

void initHashMovesPly() {
    fill_n(hashedMoves, HASH_SIZE, -1024);
}

_hash getHash(_board wc,
              _board bc,
              _board wq,
              _board bq) {
    _hash hash = 0;
    for (int i = 0; i < 32; ++i) {
        if (getBit(wc, i)) {
            hash ^= zorbistKeys[0][i];
        }
    }
    for (int i = 0; i < 32; ++i) {
        if (getBit(wq, i)) {
            hash ^= zorbistKeys[1][i];
        }
    }
    for (int i = 0; i < 32; ++i) {
        if (getBit(bc, i)) {
            hash ^= zorbistKeys[2][i];
        }
    }
    for (int i = 0; i < 32; ++i) {
        if (getBit(bq, i)) {
            hash ^= zorbistKeys[3][i];
        }
    }

    return hash;
}


const short CANCEL_SCORE = -32000;
const short DATABASE_SCORE = -32001;
volatile bool stop;
volatile int analyzed;

short alphaBeta(
        _board wc,
        _board bc,
        _board wq,
        _board bq,
        _board w90,
        _board b90,
        _hash hash,
        _cb isWhiteMove,
        short alpha,
        short beta,
        short depth,
        short ply,
        _move previousMove,
        _move &bestMove,
        _move killer1,
        _move killer2
) {
    if (stop) {
        return CANCEL_SCORE;
    }

    if ((wc | wq) == 0 || (bc | bq) == 0)
        return LOSING + ply;


    /*
    if (depth > 0 && bitCount(wc | bc | wq | bq) <= endgameSize) {
        short result = isWhiteMove ? getEndgame(wc, bc, wq, bq) : -getEndgame(reverse(bc),
                                                                              reverse(wc),
                                                                              reverse(bq),
                                                                              reverse(wq));
        if (result == 0) return 0;
        short score = result > 0 ? 25001 - depth : -25001 + depth;
        return score - result;
    }*/


    if (depth <= 0) {
        analyzed++;
        return eval(wc, bc, wq, bq, isWhiteMove);
    }

    getMoves(wc, bc, wq, bq, w90, b90, isWhiteMove, previousMove);

    _move movesSize = *currentMoves;
    if (movesSize == 0) {
        return LOSING + ply;
    }



    // Отсечение плохих ходов
    if (movesSize > 1 && !isTake(currentMoves[1])) {
        short e = eval(wc, bc, wq, bq, isWhiteMove);
        if (depth == 1 && e <= alpha - 50) //Razoring
            return e;
        if (depth < 6 && e - 4 * depth >= beta) //Futility Puring
            return e;
    }

    sortMoves(hashedMoves[hash], killer1, killer2);

    _move *moves = new _move[movesSize];
    memcpy(moves, currentMoves + 1, movesSize * sizeof(_move));

    killer1 = 0;
    killer2 = 0;


    for (int i = 0; i < movesSize; i++) {
        _move move = moves[i];

        _board copy_wc = wc;
        _board copy_bc = bc;
        _board copy_wq = wq;
        _board copy_bq = bq;
        _board copy_w90 = w90;
        _board copy_b90 = b90;
        _hash copy_hash = hash;

        makeMove(copy_wc, copy_bc, copy_wq, copy_bq, copy_w90, copy_b90, move, copy_hash,
                 isWhiteMove);

        _move childBestMove = 0;


        short result;
        if (move & MULTI_TAKE_FLAG) {
            result = alphaBeta(
                    copy_wc, copy_bc, copy_wq,
                    copy_bq, copy_w90, copy_b90,
                    copy_hash,
                    isWhiteMove,
                    alpha,
                    beta,
                    depth,
                    ply,
                    move,
                    childBestMove,
                    0,
                    0
            );
        } else {
            result = -alphaBeta(
                    copy_wc, copy_bc, copy_wq,
                    copy_bq, copy_w90, copy_b90,
                    copy_hash,
                    !isWhiteMove,
                    -beta,
                    -alpha,
                    static_cast<short>((ply > 6 && i > 3) ? depth - 2 : depth - 1),
                    static_cast<short>(ply + 1),
                    move,
                    childBestMove,
                    killer1,
                    killer2
            );
        }

        if (result == CANCEL_SCORE) {
            return CANCEL_SCORE;
        }
        if (result > alpha) {
            alpha = result;
            bestMove = move;
            if (hashedMovesDepth[hash] < depth) {
                hashedMoves[hash] = move;
                hashedMovesDepth[hash] = depth;
            }
            killer2 = killer1;
            killer1 = childBestMove;
        }
        if (alpha >= beta) {
            history[getColor(move)][getFrom(move)][getTo(move)] += depth * depth;
            break;
        }
    }

    delete[] moves;
    return alpha;
}


void getBestMove(
        _board wc,
        _board bc,
        _board wq,
        _board bq,
        _board w90,
        _board b90,
        _move previousMove,
        _cb isWhiteMove,
        short depth,
        std::vector<_move> &bestLine,
        short &eval
) {
    stop = false;

    getMoves(wc, bc, wq, bq, w90, b90, isWhiteMove, previousMove);
    _move dbMove = getFromDebutBase(wc, bc, wq, bq);
    if (dbMove != 0) {
        bestLine.push_back(dbMove);
        eval = DATABASE_SCORE;
        return;
    }


    analyzed = 0;
    _move bestMove = 0;
    for (int i = 0; i < 2; ++i) {
        for (int j = 0; j < 32; ++j) {
            for (int k = 0; k < 32; ++k) {
                 history[i][j][k] = 0;
            }
        }
    }

    eval = alphaBeta(
            wc,
            bc,
            wq,
            bq,
            w90,
            b90,
            getHash(wc, bc, wq, bq),
            isWhiteMove,
            LOSING,
            WINNING,
            depth,
            0,
            previousMove,
            bestMove,
            0,
            0
    );

    std::cout << "Analyzed positions: " << analyzed << endl;

    if (eval != CANCEL_SCORE && !isWhiteMove) {
        eval = -eval;
    }

    bestLine.push_back(bestMove);
}

void stopSearching() {
    stop = true;
}