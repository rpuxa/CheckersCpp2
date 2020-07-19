package ru.rpuxa.checkerscpp.game

import ru.rpuxa.checkerscpp.game.board.Cell
import ru.rpuxa.checkerscpp.natives.Move
import ru.rpuxa.checkerscpp.natives.NativeEngine

object StupidBot : Player {
    override fun onAction(action: Action) {
    }

    override suspend fun onMove(game: Game, multiTake: Cell?): Move {
       return if (multiTake == null) {
           NativeEngine.getAllMoves(game.position, game.isTurnWhite).first()
       } else {
           NativeEngine.getFigureMoves(game.position, multiTake).first()
       }
    }
}