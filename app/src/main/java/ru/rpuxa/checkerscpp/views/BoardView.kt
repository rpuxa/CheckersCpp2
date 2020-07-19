@file:Suppress("INTEGER_OVERFLOW")

package ru.rpuxa.checkerscpp.views

import android.content.Context
import android.graphics.*
import android.util.AttributeSet
import android.view.MotionEvent
import android.view.View
import ru.rpuxa.checkerscpp.game.board.*
import ru.rpuxa.checkerscpp.game.board.Position.Companion.BOARD_RANGE
import ru.rpuxa.checkerscpp.R
import ru.rpuxa.checkerscpp.natives.Move
import kotlin.math.min

private const val BLACK_CELLS_COLOR = 0xb48861 or (0xff00_00_0 * 16)
private const val WHITE_CELLS_COLOR = 0xefd9b4 or (0xff00_00_0 * 16)


class BoardView(context: Context, attrs: AttributeSet) : View(context, attrs) {

    var multiTake = false
    private var position: Position? = null
    private var sideIsWhite = true
    private var moves = emptyList<Move>()
    private lateinit var movesUpdater: (Position, Cell) -> Unit
    var onMove: ((Move) -> Unit)? = null

    fun setSide(sideIsWhite: Boolean) {
        this.sideIsWhite = sideIsWhite
        invalidate()
    }

    fun setPosition(position: Position) {
        this.position = position
        invalidate()
    }

    fun setMoves(list: List<Move>) {
        moves = list
        invalidate()
    }

    fun setMovesUpdater(movesUpdater: (Position, Cell) -> Unit) {
        this.movesUpdater = movesUpdater
    }

    fun clearMoves() = setMoves(emptyList())

    override fun onMeasure(widthMeasureSpec: Int, heightMeasureSpec: Int) {
        val min = min(widthMeasureSpec, heightMeasureSpec)
        setMeasuredDimension(min, min)
    }

    private val paint = Paint()

    override fun onDraw(canvas: Canvas) {
        val cellSize = width / 8f

        for (x in BOARD_RANGE)
            for (y in BOARD_RANGE) {
                paint.color = if ((x + y) % 2 == 0) WHITE_CELLS_COLOR else BLACK_CELLS_COLOR
                val canvasX1 = x * cellSize
                val canvasY1 = y * cellSize
                val canvasX2 = canvasX1 + cellSize
                val canvasY2 = canvasY1 + cellSize

                canvas.drawRect(canvasX1, canvasY1, canvasX2, canvasY2, paint)
            }

        paint.color = Color.GREEN

        for (move in moves) {
            val canvasX1 = move.to.x * cellSize + cellSize / 2
            val canvasY1 = (7 - move.to.y) * cellSize + cellSize / 2

            canvas.drawCircle(canvasX1, canvasY1, cellSize / 8, paint)
        }

        val checkerSize = 3 * cellSize / 4

        paint.color = Color.WHITE

        val position = this.position
        if (position != null)
            for (x in BOARD_RANGE)
                for (y in BOARD_RANGE) {
                    val bitmap = getFigureBitmap(position.board[x][y], checkerSize) ?: continue
                    val left = x * cellSize + (cellSize - checkerSize) / 2
                    val top = (7 - y) * cellSize + (cellSize - checkerSize) / 2

                    canvas.drawBitmap(bitmap, left, top, paint)
                }
    }

    private var lastCellSize = -1
    private val figuresBitmap = HashMap<Figure, Bitmap>()

    private fun getFigureBitmap(figure: Figure, size: Float): Bitmap? {
        val intSize = size.toInt()
        if (lastCellSize != intSize) {
            figuresBitmap[WhiteChecker] = Bitmap.createScaledBitmap(
                BitmapFactory.decodeResource(resources, R.drawable.white_checker),
                intSize,
                intSize,
                false
            )

            figuresBitmap[BlackChecker] = Bitmap.createScaledBitmap(
                BitmapFactory.decodeResource(resources, R.drawable.black_checker),
                intSize,
                intSize,
                false
            )

            figuresBitmap[WhiteQueen] = Bitmap.createScaledBitmap(
                BitmapFactory.decodeResource(resources, R.drawable.white_queen),
                intSize,
                intSize,
                false
            )

            figuresBitmap[BlackQueen] = Bitmap.createScaledBitmap(
                BitmapFactory.decodeResource(resources, R.drawable.black_queen),
                intSize,
                intSize,
                false
            )
        }
        lastCellSize = intSize
        return figuresBitmap[figure]
    }

    override fun onTouchEvent(event: MotionEvent): Boolean {
        onMove?.let { onMove ->
            val cellSize = width / 8f
            if (event.action == MotionEvent.ACTION_DOWN) {
                val x = (event.x / cellSize).toInt()
                val y = 7 - (event.y / cellSize).toInt()

                if ((x + y) % 2 == 0) {
                    val move = moves.find { it.to.x == x && it.to.y == y }
                    val correctFigure = position!!.board[x][y].isWhite == sideIsWhite
                    when {
                        move != null -> onMove(move)
                        correctFigure && !multiTake -> movesUpdater(position!!, Cell(x, y))
                        !multiTake -> clearMoves()
                    }

                }
            }
        }
        return true
    }
}