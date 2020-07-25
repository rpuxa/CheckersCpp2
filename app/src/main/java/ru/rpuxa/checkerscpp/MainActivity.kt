package ru.rpuxa.checkerscpp

import android.os.Bundle
import androidx.appcompat.app.AppCompatActivity
import kotlinx.android.synthetic.main.activity_main.*
import ru.rpuxa.checkerscpp.game.Bot
import ru.rpuxa.checkerscpp.game.Game
import ru.rpuxa.checkerscpp.game.Human
import ru.rpuxa.checkerscpp.game.board.*
import ru.rpuxa.checkerscpp.natives.Move
import ru.rpuxa.checkerscpp.natives.NativeEngine
import ru.rpuxa.checkerscpp.natives.NativeMethods

class MainActivity : AppCompatActivity() {

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        val botIsWhite = intent?.getBooleanExtra("white", false)!!
        val diagonal = intent?.getBooleanExtra("diagonal", false)!!

        NativeEngine.prepareEngine()
        NativeMethods.prepareEndGame(cacheDir.toString())

        val human = Human(game_board)
        val pos  = if (diagonal) Position.createDiagonalPosition() else Position.createStartPosition()

        Game(
            pos,
            true,
            if (botIsWhite) Bot else human,
            if (!botIsWhite) Bot else human,
            null
        ).start()

    }

}
