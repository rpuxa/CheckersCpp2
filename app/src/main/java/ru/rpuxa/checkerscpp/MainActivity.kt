package ru.rpuxa.checkerscpp

import android.os.Bundle
import androidx.appcompat.app.AppCompatActivity
import kotlinx.android.synthetic.main.activity_main.*
import ru.rpuxa.checkerscpp.game.Bot
import ru.rpuxa.checkerscpp.game.Game
import ru.rpuxa.checkerscpp.game.Human
import ru.rpuxa.checkerscpp.game.StupidBot
import ru.rpuxa.checkerscpp.game.board.*
import ru.rpuxa.checkerscpp.natives.Move
import ru.rpuxa.checkerscpp.natives.NativeEngine

class MainActivity : AppCompatActivity() {

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        val botIsWhite = intent?.getBooleanExtra("white", false)!!
        val diagonal = intent?.getBooleanExtra("diagonal", false)!!

        NativeEngine.prepareEngine()

        val wc = 198230
        val bc = -1000554488
        val wq = 0
        val bq = 0

        val human = Human(game_board)
        val pos = if (diagonal) Position.createDiagonalPosition() else Position.createStartPosition()

//        val pos = Position.createFromNative(wc, bc, wq, bq)


//        pos.setFromNative(intArrayOf(wc, bc, wq.toInt(), bq))
//        pos.setFromNative(intArrayOf(wc, bc.toInt(), wq, bq))
/*        pos.board[1][1] = WhiteChecker
        pos.board[2][4] = BlackChecker
        pos.board[3][5] = BlackChecker
        pos.board[5][5] = BlackChecker*/
/*
        pos.board[0][6] = NullFigure
        pos.board[2][6] = NullFigure
        pos.board[3][5] = NullFigure
        pos.board[5][7] = NullFigure
        pos.board[2][2] = BlackChecker
        pos.board[3][1] = NullFigure
        pos.board[4][2] = NullFigure
        pos.board[5][1] = NullFigure
        pos.board[6][2] = NullFigure
*/

/*
1388
2176
540

70174
3189768448
0
0
1089607
890112000
isWhiteMove = true




540

 */
        Game(
            pos,
            true,
            if (botIsWhite) Bot else human,
            if (!botIsWhite) Bot else human,
            null
        ).start()

    }

}
