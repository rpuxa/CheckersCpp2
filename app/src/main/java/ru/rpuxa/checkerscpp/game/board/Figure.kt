package ru.rpuxa.checkerscpp.game.board

sealed class Figure {
    val isQueen get() = this === WhiteQueen || this === BlackQueen
    val isWhite get() = this === WhiteChecker || this === WhiteQueen
}

object WhiteChecker : Figure()

object BlackChecker : Figure()

object WhiteQueen : Figure()

object BlackQueen : Figure()

object NullFigure : Figure()