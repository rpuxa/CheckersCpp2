package ru.rpuxa.checkerscpp.natives

import ru.rpuxa.checkerscpp.game.board.Cell
import ru.rpuxa.checkerscpp.game.board.Position
import ru.rpuxa.checkerscpp.game.board.x
import kotlin.experimental.inv

class Move(val from: Cell, val to: Cell, val isTaking: Boolean, val multiTake: Boolean) {

    fun toNative(position: Position): Int {
        return (Position.BITS[from.x][from.y] shl 1) or
                (Position.BITS[to.x][to.y] shl 6) or
                ((if (position.board[from.x][from.y].isWhite) 1 else 0) shl 11) or
                ((if (position.board[from.x][from.y].isQueen) 1 else 0) shl 12)
    }

    override fun toString(): String {
        fun letter(i: Int) = 'a' + i
        fun digit(i: Int) = '1' + i

        return "${letter(from.x)}${digit(from.y)}x${letter(to.x)}${digit(to.y)}"
    }

    companion object {

        fun fromNative(move: Int): Move {
            val fromInt = getFrom(move)
            val toInt = getTo(move)
            return Move(
                X[fromInt] x Y[fromInt],
                X[toInt] x Y[toInt],
                move checkBit 0,
                move checkBit 13
            )
        }

        val X = arrayOf(
            0,
            2, 1, 0,
            4, 3, 2, 1, 0,
            6, 5, 4, 3, 2, 1, 0,
            7, 6, 5, 4, 3, 2, 1,
            7, 6, 5, 4, 3,
            7, 6, 5,
            7
        )

        val Y = arrayOf(
            0,
            0, 1, 2,
            0, 1, 2, 3, 4,
            0, 1, 2, 3, 4, 5, 6,
            1, 2, 3, 4, 5, 6, 7,
            3, 4, 5, 6, 7,
            5, 6, 7,
            7
        )

        private fun getFrom(move: Int) =
            (move ushr 1) and 0b11_111

        private fun getTo(move: Int) =
            (move ushr 6) and 0b11_111

    }
}
