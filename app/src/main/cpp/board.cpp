#include <zconf.h>
#include "board.h"
#include "bitutils.h"
#include "bitutils.cpp"
#include "types.h"

#define MULTI_TAKE_FLAG 0b10000000000000

_move createMove(_ci from, _ci to, _cb isTake, _cb isWhite, _cb isQueen) {
    return static_cast<_move>(isTake | (from << 1) | (to << 6) | (isWhite << 11) | (isQueen << 12));
}


inline _ui isTake(_cmove move) {
    return move & 1u;
}

inline _ui getFrom(_cmove move) {
    return (move >> 1) & 0b11111u;
}

inline _ui getTo(_cmove move) {
    return (move >> 6) & 0b11111u;
}

const _ui FIRST_HORIZONTAL[] = {
        1, 1, 0, 0, 1,
        0, 0, 0, 0, 1,
        0, 0, 0, 0, 0,
        0, 0, 0, 0, 0,
        0, 0, 0, 0, 0,
        0, 0, 0, 0, 0,
        0, 0
};

const _ui LAST_HORIZONTAL[] = {
        0, 0, 0, 0, 0,
        0, 0, 0, 0, 0,
        0, 0, 0, 0, 0,
        0, 0, 0, 0, 0,
        0, 0, 1, 0, 0,
        0, 0, 1, 0, 0,
        1, 1
};

const _ui UN_ROTATED_CELLS[] = {
        9, 16,
        4, 10, 17, 23,
        1, 5, 11, 18, 24, 28,
        0, 2, 6, 12, 19, 25, 29, 31,
        3, 7, 13, 20, 26, 30,
        8, 14, 21, 27,
        15, 22,
};

const _ui ROTATED_CELLS[] = {
        12,
        6, 13, 20,
        2, 7, 14, 21, 26,
        0, 3, 8, 15, 22, 27, 30,
        1, 4, 9, 16, 23, 28, 31,
        5, 10, 17, 24, 29,
        11, 18, 25,
        19
};

const _ui SHIFT[] = {
        0,
        1, 1, 1,
        4, 4, 4, 4, 4,
        9, 9, 9, 9, 9, 9, 9,
        16, 16, 16, 16, 16, 16, 16,
        23, 23, 23, 23, 23,
        28, 28, 28,
        31
};

const _ui SHIFT_90[] = {
        0, 0,
        2, 2, 2, 2,
        6, 6, 6, 6, 6, 6,
        12, 12, 12, 12, 12, 12, 12, 12,
        20, 20, 20, 20, 20, 20,
        26, 26, 26, 26,
        30, 30

};

const _ui MASK[] = {
        0b1,
        0b111, 0b111, 0b111,
        0b11111, 0b11111, 0b11111, 0b11111, 0b11111,
        0b1111111, 0b1111111, 0b1111111, 0b1111111, 0b1111111, 0b1111111, 0b1111111,
        0b1111111, 0b1111111, 0b1111111, 0b1111111, 0b1111111, 0b1111111, 0b1111111,
        0b11111, 0b11111, 0b11111, 0b11111, 0b11111,
        0b111, 0b111, 0b111,
        0b1,
};

const _ui MASK_90[] = {
        0b11, 0b11,
        0b1111, 0b1111, 0b1111, 0b1111,
        0b111111, 0b111111, 0b111111, 0b111111, 0b111111, 0b111111,
        0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111,
        0b11111111,
        0b111111, 0b111111, 0b111111, 0b111111, 0b111111, 0b111111,
        0b1111, 0b1111, 0b1111, 0b1111,
        0b11, 0b11
};


_move ***movesW[32];
_move ***movesB[32];
_move ***movesWQueen[32];
_move ***movesBQueen[32];
_move ***movesW90[32];
_move ***movesB90[32];
_move ***movesWQueen90[32];
_move ***movesBQueen90[32];

static const int MOVES_SET_POWER = 2 * 2 * 2 * 32 * 32 * 2;

_ui takeMasks[MOVES_SET_POWER];
_ui moveBitMasks[MOVES_SET_POWER];
_ui moveBitMasksQueen[MOVES_SET_POWER];
_ui takeMasks90[MOVES_SET_POWER];
_ui moveBitMasks90[MOVES_SET_POWER];
_ui moveHash[MOVES_SET_POWER];

const int dirs[] = {1, -1};

_move *genMoves(
        _ui posOnBoard,
        _ui pos,
        _ui length,
        _board enemy,
        _board all,
        bool isQueen,
        bool white,
        bool rotated
) {
    std::vector<_move> moves;
    bool globalTaking = false;
    for (const auto &dir : dirs) {
        if (isQueen) {
            bool taking = false;
            for (int i = 1; i < 8; ++i) {
                int currentPos = pos + i * dir;
                if (currentPos < 0 || currentPos >= length)
                    break;
                _ui unsignedCurrentPos = static_cast<_ui>(currentPos);
                _ui forward = unsignedCurrentPos + dir;
                if (!getBit(all, unsignedCurrentPos)) {
                    if (globalTaking == taking) {
                        _ci to = posOnBoard - pos + unsignedCurrentPos;
                        moves.push_back(createMove(
                                rotated ? UN_ROTATED_CELLS[posOnBoard] : posOnBoard,
                                rotated ? UN_ROTATED_CELLS[to] : to,
                                globalTaking,
                                white,
                                isQueen
                        ));
                    }
                } else if (taking) {
                    break;
                } else if (getBit(enemy, unsignedCurrentPos) && forward >= 0 && forward < length &&
                           !getBit(all, forward)) {
                    if (!globalTaking)
                        moves.clear();
                    globalTaking = true;
                    taking = true;
                } else {
                    break;
                }
            }
        } else {
            int forward = pos + dir;
            if (forward < 0 || forward >= length)
                continue;
            _ui unsignedForward = static_cast<_ui>(forward);
            if (!globalTaking && dir == 1 == white && !getBit(all, unsignedForward)) {
                _ci to = posOnBoard - pos + forward;
                moves.push_back(createMove(
                        rotated ? UN_ROTATED_CELLS[posOnBoard] : posOnBoard,
                        rotated ? UN_ROTATED_CELLS[to] : to,
                        globalTaking,
                        white,
                        isQueen
                ));
                continue;
            }
            int forward2 = forward + dir;
            if (forward2 < 0 || forward2 >= length)
                continue;
            _ui unsignedForward2 = static_cast<_ui>(forward2);
            if (getBit(enemy, unsignedForward) && !getBit(all, unsignedForward2)) {
                if (!globalTaking)
                    moves.clear();
                globalTaking = true;

                int to = posOnBoard - pos + unsignedForward2;
                moves.push_back(createMove(
                        rotated ? UN_ROTATED_CELLS[posOnBoard] : posOnBoard,
                        rotated ? UN_ROTATED_CELLS[to] : to,
                        globalTaking,
                        white,
                        isQueen
                ));
            }
        }
    }

    _move *movesArray = new _move[moves.size() + 1];
    auto size = moves.size();
    *movesArray = size;
    for (int i = 1; i <= size; ++i) {
        movesArray[i] = moves[i - 1];
    }

    return movesArray;
}

void genMoves() {
    for (_ui cell = 0; cell < 32; ++cell) {
        _ui length = getLowestBit(MASK[cell] + 1);
        _ui lengthPower = 1u << length;
        movesWQueen[cell] = new _move **[lengthPower];
        movesBQueen[cell] = new _move **[lengthPower];
        movesB[cell] = new _move **[lengthPower];
        movesW[cell] = new _move **[lengthPower];
        for (int i = 0; i < lengthPower; ++i) {
            movesWQueen[cell][i] = new _move *[lengthPower];
            movesBQueen[cell][i] = new _move *[lengthPower];
            movesB[cell][i] = new _move *[lengthPower];
            movesW[cell][i] = new _move *[lengthPower];
        }
        for (_ui enemy = 0; enemy < lengthPower; ++enemy) {
            for (_ui all = 0; all < lengthPower; ++all) {
                _ui all0 = all | enemy;
                _ui pos = cell - SHIFT[cell];
                movesWQueen[cell][enemy][all] = genMoves(cell, pos, length, enemy, all0, true, true,
                                                         false);
                movesBQueen[cell][enemy][all] = genMoves(cell, pos, length, enemy, all0, true,
                                                         false, false);
                movesW[cell][enemy][all] = genMoves(cell, pos, length, enemy, all0, false, true,
                                                    false);
                movesB[cell][enemy][all] = genMoves(cell, pos, length, enemy, all0, false, false,
                                                    false);
            }
        }

        _ui rotatedCell = ROTATED_CELLS[cell];
        length = getLowestBit(MASK_90[rotatedCell] + 1);
        lengthPower = 1u << length;
        movesWQueen90[rotatedCell] = new _move **[lengthPower];
        movesBQueen90[rotatedCell] = new _move **[lengthPower];
        movesB90[rotatedCell] = new _move **[lengthPower];
        movesW90[rotatedCell] = new _move **[lengthPower];
        for (int i = 0; i < lengthPower; ++i) {
            movesWQueen90[rotatedCell][i] = new _move *[lengthPower];
            movesBQueen90[rotatedCell][i] = new _move *[lengthPower];
            movesB90[rotatedCell][i] = new _move *[lengthPower];
            movesW90[rotatedCell][i] = new _move *[lengthPower];
        }

        for (_ui enemy = 0; enemy < lengthPower; ++enemy) {
            for (_ui all = 0; all < lengthPower; ++all) {

                /*
                 cell = 0;
                 rotatedCell = ROTATED_CELLS[cell];
                 enemy = 0;
                 all = 0;
                 length = 8;
                 */

                if (rotatedCell == 13 && enemy == 8 && all == 10) {
                    int a = 123;
                    int b = a + 23;
                }

                _ui all0 = all | enemy;
                _ui pos = rotatedCell - SHIFT_90[rotatedCell];


                movesWQueen90[rotatedCell][enemy][all] = genMoves(rotatedCell, pos, length, enemy,
                                                                  all0, true, true, true);
                movesBQueen90[rotatedCell][enemy][all] = genMoves(rotatedCell, pos, length, enemy,
                                                                  all0, true, false, true);
                movesW90[rotatedCell][enemy][all] = genMoves(rotatedCell, pos, length, enemy, all0,
                                                             false, true, true);
                movesB90[rotatedCell][enemy][all] = genMoves(rotatedCell, pos, length, enemy, all0,
                                                             false, false, true);
            }
        }
    }
}


_hash zorbistKeys[4][32];


void genMovesMask() {
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 32; ++j) {
            zorbistKeys[i][j] = rand() & HASH_MASK;
        }
    }

    for (_ui multiTake = 0; multiTake < 2; ++multiTake)
        for (_ui white = 0; white < 2; ++white)
            for (_ui queen = 0; queen < 2; ++queen)
                for (_ui from = 0; from < 32; ++from)
                    for (_ui to = 0; to < 32; ++to)
                        for (_ui isTake = 0; isTake < 2; ++isTake) {
                            if (from == to)
                                break;
                            _ui rotatedFrom = ROTATED_CELLS[from];
                            _ui rotatedTo = ROTATED_CELLS[to];
                            _move move = createMove(from, to,
                                                    static_cast<const bool &>(isTake),
                                                    static_cast<_cb &>(white),
                                                    static_cast<_cb>(queen));
                            if (move == 71) {
                                int asda;
                            }
                            if (multiTake) {
                                move |= MULTI_TAKE_FLAG;
                            }
                            if (SHIFT[from] == SHIFT[to]) {
                                _ui take = 0u;
                                for (int cell = (to < from ? to : from);
                                     cell <= (to > from ? to : from); ++cell) {
                                    take |= 1 << cell;
                                }
                                takeMasks[move] = ~take;
                            } else if (SHIFT_90[rotatedFrom] == SHIFT_90[rotatedTo]) {
                                _ui take = 0u;

                                for (int rotatedCell = (rotatedTo < rotatedFrom ? rotatedTo
                                                                                : rotatedFrom);
                                     rotatedCell <=
                                     (rotatedTo > rotatedFrom ? rotatedTo : rotatedFrom);
                                     ++rotatedCell) {
                                    take |= 1 << UN_ROTATED_CELLS[rotatedCell];
                                }
                                takeMasks[move] = ~take;
                            } else {
                                continue;
                            }
                            if (queen) {
                                moveBitMasks[move] = 0;
                                moveBitMasksQueen[move] = (1u << from) | (1u << to);

                                moveBitMasks90[move] =
                                        (1u << ROTATED_CELLS[from]) | (1u << ROTATED_CELLS[to]);
                            } else if ((white && LAST_HORIZONTAL[to]) ||
                                       (!white && FIRST_HORIZONTAL[to])) {
                                moveBitMasks[move] = 1u << from;
                                moveBitMasksQueen[move] = 1u << to;
                            } else {
                                moveBitMasksQueen[move] = 0;
                                moveBitMasks[move] = (1u << from) | (1u << to);
                            }

                            //rotated
                            for (_ui i = 0; i < 32; ++i) {
                                if (getBit(takeMasks[move], i))
                                    setBitAssign(takeMasks90[move], ROTATED_CELLS[i]);
                                if (getBit(moveBitMasks[move] | moveBitMasksQueen[move], i))
                                    setBitAssign(moveBitMasks90[move], ROTATED_CELLS[i]);
                            }

                            int type;
                            if (white) {
                                if (queen) {
                                    type = 1;
                                } else {
                                    type = 0;
                                }
                            } else {
                                if (queen) {
                                    type = 3;
                                } else {
                                    type = 2;
                                }
                            }
                            if (move == 6339) {
                                int a = 213;
                            }

                            int type2 = type;
                            if (white && LAST_HORIZONTAL[to]) {
                                type2 = 1;
                            }
                            if (!white && FIRST_HORIZONTAL[to]) {
                                type2 = 3;
                            }
                            moveHash[move] = zorbistKeys[type][from] ^ zorbistKeys[type2][to];
                        }
}


void gen() {
    genMoves();
    genMovesMask();
}

_move *EMPTY_MOVES = new _move(0);

inline _move *__getTakes(
        _cboard all,
        _cboard enemy,
        _cboard all90,
        _cboard enemy90,
        _cb isWhiteMove,
        _ui cell,
        bool queen,
        bool rotated
) {
    _move *moves;
    if (rotated) {
        _ui rotatedCell = ROTATED_CELLS[cell];
        moves = (queen ? (isWhiteMove ? movesWQueen90 : movesBQueen90) : (isWhiteMove ? movesW90
                                                                                      : movesB90))
        [rotatedCell][(enemy90 >> SHIFT_90[rotatedCell]) & MASK_90[rotatedCell]]
        [(all90 >> SHIFT_90[rotatedCell]) & MASK_90[rotatedCell]];
    } else {
        moves = (queen ? (isWhiteMove ? movesWQueen : movesBQueen) : (isWhiteMove ? movesW
                                                                                  : movesB))
        [cell]
        [(enemy >> SHIFT[cell]) & MASK[cell]]
        [(all >> SHIFT[cell]) & MASK[cell]];
    }
    if (*moves == 0 || isTake(moves[1])) return moves;
    return EMPTY_MOVES;
}

_move currentMoves[256];


void __getMoves(
        _cboard our_c,
        _cboard enemy_c,
        _cboard our_q,
        _cboard enemy_q,
        _cboard our90,
        _cboard enemy90,
        _cb isWhiteMove,
        bool take,
        int figureCell
) {
    _move currentMovesSize = 0;

    _ui enemy = enemy_c | enemy_q;
    _ui our = our_c | our_q;
    _ui all = enemy | our;
    _ui all90 = enemy90 | our90;
    for (_ui queen = 0; queen < 2; ++queen) {
        _board ourIter = queen ? our_q : our_c;
        while (ourIter) {
            _ui cell = getLowestBit(ourIter);
            zeroLowestBitAssign(ourIter);

            if (figureCell != -1) {
                queen = getBit(our_q, figureCell);
                cell = figureCell;
            }

            for (_ui rotated = 0; rotated < 2; ++rotated) {
                _move *generatedMoves = rotated ?
                                        (queen ? (isWhiteMove ? movesWQueen90 : movesBQueen90)
                                               : (isWhiteMove ? movesW90 : movesB90))
                                        [ROTATED_CELLS[cell]]
                                        [(enemy90 >> SHIFT_90[ROTATED_CELLS[cell]]) &
                                         MASK_90[ROTATED_CELLS[cell]]]
                                        [(all90 >> SHIFT_90[ROTATED_CELLS[cell]]) &
                                         MASK_90[ROTATED_CELLS[cell]]]
                                                :
                                        (queen ? (isWhiteMove ? movesWQueen : movesBQueen)
                                               : (isWhiteMove ? movesW : movesB))
                                        [cell]
                                        [(enemy >> SHIFT[cell]) & MASK[cell]]
                                        [(all >> SHIFT[cell]) & MASK[cell]];
                _move size = *generatedMoves;

                if (size == 0) continue;

                if (isTake(generatedMoves[1])) {
                    if (!take) {
                        currentMovesSize = 0;
                        take = true;
                    }
                } else if (take) continue;

                if (take) {
                    if (queen) {
                        bool opposite = false;
                        int queenMoves = 0;
                        bool queenTaking = false;
                        for (int i = 1;; ++i) {
                            if (i > size) goto breakLoop;
                            _move move = generatedMoves[i];
                            _ui to = getTo(move);
                            if (!opposite && to < cell) {
                                queenMoves = 0;
                                queenTaking = false;
                                opposite = true;
                            }
                            bool isNotEmpty = *__getTakes(all, enemy, all90, enemy90, isWhiteMove,
                                                          to, true, !rotated);

                            if (!queenTaking && !isNotEmpty) {
                                _move *m = __getTakes(all, enemy, all90, enemy90, isWhiteMove, to,
                                                      true, (bool) rotated);
                                _move msize = *m;
                                if (msize > 0) {
                                    bool dir = cell > to;
                                    if (((getFrom(m[1]) > getTo(m[1])) == dir) ||
                                        ((getFrom(m[msize]) > getTo(m[msize])) == dir)) {
                                        isNotEmpty = true;
                                    }
                                }
                            }

                            if (isNotEmpty) {
                                if (!queenTaking) {
                                    currentMovesSize -= queenMoves;
                                    queenTaking = true;
                                }
                            } else if (queenTaking) continue;
                            if (queenTaking) {
                                move |= MULTI_TAKE_FLAG;
                            }
                            currentMoves[++currentMovesSize] = move;
                            queenMoves++;
                        }
                    } else {
                        for (int i = 1;; ++i) {
                            if (i > size) goto breakLoop;
                            _move move = generatedMoves[i];
                            _ui to = getTo(move);
                            if (
                                    *__getTakes(all, enemy, all90, enemy90, isWhiteMove, to, false,
                                                !rotated) > 0 ||
                                    *__getTakes(all, enemy, all90, enemy90, isWhiteMove, to, false,
                                                (bool) rotated) > 0
                                    ) {
                                move |= MULTI_TAKE_FLAG;
                            } else if (((isWhiteMove && LAST_HORIZONTAL[to]) || (!isWhiteMove && FIRST_HORIZONTAL[to]))) {
                                if (*__getTakes(all, enemy, all90, enemy90, isWhiteMove, to, true, !rotated)) {
                                    move |= MULTI_TAKE_FLAG;
                                }
                            }
                            currentMoves[++currentMovesSize] = move;
                        }
                    }
                } else {
                    for (int i = 1; i <= size; ++i) {
                        currentMoves[++currentMovesSize] = generatedMoves[i];
                    }
                }
                breakLoop:;
            }
            if (figureCell != -1) goto ending;
        }
    }

    ending:
    *currentMoves = currentMovesSize;
}

void getMoves(
        _cboard wc,
        _cboard bc,
        _cboard wq,
        _cboard bq,
        _cboard w90,
        _cboard b90,
        _cb isWhiteMove,
        bool take,
        int figureCell
) {
    if (isWhiteMove) {
        __getMoves(wc, bc, wq, bq, w90, b90, isWhiteMove, take, figureCell);
    } else {
        __getMoves(bc, wc, bq, wq, b90, w90, isWhiteMove, take, figureCell);
    }
}

inline void swap(_move &a, _move &b) {
    _move tmp = a;
    a = b;
    b = tmp;
}

void sortMoves(_move hashedMove) {
    _move movesSize = *currentMoves;
    if (movesSize < 2)
        return;
/*
    _move *previous = new _move[*currentMoves];
    for (int i = 1; i <= *currentMoves; ++i) {
        previous[i - 1] = currentMoves[i];
    }*/


    for (int i = 1; i <= movesSize; ++i) {
        if (currentMoves[i] == hashedMove) {
            swap(currentMoves[1], currentMoves[i]);
            break;
        }
    }


/*    int i = 1;
    t:
    while (i <= *currentMoves) {
        _move move = currentMoves[i];
        for (int j = 0; j < *currentMoves; ++j) {
            if (move == previous[j]) {
                ++i;
                goto t;
            }
        }
        throw 1488;
    }*/
}

inline void __makeMove(
        _board &our_c,
        _board &enemy_c,
        _board &our_q,
        _board &enemy_q,
        _board &our90,
        _board &enemy90,
        _cmove &move,
        _hash &hash,
        bool isWhiteTurn
) {
    _board takeMask = takeMasks[move];

    hash ^= moveHash[move];
    if (isTake(move)) {
        _board q = enemy_q & ~takeMask;
        _board c = enemy_c & ~takeMask;

        int type;
        int cell;
        if (isWhiteTurn) {
            if (q) {
                type = 3;
                cell = getLowestBit(q);
            } else {
                type = 2;
                cell = getLowestBit(c);
            }
        } else {
            if (q) {
                type = 1;
                cell = getLowestBit(q);
            } else {
                type = 0;
                cell = getLowestBit(c);
            }
        }
        hash ^= zorbistKeys[type][cell];
    }


    our_c ^= moveBitMasks[move];
    enemy_c &= takeMask;
    our_q ^= moveBitMasksQueen[move];
    enemy_q &= takeMask;
    our90 ^= moveBitMasks90[move];
    enemy90 &= takeMasks90[move];
}

void makeMove(
        _board &wc,
        _board &bc,
        _board &wq,
        _board &bq,
        _board &w90,
        _board &b90,
        _cmove move,
        _hash &hash,
        _cb isWhiteMove
) {
    if (isWhiteMove)
        __makeMove(wc, bc, wq, bq, w90, b90, move, hash, true);
    else
        __makeMove(bc, wc, bq, wq, b90, w90, move, hash, false);
}

_board rotateBoard(_board board) {
    _board rotated = 0;
    for (_ui cell = 0; cell < 32; ++cell) {
        if (getBit(board, cell))
            rotated = setBit(rotated, ROTATED_CELLS[cell]);
    }

    return rotated;
}