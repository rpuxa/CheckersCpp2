package ru.rpuxa.checkerscpp.natives

import kotlinx.coroutines.*
import kotlinx.coroutines.channels.Channel
import ru.rpuxa.checkerscpp.game.board.*
import ru.rpuxa.checkerscpp.game.board.Position.Companion.BITS
import ru.rpuxa.checkerscpp.game.board.Position.Companion.BOARD_RANGE
import java.util.*
import kotlin.collections.ArrayList
import kotlin.math.sign


object NativeEngine {

    private const val MAX_MOVES_COUNT = 200

    private val executor = newFixedThreadPoolContext(1, "Engine Thread")

    fun prepareEngine() {
        NativeMethods.prepareEngine()
    }

/*    fun isTaking(position: Position, move: Move): Boolean {
        val dx = (move.to.x - move.from.x).sign
        val dy = (move.to.y - move.from.y).sign
        var x = move.from.x + dx
        var y = move.from.y + dy
        while (x != move.to.x && y != move.to.y) {
            if (position.board[x][y] != NullFigure) return true
            x += dx
            y += dy
        }
        return false
    }*/

    suspend fun getFigureMoves(position: Position, cell: Cell, previousMove: Move?) =
        getAllMoves(position, position.board[cell.x][cell.y].isWhite, previousMove)
            .filter { it.from == cell }

    fun getAllMoves(position: Position, white: Boolean, previousMove: Move?): List<Move> {
        val (whiteCheckers, blackCheckers, whiteQueens, blackQueens) =
            position.toNative()

        val array = ShortArray(MAX_MOVES_COUNT)
//        withContext(executor) {
        NativeMethods.getAvailableMoves(
            whiteCheckers,
            blackCheckers,
            whiteQueens,
            blackQueens,
            white,
            previousMove?.native ?: 0,
            array
        )
//        }

        val list = ArrayList<Move>()
        for (move in array) {
            if (move == END_MOVES_FLAG)
                break
            list.add(Move.fromNative(move.toInt()))
        }

        return list
    }

    fun getBestMove(
        position: Position,
        previousMove: Move?,
        isTurnWhite: Boolean,
        depth: Int
    ): Pair<List<Move>, Double> {
        val (whiteCheckers, blackCheckers, whiteQueens, blackQueens) =
            position.toNative()


        val array = ShortArray(MAX_MOVES_COUNT)
        NativeMethods.getBestMove(
            whiteCheckers,
            blackCheckers,
            whiteQueens,
            blackQueens,
            previousMove?.native ?: 0,
            isTurnWhite,
            depth.toShort(),
            array
        )

        val score = array[0]

        val list = ArrayList<Move>()
        var i = 1
        while (true) {
            val move = array[i++]
            if (move == END_MOVES_FLAG)
                break
            list += Move.fromNative(move.toInt())
        }

        return list to score / 100.0
    }



    const val END_MOVES_FLAG: Short = -1
}