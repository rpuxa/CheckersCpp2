package ru.rpuxa.checkerscpp.game.board

import ru.rpuxa.checkerscpp.natives.*
import ru.rpuxa.checkerscpp.natives.Move.Companion.X
import ru.rpuxa.checkerscpp.natives.Move.Companion.Y

class Position {
    val board: Array<Array<Figure>> =
        Array(BOARD_SIZE) {
            Array<Figure>(
                BOARD_SIZE
            ) { NullFigure }
        }

    fun clone(): Position {
        val position = Position()
        for (x in BOARD_RANGE)
            for (y in BOARD_RANGE) {
                position.board[x][y] = board[x][y]
            }
        return position
    }

    operator fun set(c: Char, n: Int, f: Figure) {
        board[c - 'a'][n - 1] = f
    }


    fun toNative(): IntArray {
        var whiteCheckers = 0
        var blackCheckers = 0
        var blackQueens = 0
        var whiteQueens = 0

        for (i in BOARD_RANGE)
            for (j in BOARD_RANGE) {
                val bit = BITS[i][j]
                when (board[i][j]) {
                    WhiteChecker -> whiteCheckers = whiteCheckers setBit bit
                    BlackChecker -> blackCheckers = blackCheckers setBit bit
                    WhiteQueen -> whiteQueens = whiteQueens setBit bit
                    BlackQueen -> blackQueens = blackQueens setBit bit
                }
            }

        return intArrayOf(whiteCheckers, blackCheckers, whiteQueens, blackQueens)
    }

    fun makeMove(move: Move) {
        val (whiteCheckers, blackCheckers, whiteQueens, blackQueens) = toNative()
        val nativeMove = move.toNative(this)
        val pos = IntArray(4)
        NativeMethods.makeMove(
            whiteCheckers,
            blackCheckers,
            whiteQueens,
            blackQueens,
            nativeMove.toShort(),
            pos
        )

        setFromNative(pos)
    }

    fun setFromNative(figures: IntArray) {
        for (bit in 0 until 32) {
            for (type in 0 until 4) {
                val b = figures[type] checkBit bit
                board[X[bit]][Y[bit]] =
                    if (b) {
                        when (type) {
                            0 -> WhiteChecker
                            1 -> BlackChecker
                            2 -> WhiteQueen
                            3 -> BlackQueen
                            else -> throw IllegalStateException()
                        }
                    } else {
                        NullFigure
                    }
                if (b)
                    break
            }
        }
    }

    companion object {
        const val BOARD_SIZE = 8

        @JvmField
        val BOARD_RANGE = 0 until BOARD_SIZE

        fun createStartPosition(): Position {
            val position = Position()
            for (x in BOARD_RANGE)
                for (y in BOARD_RANGE)
                    if ((x + y) % 2 == 0)
                        position.board[x][y] = when (y) {
                            in 0..2 -> WhiteChecker
                            in 5..7 -> BlackChecker
                            else -> NullFigure
                        }

            return position
        }

        fun createDiagonalPosition(): Position {
            val position = Position()
            for (x in BOARD_RANGE)
                for (y in BOARD_RANGE) {
                    if ((x + y) % 2 == 0 && x != y) {
                        position.board[x][y] =
                            if (y > x) {
                                BlackChecker
                            } else {
                                WhiteChecker
                            }
                    }
                }
            return position
        }

        fun createFromNative(vararg figures: Int): Position {
            val pos = Position()
            pos.setFromNative(figures)
            return pos
        }

        val BITS = arrayOf(
            arrayOf(0, -1, 3, -1, 8, -1, 15, -1),
            arrayOf(-1, 2, -1, 7, -1, 14, -1, 22),
            arrayOf(1, -1, 6, -1, 13, -1, 21, -1),
            arrayOf(-1, 5, -1, 12, -1, 20, -1, 27),
            arrayOf(4, -1, 11, -1, 19, -1, 26, -1),
            arrayOf(-1, 10, -1, 18, -1, 25, -1, 30),
            arrayOf(9, -1, 17, -1, 24, -1, 29, -1),
            arrayOf(-1, 16, -1, 23, -1, 28, -1, 31)
        )
    }

}