package ru.rpuxa.checkerscpp.game

import kotlinx.coroutines.GlobalScope
import kotlinx.coroutines.launch
import ru.rpuxa.checkerscpp.game.board.Cell
import ru.rpuxa.checkerscpp.natives.Move
import ru.rpuxa.checkerscpp.natives.NativeEngine
import ru.rpuxa.checkerscpp.views.BoardView
import kotlin.coroutines.resume
import kotlin.coroutines.suspendCoroutine

class Human(private val view: BoardView) : Player {

    private var previousMove: Move? = null

    init {
        view.setMovesUpdater { position, cell ->
            GlobalScope.launch {
                val moves = NativeEngine.getFigureMoves(position, cell, previousMove)
                view.setMoves(moves)
            }
        }
    }

    override fun onAction(action: Action) {
        when (action) {
            is GameBegin -> {
                view.setPosition(action.position)
                view.setSide(action.sideIsWhite)
            }
            is MakeMove -> view.setPosition(action.position)
        }
    }

    override suspend fun onMove(game: Game, previousMove: Move?): Move {
        this.previousMove = previousMove
        view.multiTake = previousMove != null
        if (previousMove != null) {
            view.setMoves(NativeEngine.getFigureMoves(game.position, previousMove.to, previousMove))
        }
        val result = suspendCoroutine<Move> { cont ->
            view.onMove = {
                cont.resume(it)
            }
        }
        view.onMove = null
        view.clearMoves()
        return result
    }
}