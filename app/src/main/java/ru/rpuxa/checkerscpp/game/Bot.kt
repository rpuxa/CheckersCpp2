package ru.rpuxa.checkerscpp.game

import android.util.Log
import kotlinx.coroutines.GlobalScope
import kotlinx.coroutines.delay
import kotlinx.coroutines.launch
import ru.rpuxa.checkerscpp.game.board.Cell
import ru.rpuxa.checkerscpp.natives.Move
import ru.rpuxa.checkerscpp.natives.NativeEngine
import ru.rpuxa.checkerscpp.natives.NativeMethods

object Bot : Player {


    override fun onAction(action: Action) {

    }

    override suspend fun onMove(game: Game, previousMove: Move?): Move {
        val startTime = System.currentTimeMillis()

        val moves = NativeEngine.getAllMoves(game.position, game.isTurnWhite, previousMove)
        if (moves.size == 1) {
            return moves.first()
        }

        val delay =
            when {
                moves.size == 2 -> 3000
                moves.size > 7 -> 9000
                else -> 5000
            }

        val job = GlobalScope.launch {
            delay(delay.toLong())
            NativeMethods.stopSearching()
        }

        var lastMove: Move? = null

        var depth = 6
        while (true) {
            val (move, score) = NativeEngine.getBestMove(
                game.position,
                previousMove,
                game.isTurnWhite,
                depth
            )
            if (score == -320.0)
                break
            val time = System.currentTimeMillis() - startTime
            Log.d(
                "Checkers Bot",
                "Score: $score Depth: $depth Time: $time  Line: ${move.joinToString(", ") {
                    it.toNative(game.position).toString()
                }}"
            )
            lastMove = move.first()
            if (time > 0.2 * delay)
                break
            if (game.isTurnWhite) {
                if (score > 200) {
                    break
                }
            } else {
                if (score < -200) {
                    break
                }
            }
            if (depth >= 32) break
            depth++
        }
        job.cancel()
        return lastMove ?: error("Не успели подсчитать")
    }

}