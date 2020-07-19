//
// Created by Григорий on 01.12.2018.
//

#ifndef CHECKERSCPP_BOARD_H
#define CHECKERSCPP_BOARD_H

#endif //CHECKERSCPP_BOARD_H

#include "types.h"
#define HASH_BITS_COUNT 20
#define HASH_SIZE (1 << HASH_BITS_COUNT)
#define HASH_MASK ((1 << HASH_BITS_COUNT) - 1)

using namespace types;

void gen();

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
);

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
);

_board rotateBoard(_board board);

extern _move currentMoves[256];
