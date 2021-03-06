#include "board.h"
#include "bitutils.h"
#include "bitutils.cpp"
#include "types.h"
#include <string>
#include <memory.h>
#include <map>


#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#define MULTI_TAKE_FLAG 0b10000000000000

using namespace std;

_move createMove(_ci from, _ci to, _cb isTake, _cb isWhite, _cb isQueen) {
    return static_cast<_move>(isTake | (from << 1) | (to << 6) | (isWhite << 11) | (isQueen << 12));
}


inline _ui isTake(_cmove move) {
    return move & 1u;
}

inline _ui getColor(_cmove move) {
    return (move >> 11) & 1u;
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

const char DIAGONAL[] = {
        0,
        1, 1, 1,
        2, 2, 2, 2, 2,
        3, 3, 3, 3, 3, 3, 3,
        4, 4, 4, 4, 4, 4, 4,
        5, 5, 5, 5, 5,
        6, 6, 6,
        7
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
bool isPromotion[MOVES_SET_POWER];
// first bit - rotated
// second bit - opposite
unsigned char reversedDirection[MOVES_SET_POWER];

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


_hash zorbistKeys[4][32] = {
        {0x7816f, 0x2de7b, 0x2d51d, 0x10ab6, 0xca6e1, 0x73cbd, 0x5c83e, 0xaf0eb, 0xbbd09, 0xeddfd, 0x319f1, 0x8db7,  0xaeaed, 0x63245, 0xe3086, 0x8f841, 0xc22d4, 0x94916, 0x21afd, 0x45f2,  0x7e666, 0x85117, 0xcb3ad, 0x89f47, 0x35be4, 0x3c42e, 0x95c36, 0xf931b, 0xe843e, 0xef0b5, 0x5b003, 0x1cce1},
        {0x72d41, 0xb6884, 0x400e3, 0xfd59e, 0xa75d9, 0x45e14, 0x1bc61, 0x4d9a0, 0x97513, 0x1ec93, 0x845bc, 0x160c0, 0x73611, 0x8501b, 0x8e87f, 0x8c463, 0xcc416, 0xd4c93, 0x11f81, 0xce4b6, 0xa9d15, 0xd6b74, 0x7b8f1, 0xe6958, 0xd45ad, 0x874c1, 0x30865, 0x4cb48, 0xdf006, 0xf0ccc, 0xa1bd5, 0x9fba0},
        {0x65f60, 0x995de, 0xe1c0c, 0x5b635, 0xf42b1, 0xbaac,  0x56cbc, 0x78f5c, 0x4e5ec, 0xb935,  0xe040e, 0x18f15, 0x7e1eb, 0x9b675, 0xa2447, 0xd5f49, 0x20ab2, 0x61845, 0x8ca1d, 0x3acf8, 0xd26ff, 0x9f55e, 0xac6de, 0xbfdee, 0x4b0ec, 0xf0390, 0x554eb, 0xd770,  0x9f0a5, 0x5e327, 0xc0445, 0xcf39a},
        {0x400a1, 0x34426, 0x87e3a, 0xb446,  0xabb63, 0xeeb51, 0x8cdc,  0xd2ec6, 0xc74af, 0x2e282, 0x9e333, 0x7c9da, 0x72124, 0xb192d, 0xe36ae, 0x81b8e, 0xd1a04, 0x394a0, 0x83289, 0x4d706, 0xb026,  0x4277f, 0xf03af, 0x2769b, 0x8cc9f, 0x379db, 0x1b8ee, 0xc12f0, 0x5848d, 0xd5cff, 0xafd34, 0xcef7b}

};


void genMovesMask() {
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

                            int type2 = type;
                            if (white && LAST_HORIZONTAL[to]) {
                                type2 = 1;
                            }
                            if (!white && FIRST_HORIZONTAL[to]) {
                                type2 = 3;
                            }
                            moveHash[move] = zorbistKeys[type][from] ^ zorbistKeys[type2][to];


                            bool nrotated = DIAGONAL[from] == DIAGONAL[to];
                            int f, t;
                            if (nrotated) {
                                f = from;
                                t = to;
                            } else {
                                f = rotatedFrom;
                                t = rotatedTo;
                            }
                            reversedDirection[move] = (nrotated ? 0 : 1) | ((t > f) ? 0b10 : 0);


                            if (!queen) {
                                if (white) {
                                    isPromotion[move] = (bool) LAST_HORIZONTAL[to];
                                } else {
                                    isPromotion[move] = (bool) FIRST_HORIZONTAL[to];
                                }
                            } else {
                                isPromotion[move] = false;
                            }
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
        int figureCell,
        bool q,
        bool rot,
        bool opp
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
                queen = q;
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
                            if (q && (opposite == opp) && (rotated == rot)) {
                                continue;
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
                            } else if (((isWhiteMove && LAST_HORIZONTAL[to]) ||
                                        (!isWhiteMove && FIRST_HORIZONTAL[to]))) {
                                if (*__getTakes(all, enemy, all90, enemy90, isWhiteMove, to, true,
                                                !rotated)) {
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
        _move previousMove
) {
    bool take;
    int figureCell;
    bool q;
    bool opp = false;
    bool rot;

    if (previousMove & MULTI_TAKE_FLAG) {
        take = true;
        figureCell = getTo(previousMove);
        _cboard i1 = wq | bq;
        _cboard i = i1 >> figureCell;
        q = i & 1;
        auto reversedBits = reversedDirection[previousMove];
        rot = reversedBits & 1;
        opp = reversedBits >> 1;
    } else {
        take = false;
        figureCell = -1;
        q = false;
        rot = false;
        opp = false;
    }
    if (isWhiteMove) {
        __getMoves(wc, bc, wq, bq, w90, b90, isWhiteMove, take, figureCell, q, rot, opp);
    } else {
        __getMoves(bc, wc, bq, wq, b90, w90, isWhiteMove, take, figureCell, q, rot, opp);
    }
}

inline void swap(_move &a, _move &b) {
    _move tmp = a;
    a = b;
    b = tmp;
}


/*
void qsort(int first, int last) {
    int mid;
    int f = first, l = last;
    mid = moveScore[(f + l) / 2];
    do {
        while (moveScore[f] < mid) f++;
        while (moveScore[l] > mid) l--;
        if (f <= l) 
        {
            swap(moveScore[f], moveScore[l]);
            swap(currentMoves[f], currentMoves[l]);
            f++;
            l--;
        }
    } while (f < l);
    if (first < l) qsort(first, l);
    if (f < last) qsort(f, last);
}*/

short moveScore[128];

void insertionSort(short *indexes, int size) {
    if (size < 2) return;
    if (size == 2) {
        if (moveScore[indexes[0]] > moveScore[indexes[1]]) {
            swap(indexes[0], indexes[1]);
        }
    } else {
        for (int j = 1; j < size; ++j) {
            auto key = indexes[j];
            auto keyScore = moveScore[key];
            auto i = j - 1;
            for (; i >= 0 && moveScore[indexes[i]] > keyScore; --i) {
                indexes[i + 1] = indexes[i];
            }
            indexes[i + 1] = key;
        }
    }
}

int history[2][32][32];

void sortMoves(_move hashedMove, _move killer1, _move killer2) {
    _move movesSize = *currentMoves;
    if (movesSize < 2)
        return;

    int negativesSize = 0;
    int zeroesSize = 0;
    static short negatives[128];
    static short zeroes[128];

    static int historyScores[128];
    int maxHistoryPoints = 0;

    for (short i = 1; i <= movesSize; ++i) {
        _move move = currentMoves[i];
        int points = history[getColor(move)][getFrom(move)][getTo(move)];
        if (points > maxHistoryPoints) {
            maxHistoryPoints = points;
        }
        historyScores[i] = points;
    }

    for (short i = 1; i <= movesSize; ++i) {
        _move move = currentMoves[i];
        short score = 0;

        if (maxHistoryPoints > 0) {
            score -= static_cast<short>(historyScores[i] * 128 / maxHistoryPoints);
        }

        if (move == killer1 || move == killer2) {
            score -= 130;
        }
        if (isPromotion[move]) {
            score -= 150;
        }
        if (move == hashedMove) {
            score -= 200;
        }


        moveScore[i] = score;
        if (score < 0) {
            negatives[negativesSize++] = i;
        } else {
            zeroes[zeroesSize++] = i;
        }
    }

    if (negativesSize == 0) {
        return;
    }
    insertionSort(negatives, negativesSize);
    static _move movesCopy[128];
    memcpy(movesCopy + 1, currentMoves + 1, movesSize * sizeof(_move));

    int index = 1;
    for (int j = 0; j < negativesSize; ++j) {
        currentMoves[index++] = movesCopy[negatives[j]];
    }
    for (int j = 0; j < zeroesSize; ++j) {
        currentMoves[index++] = movesCopy[zeroes[j]];
    }
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

int endgameSize = 0;
FILE *endgameData;
FILE *endgameIndex;

void prepareEndGame(string path) {
    for (int i = 2; i < 10; ++i) {
        FILE *index = fopen((path + "/endgame" + to_string(i) + ".index").c_str(), "r");
        FILE *data = fopen((path + "/endgame" + to_string(i) + ".data").c_str(), "r");
        if (index != nullptr && data != nullptr) {
            endgameSize = i;
            endgameData = data;
            endgameIndex = index;
            return;
        }
    }
    throw exception();
}


const short WHITE_CHECKER_POS[] = {
        0,
        1, 2, 3,
        4, 5, 6, 7, 8,
        9, 10, 11, 12, 13, 14, 15,
        16, 17, 18, 19, 20, 21, -1,
        22, 23, 24, 25, -1,
        26, 27, -1,
        -1
};

const short BLACK_CHECKER_POS[] = {
        -1,
        -1, 0, 1,
        -1, 2, 3, 4, 5,
        -1, 6, 7, 8, 9, 10, 11,
        12, 13, 14, 15, 16, 17, 18,
        19, 20, 21, 22, 23,
        24, 25, 26,
        27
};


// bq = 64
// wc = 8388608
// 1494
short getEndgame(_board wc, _board bc, _board wq, _board bq) {
    _ui index = (((bitCount(wc) * endgameSize) + bitCount(bc)) * endgameSize + bitCount(wq)) *
                endgameSize + bitCount(bq);
    fseek(endgameIndex, index * 4, SEEK_SET);
    int offset;
    fread(&offset, sizeof(int), 1, endgameIndex);
    int offset2 = 0;
    while (wc) {
        auto i = getLowestBit(wc);
        zeroLowestBitAssign(wc);
        offset2 = 28 * offset2 + WHITE_CHECKER_POS[i];
    }
    while (bc) {
        auto i = getLowestBit(bc);
        zeroLowestBitAssign(bc);
        offset2 = 28 * offset2 + BLACK_CHECKER_POS[i];
    }
    while (wq) {
        auto i = getLowestBit(wq);
        zeroLowestBitAssign(wq);
        offset2 = 32 * offset2 + i;
    }
    while (bq) {
        auto i = getLowestBit(bq);
        zeroLowestBitAssign(bq);
        offset2 = 32 * offset2 + i;
    }
    fseek(endgameData, offset + offset2, SEEK_SET);
    signed char score;
    fread(&score, 1, 1, endgameData);
    return score;
}

std::map<_ull, _move> *debuts = nullptr;

void prepareDebuts(string path) {
    FILE *file = fopen(path.c_str(), "r");
    if (file == nullptr) throw exception();
    fseek(file, 0, SEEK_END);
    _ui size = static_cast<_ui>(ftell(file) / 12);
    rewind(file);

    debuts = new map<_ull, _move>();
    _ui *wholeFile = new _ui[size * 3];
    fread(wholeFile, sizeof(_ui), size * 3, file);

    for (_ui *p = wholeFile, i = 0; i < size; i++, p += 3) {
        _ull pos = *((_ull *) p);
        int move = p[2];
        debuts->insert(make_pair(pos, move));
    }
    delete[] wholeFile;
    fclose(file);
}


_move getFromDebutBase(_board wc, _board bc, _board wq, _board bq) {
    if (debuts == nullptr) return 0;
    if ((wq | bq) != 0) return 0;
    _ull pos = 0ULL;
    pos |= bc;
    pos <<= 32;
    pos |= wc;
    int size = *currentMoves;

    auto find = debuts->find(pos);
    if (find == debuts->end()) {
        return 0;
    }
    _move move = find->second;

    for (int i = 1; i <= size; ++i) {
        if (currentMoves[i] == move)
            return move;
    }

    return 0;
}
/*
int main() {
    gen();
    prepareDebuts("C:/Projects/CheckersDesktop/filtered positions");
    getMoves(200319, -26165248, 0, 0, rotateBoard(200319), rotateBoard(-26165248), 1, 0);
    int move = getFromDebutBase(200319, -26165248, 0, 0);
    return move;
}*/

#pragma clang diagnostic pop