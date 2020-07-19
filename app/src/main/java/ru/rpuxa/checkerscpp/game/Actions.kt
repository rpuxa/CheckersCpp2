package ru.rpuxa.checkerscpp.game

import ru.rpuxa.checkerscpp.game.board.Position
import ru.rpuxa.checkerscpp.natives.Move


sealed class Action


class GameBegin(val position: Position, val sideIsWhite: Boolean) : Action()
class MakeMove(val position: Position, val move: Move) : Action()
object GameEnd : Action()