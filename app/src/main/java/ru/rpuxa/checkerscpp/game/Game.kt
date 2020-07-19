package ru.rpuxa.checkerscpp.game

import android.util.Log
import kotlinx.coroutines.*
import ru.rpuxa.checkerscpp.game.board.Cell
import ru.rpuxa.checkerscpp.game.board.Position
import ru.rpuxa.checkerscpp.natives.NativeEngine

class Game(
    val position: Position,
    isTurnWhite: Boolean,
    private val whitePlayer: Player,
    private val blackPlayer: Player,
    private val observer: Player?
) {
    var movesCount = 0
    var isTurnWhite: Boolean = isTurnWhite
        private set

    private val gameScope = CoroutineScope(Job() + Dispatchers.IO)


    fun start() {
        gameScope.launch {
            whitePlayer.onAction(GameBegin(position, true))
            blackPlayer.onAction(GameBegin(position, false))
            observer?.onAction(GameBegin(position, true))

            var multiTake: Cell? = null
            while (isActive) {
                val player = if (isTurnWhite) whitePlayer else blackPlayer
                val otherPlayer = if (!isTurnWhite) whitePlayer else blackPlayer

                val moves = NativeEngine.getAllMoves(position, isTurnWhite)

                if (moves.isEmpty()) {
                    break
                }

                Log.d("MyGame", "Waiting for move $player")
                val move = player.onMove(this@Game, multiTake)
                movesCount++
                Log.d("MyGame", "Move is $move")
                position.makeMove(move)
                Log.d("MyGame", "Move made")
                val action = MakeMove(position, move)
                otherPlayer.onAction(action)
                observer?.onAction(action)

                if (!move.multiTake) {
                    isTurnWhite = !isTurnWhite
                    multiTake = null
                } else {
                    multiTake = move.to
                }
                if (movesCount > 100) break
                Log.d("MyGame", "Next move...")
            }

            whitePlayer.onAction(GameEnd)
            blackPlayer.onAction(GameEnd)
            observer?.onAction(GameEnd)
        }
    }
}