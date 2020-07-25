//
// Created by Григорий on 06.12.2018.
//

#ifndef CHECKERSCPP_ENGINE_H
#define CHECKERSCPP_ENGINE_H

#endif //CHECKERSCPP_ENGINE_H

#include "types.h"

using namespace types;

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
);

void stopSearching();
