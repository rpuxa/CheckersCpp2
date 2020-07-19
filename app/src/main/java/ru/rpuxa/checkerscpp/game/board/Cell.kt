package ru.rpuxa.checkerscpp.game.board

data class Cell(val x: Int, val y: Int)

infix fun Int.x(other: Int) = Cell(this, other)