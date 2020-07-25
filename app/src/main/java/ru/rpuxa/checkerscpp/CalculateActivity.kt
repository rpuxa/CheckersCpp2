package ru.rpuxa.checkerscpp

import android.content.Intent
import android.os.Bundle
import android.view.View
import androidx.appcompat.app.AppCompatActivity
import kotlinx.android.synthetic.main.activity_calculate.*
import ru.rpuxa.checkerscpp.game.board.*
import ru.rpuxa.checkerscpp.natives.*
import java.io.DataOutputStream
import java.io.File
import java.io.FileOutputStream
import kotlin.concurrent.thread

val ALL = arrayOf(NullFigure, WhiteChecker, BlackChecker, WhiteQueen, BlackQueen)


class CalculateActivity : AppCompatActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_calculate)

/*
        startActivity(Intent(this, MainActivity::class.java).apply {
            putExtra("white", false)
            putExtra("diagonal", false)
        })
        return*/
        thread {
            generate()
            runOnUiThread {
                textView.visibility = View.GONE
            }
        }
    }

    fun generate() {
        NativeEngine.prepareEngine()
//        figures(2).forEach {  }

        val N = 3

        iterateAllPositions(N).forEach { (position, _) ->

            if (position != null) {
                using.clear()
                fakeDraws.clear()
                val result = search(position, 0)
                println(result)
                if (position in fakeDraws) {
                    fakeDraws.forEach {
                        positions[it] = 0
                    }
                }
            }
        }


        val folder = cacheDir
        val size = N * N * N * N
        val used = BooleanArray(size)
        val offsets = IntArray(size)

        DataOutputStream(FileOutputStream(File(folder, "endgame$N.data"))).use { data ->
            var i = 0
            iterateAllPositions(N).forEach { (position, counts) ->
                if (position?.bq == 64 && position?.wc == 8388608) {
                    println()
                }
                val index = (((counts[0] * N) + counts[1]) * N + counts[2]) * N + counts[3]
                if (!used[index]) {
                    offsets[index] = data.size()
                    used[index] = true
                }
                if (position == null) {
                    data.writeByte(0)
                } else {
                    var result = positions[position]!!
                    when {
                        result > 10_000 -> {
                            result = 30_000 - result + 1
                        }
                        result < -10_000 -> {
                            result = -30_000 - result - 1
                        }
                        result != 0 -> error("Wrong result: $result")
                    }
                    require(result in -127..127) {
                        println(result)
                    }
                    data.writeByte(result)
                }
                i++
            }
        }
        DataOutputStream(FileOutputStream(File(folder, "endgame$N.index"))).use { ind ->
            offsets.forEach {
                ind.writeByte(it ushr 0 and 0xFF)
                ind.writeByte(it ushr 8 and 0xFF)
                ind.writeByte(it ushr 16 and 0xFF)
                ind.writeByte(it ushr 24 and 0xFF)
            }
        }
    }

    fun search(pos: NativePos, lastMove: Int): Int {
        positions[pos]?.let {
            return it
        }

        val movesArray = ShortArray(128)


        NativeMethods.getAvailableMoves(
            pos.wc,
            pos.bc,
            pos.wq,
            pos.bq,
            pos.turn,
            lastMove.toShort(),
            movesArray
        )

        if (movesArray[0] == NativeEngine.END_MOVES_FLAG) {
            val score = if (pos.turn) -30_000 else 30_000
            positions[pos] = score
            return score
        }

        try {
            using.add(pos)
        } catch (e: StackOverflowError) {
            println()
        }
        var i = 0
        var bestResult = if (pos.turn) Int.MIN_VALUE else Int.MAX_VALUE
        var fakeDraw = false
        var increaseDepth = false
        while (movesArray[i] != NativeEngine.END_MOVES_FLAG) {
            val move = movesArray[i]
            NativeMethods.makeMove(pos.wc, pos.bc, pos.wq, pos.bq, move, newPos)
            val multitake = move.toInt() checkBit 13
            val nextPos = NativePos(
                newPos[0],
                newPos[1],
                newPos[2],
                newPos[3],
                if (multitake) pos.turn else !pos.turn
            )
            val fake: Boolean
            val result: Int
            if (nextPos in using) {
                fake = true
                result = 0
            } else {
                result = search(
                    nextPos,
                    move.toInt()
                )
                fake = nextPos in fakeDraws
            }
            if (pos.turn) {
                if (result > bestResult) {
                    bestResult = result
                    fakeDraw = fake
                    increaseDepth = !multitake
                }
            } else {
                if (result < bestResult) {
                    bestResult = result
                    fakeDraw = fake
                    increaseDepth = !multitake
                }
            }
            i++
        }
        if (fakeDraw) {
            fakeDraws.add(pos)
        } else {
            if (bestResult != 0 && increaseDepth) {
                if (bestResult < 0) {
                    bestResult++
                } else {
                    bestResult--
                }
            }
            positions[pos] = bestResult
        }
        return bestResult
    }


    fun iterateAllPositions(figuresCount: Int) = iterator {
        figures(figuresCount).forEach { figures ->
            val sizes = figures.map { figure ->
                when (figure) {
                    NullFigure -> 1
                    WhiteChecker -> 28
                    BlackChecker -> 28
                    WhiteQueen -> 32
                    BlackQueen -> 32
                }
            }

            val counts = arrayOf(WhiteChecker, BlackChecker, WhiteQueen, BlackQueen).map { type ->
                figures.count { it == type }
            }

            iterator(sizes).forEach l@{ indieces ->
                var wc = 0
                var bc = 0
                var wq = 0
                var bq = 0

                loop@ for (i in sizes.indices) {
                    val pos = when (figures[i]) {
                        WhiteChecker -> WHITE_CHECKER_BIT[indieces[i]]
                        BlackChecker -> BLACK_CHECKER_BIT[indieces[i]]
                        WhiteQueen -> indieces[i]
                        BlackQueen -> indieces[i]
                        NullFigure -> continue@loop
                    }
                    if ((wc or bc or wq or bq) checkBit pos) {
                        yield(null to counts)
                        return@l
                    }
                    when (figures[i]) {
                        WhiteChecker -> wc = wc setBit pos
                        BlackChecker -> bc = bc setBit pos
                        WhiteQueen -> wq = wq setBit pos
                        BlackQueen -> bq = bq setBit pos
                    }
                }

                val position = NativePos(wc, bc, wq, bq, true)
                yield(position to counts)
            }
        }
    }

    val WHITE_CHECKER_BIT = arrayOf(
        0,
        1, 2, 3,
        4, 5, 6, 7, 8,
        9, 10, 11, 12, 13, 14, 15,
        16, 17, 18, 19, 20, 21,
        23, 24, 25, 26,
        28, 29
    )

    val BLACK_CHECKER_BIT = arrayOf(
        2, 3,
        5, 6, 7, 8,
        10, 11, 12, 13, 14, 15,
        16, 17, 18, 19, 20, 21, 22,
        23, 24, 25, 26, 27,
        28, 29, 30, 31
    )

    fun figures(count: Int) = iterator<Array<Figure>> {
        recursive(count, arrayOfNulls<Figure>(count) as Array<Figure>, 0)
    }

    suspend fun SequenceScope<Array<Figure>>.recursive(
        count: Int,
        current: Array<Figure>,
        lastInd: Int
    ) {
        if (count == 0) {
            val color = current.firstOrNull { it != NullFigure }?.isWhite ?: return
            current.forEach {
                if (it != NullFigure && it.isWhite != color) {
                    yield(current.reversedArray())
                    return
                }
            }
            return
        }
        for (it in lastInd until ALL.size) {
            val figure = ALL[it]
            current[count - 1] = figure
            recursive(count - 1, current, it)
        }
    }

    fun iterator(sizes: List<Int>) = iterator<IntArray> {
        recursive2(sizes.size, sizes, IntArray(sizes.size))
    }

    suspend fun SequenceScope<IntArray>.recursive2(
        count: Int,
        sizes: List<Int>,
        current: IntArray
    ) {
        if (count == 0) {
            yield(current)
            return
        }
        for (it in 0 until sizes[sizes.size - count]) {
            current[sizes.size - count] = it
            recursive2(count - 1, sizes, current)
        }
    }

    data class NativePos(
        var wc: Int,
        var bc: Int,
        var wq: Int,
        var bq: Int,
        var turn: Boolean
    )


    val positions = HashMap<NativePos, Int>()
    val newPos = IntArray(4)
    val using = HashSet<NativePos>()
    val fakeDraws = HashSet<NativePos>()

}