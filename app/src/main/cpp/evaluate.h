//
// Created by Григорий on 02.12.2018.
//

#ifndef CHECKERSCPP_EVALUATE_H
#define CHECKERSCPP_EVALUATE_H

static const int EMPTY_FLAG_SCORE = 10;
#endif //CHECKERSCPP_EVALUATE_H

#include "types.h"

using namespace types;

short eval(
        _board our,
        _board enemy,
        _board our_q,
        _board enemy_q,
        bool isTurnWhite
);