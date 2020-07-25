package ru.rpuxa.checkerscpp.natives


infix fun Int.checkBit(to: Int) = ((this shr to) and 1) != 0

infix fun Int.setBit(to: Int) = this or (1 shl to)

fun Int.bitCount() = java.lang.Integer.bitCount(this)