package ru.rpuxa.checkerscpp.game

import ru.rpuxa.checkerscpp.game.board.Cell
import ru.rpuxa.checkerscpp.natives.Move

interface Player {

    fun onAction(action: Action)

    suspend fun onMove(game: Game, previousMove: Move?): Move
}

