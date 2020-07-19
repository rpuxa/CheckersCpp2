//
// Created by Григорий on 01.12.2018.
//

#ifndef CHECKERSCPP_BITUTILS_H
#define CHECKERSCPP_BITUTILS_H

#endif //CHECKERSCPP_BITUTILS_H

#include "types.h"

using namespace types;

_ui getBit(_ci number, _ci position);

_ui setBit(_ci number, _ci position);

void setBitAssign(_ui &number, _ci position);

_ui getLowestBit(_ci number);

_ui powerLowestBit(_ci number);

_ui zeroLowestBit(_ci number);

void zeroLowestBitAssign(_ui &number);

_ui bitCount(_ui number);