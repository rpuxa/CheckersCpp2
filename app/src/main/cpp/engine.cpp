#include <vector>
#include "types.h"
#include "board.h"
#include "evaluate.h"
#include "evaluate.cpp"


using namespace types;
using namespace std;

const short WHITE_LOSING = -30000;
const short BLACK_LOSING = 30000;
const short NO_SCORE = 0b1000000010000000;

_move hashedMoves[HASH_SIZE];
short hashedScores[HASH_SIZE];
_pos positions[HASH_SIZE * 2];

_move movesLine[32];

_hash mhash(_board wc,
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
int c = 0;
volatile bool stop;
const int POS_SIZE = 18;

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
        short maxDepth,
        _move previousMove,
        int moveCount,
        int &moveLineLength
) {
   if (stop) {
       return CANCEL_SCORE;
   }

    short score;
    /*
    if (mhash(wc, bc, wq, bq) != hash) {
        throw exception();
    }*/

    _pos pos[5] = {wc, bc, wq, bq, static_cast<_pos>(
            (isWhiteMove << 6) | (depth << 7)
    )};
    if (previousMove & MULTI_TAKE_FLAG) {
        pos[4] |= (getTo(previousMove) << 1) | 1;
    }
    short hashedScore = hashedScores[hash];
    if (hashedScore != NO_SCORE && memcmp(positions + (hash * 2), pos, POS_SIZE) == 0) {
        c++;
        return hashedScore;
    }


    if (depth >= maxDepth) {
        score = eval(wc, bc, wq, bq);
        return score;
    }

    if (previousMove & MULTI_TAKE_FLAG) {
        getMoves(wc, bc, wq, bq, w90, b90, isWhiteMove, true, getTo(previousMove));
    } else {
        getMoves(wc, bc, wq, bq, w90, b90, isWhiteMove, false, -1);
    }
    sortMoves(hashedMoves[hash]);

    _move movesSize = *currentMoves;
    if (movesSize == 0) {
        score = isWhiteMove ? WHITE_LOSING + depth : BLACK_LOSING - depth;
        return score;
    }
    _move *moves = new _move[movesSize];
    memcpy(moves, currentMoves + 1, movesSize * sizeof(_move));

    _move *bestLine = nullptr;
    _move bestLineLength = 0;
    short copyAlpha = alpha;
    short copyBeta = beta;

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

        int length = 0;

        short result = alphaBeta(
                copy_wc, copy_bc, copy_wq,
                copy_bq, copy_w90, copy_b90,
                copy_hash,
                (move & MULTI_TAKE_FLAG) != 0 == isWhiteMove,
                alpha,
                beta,
                move & MULTI_TAKE_FLAG ? depth : depth + 1,
                isTake(move) && !(move & MULTI_TAKE_FLAG) && depth + 1 == maxDepth ? maxDepth + 1
                                                                                   : maxDepth,
                move,
                moveCount + 1,
                length
        );

        if (result == CANCEL_SCORE) {
            return CANCEL_SCORE;
        }

        if (isWhiteMove) {
            if (result > alpha) {
                alpha = result;
                goto resultHasBeenImproved;
            }
        } else {
            if (result < beta) {
                beta = result;

                resultHasBeenImproved:
                delete[] bestLine;
                bestLine = new _move[length + 1];
                bestLine[0] = move;
                memcpy(bestLine + 1, movesLine + (moveCount + 1), length * sizeof(_move));
                bestLineLength = length + 1;
                hashedMoves[hash] = move;
            }
        }
        if (alpha >= beta)
            break;
    }

    delete[] moves;
    moveLineLength = bestLineLength;
    memcpy(movesLine + moveCount, bestLine, bestLineLength * sizeof(_move));
    delete[] bestLine;
    score = isWhiteMove ? alpha : beta;

    // score hash
    if (score > copyAlpha && score < copyBeta) {
        hashedScores[hash] = score;
        memcpy(positions + (hash * 2), pos, POS_SIZE);
    }

    return score;
}


void getBestMove(
        _board wc,
        _board bc,
        _board wq,
        _board bq,
        _board w90,
        _board b90,
        int multiTake,
        _cb isWhiteMove,
        short depth,
        std::vector<_move> &bestLine,
        short &eval
) {
    stop = false;
    memset(hashedScores, 1 << 7, HASH_SIZE * sizeof(short));

    _move previousMove;
    if (multiTake == -1) {
        previousMove = 0;
    } else {
        previousMove = createMove(0, multiTake, 0, 0, 0) | MULTI_TAKE_FLAG;
    }

    int length = 0;
    eval = alphaBeta(
            wc,
            bc,
            wq,
            bq,
            w90,
            b90,
            mhash(wc, bc, wq, bq),
            isWhiteMove,
            WHITE_LOSING,
            BLACK_LOSING,
            0,
            depth,
            previousMove,
            0,
            length
    );
    for (int i = 0; i < length; ++i) {
        bestLine.push_back(movesLine[i]);
    }
}

void stopSearching() {
    stop = true;
}