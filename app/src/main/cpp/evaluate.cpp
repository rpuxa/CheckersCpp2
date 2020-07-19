//
// Created by Григорий on 02.12.2018.
//

#include "types.h"
#include "bitutils.h"

#define cell(i) (1 << (i))
#define atCell(board, i) ((board) & (1 << (i)))

const short CHECKER_SCORE = 100;
const short QUEEN_SCORE_ENDGAME = 300;
const short QUEEN_SCORE = 250;
const short DIFFERENCE_FLANK = 2;


const _ui BIG_WAY_MASK = 0b10100010000010000001000001000101;
const _ui RIGHT_FLANK_MASK = 0b11110011100001110000011000000000;
const _ui LEFT_FLANK_MASK = 0b11000001110000111001111;


const _ui RIGHT_CORNER = 0b10100000000000000000000000000000;
const _ui LEFT_CORNER = 0b101;

const short BACK_RANK_VALUES[32] = {
        0, -1, 1, 0, 3, 3, 3, 3, 2, 2, 2, 2, 4, 4, 8, 7, 1, 0, 1, 0, 3, 3, 3, 3, 2,
        2, 2, 2, 4, 4, 8, 7
};


inline short endGameEval(
        _board wc,
        _board bc,
        _board wq,
        _board bq
) {
    _ui whiteCheckersCount = bitCount(wc);
    _ui whiteQueensCount = bitCount(wq);
    _ui blackCheckersCount = bitCount(bc);
    _ui blackQueensCount = bitCount(bq);

    short score = CHECKER_SCORE * (blackCheckersCount - whiteCheckersCount) + QUEEN_SCORE_ENDGAME * (blackQueensCount - whiteQueensCount);

    _ui whiteCount = whiteCheckersCount + whiteQueensCount;
    _ui blackCount = blackCheckersCount + blackQueensCount;
    if (blackQueensCount > 0 && whiteCount < blackQueensCount + 2 && score < 0) return 0;
    if (whiteQueensCount > 0 && blackCount < whiteQueensCount + 2 && score > 0) return 0;


    bool whiteOnBigWay;
    bool blackOnBigWay;
    if ((wc | bc) & BIG_WAY_MASK) {
        whiteOnBigWay = false;
        blackOnBigWay = false;
    } else {
        whiteOnBigWay = wq & BIG_WAY_MASK;
        blackOnBigWay = bq & BIG_WAY_MASK;
    }


    if (whiteCount == 1 && whiteCheckersCount == 1 && blackCount > 3) score = score + (score >> 1);
    if (blackCount == 1 && blackCheckersCount == 1 && whiteCount > 3) score = score + (score >> 1);

    if (blackQueensCount > 0 && score < 0) score = score >> 1;
    if (whiteQueensCount > 0 && score > 0) score = score >> 1;

    if (blackQueensCount == 1 && blackOnBigWay && !whiteOnBigWay && whiteCount < 4 &&
        (blackCount < 3 || score < 500))
        return 0;
    if (whiteQueensCount == 1 && !blackOnBigWay && whiteOnBigWay && blackCount < 4 &&
        (whiteCount < 3 || score > -500))
        return 0;

    return score;
}


inline short evalMiddle(
        _board wc,
        _board bc,
        _board wq,
        _board bq,
        bool opening
) {
    _ui whiteCheckersCount = bitCount(wc);
    _ui whiteQueensCount = bitCount(wq);
    _ui blackCheckersCount = bitCount(bc);
    _ui blackQueensCount = bitCount(bq);

    short whiteScore = CHECKER_SCORE * whiteCheckersCount + QUEEN_SCORE * whiteQueensCount;
    short blackScore = CHECKER_SCORE * blackCheckersCount + QUEEN_SCORE * blackQueensCount;

    short score = blackScore - whiteScore;
    if (score != 0) {
        score += 200 * score / (whiteScore + blackScore);
    }

    if (blackQueensCount != whiteQueensCount) {
        if (whiteQueensCount == 0 && blackCheckersCount >= whiteCheckersCount - 2)
            score += 200;
        else if (blackQueensCount == 0 && whiteCheckersCount >= blackCheckersCount - 2)
            score -= 200;
    }

    if ((blackQueensCount > 0 && score < 0) || (whiteQueensCount > 0 && score > 0))
        score = ((3 * score) >> 2);
    //  0b1_111_00111_0000111_0000011_000000000


    int code = 0;
    if (bc & cell(31)) code++;
    if (bc & cell(30)) code += 2;
    if (bc & cell(27)) code += 4;
    if (bc & cell(22)) code += 8;
    if (bc & cell(15)) code += 16;

    short backRank = BACK_RANK_VALUES[code];

    code = 0;
    if (wc & cell(0)) code++;
    if (wc & cell(1)) code += 2;
    if (wc & cell(4)) code += 4;
    if (wc & cell(9)) code += 8;
    if (wc & cell(16)) code += 16;

    backRank -= BACK_RANK_VALUES[code];
    score += opening ? 3 * backRank : backRank;


    if (whiteCheckersCount == blackCheckersCount) {
        int leftWhite = bitCount(wc & LEFT_FLANK_MASK);
        int leftBlack = bitCount(bc & LEFT_FLANK_MASK);
        int rightWhite = bitCount(wc & RIGHT_FLANK_MASK);
        int rightBlack = bitCount(bc & RIGHT_FLANK_MASK);

        if (leftWhite == 0) score += EMPTY_FLAG_SCORE;
        if (leftBlack == 0) score -= EMPTY_FLAG_SCORE;
        if (rightWhite == 0) score += EMPTY_FLAG_SCORE;
        if (rightBlack == 0) score -= EMPTY_FLAG_SCORE;

        int difference = abs(leftWhite - rightWhite);
        if (difference > 1) {
            score += difference * DIFFERENCE_FLANK;
        }
        difference = abs(leftBlack - rightBlack);
        if (difference > 1) {
            score -= difference * DIFFERENCE_FLANK;
        }
        if (rightWhite + leftWhite == whiteCheckersCount) score += 4;
        if (rightBlack + leftBlack == blackCheckersCount) score -= 4;
    }

    if (!(bc & RIGHT_CORNER)) {
        score += 8;
        if (!(bc & cell(30))) {
            score -= 5;
        }
    }
    if (!(wc & LEFT_CORNER)) {
        score -= 8;
        if (!(wc & cell(1))) {
            score += 5;
        }
    }


    _board free = ~(wc | bc);

    if (atCell(bc, 13)) {
        score++;
        if (atCell(bc, 20)) {
            if (atCell(bc, 26))
                score++;
            else
                score--;
        }

        if (atCell(bc, 14)) {
            if (atCell(bc, 15))
                score++;
            else
                score--;
        }
    }
    if ((atCell(bc, 19))) {
        score++;
        if ((atCell(bc, 25)))
            if ((atCell(bc, 29)))
                score++;
            else
                score--;

        if ((atCell(bc, 20)))
            if ((atCell(bc, 21)))
                score++;
            else
                score--;
    }
    if ((atCell(bc, 20))) {
        score++;
        if ((atCell(bc, 26)))
            if ((atCell(bc, 30)))
                score++;
            else
                score--;

        if ((atCell(bc, 21)))
            if ((atCell(bc, 22)))
                score++;
            else
                score--;
    }
    if ((atCell(bc, 18) && (free & (cell(1) | cell(11) | cell(17))))) {
        score += 2;
        if ((atCell(bc, 24)))
            if ((atCell(bc, 28)))
                score++;
            else
                score--;

        if ((atCell(bc, 19)))
            if ((atCell(bc, 20)))
                score++;
            else
                score--;
    }
    if ((atCell(bc, 11) && atCell(free, 4))) {
        score += 2;
        if ((atCell(bc, 18)))
            if ((atCell(bc, 24)))
                score++;
            else
                score--;

        if ((atCell(bc, 12)))
            if ((atCell(bc, 13)))
                score++;
            else
                score--;
    }
    if ((atCell(bc, 6) && atCell(free, 1))) {
        score += 2;
        if ((atCell(bc, 12)))
            if ((atCell(bc, 19)))
                score++;
            else
                score--;

        if ((atCell(bc, 7)))
            if ((atCell(bc, 8)))
                score++;
            else
                score--;
    }

    if ((atCell(wc, 18))) {
        score--;
        if ((atCell(wc, 17)))
            if ((atCell(wc, 16)))
                score--;
            else
                score++;

        if ((atCell(wc, 11)))
            if ((atCell(wc, 5)))
                score--;
            else
                score++;
    }
    if ((atCell(wc, 11))) {
        score--;
        if ((atCell(wc, 10)))
            if ((atCell(wc, 9)))
                score--;
            else
                score++;

        if ((atCell(wc, 5)))
            if ((atCell(wc, 1)))
                score--;
            else
                score++;
    }
    if ((atCell(wc, 12))) {
        score--;
        if ((atCell(wc, 11)))
            if ((atCell(wc, 10)))
                score--;
            else
                score++;

        if ((atCell(wc, 6)))
            if ((atCell(wc, 2)))
                score--;
            else
                score++;
    }
    if (atCell(wc, 25) && atCell(free, 30)) {
        score -= 2;
        if (atCell(wc, 24))
            if (atCell(wc, 23))
                score--;
            else
                score++;

        if (atCell(wc, 19))
            if (atCell(wc, 12))
                score--;
            else
                score++;
    }
    if (atCell(wc, 20) && atCell(free, 27)) {
        score -= 2;
        if (atCell(wc, 19))
            if (atCell(wc, 18))
                score--;
            else
                score++;

        if (atCell(wc, 13))
            if (atCell(wc, 7))
                score--;
            else
                score++;
    }
    if (atCell(wc, 13) && (free & (cell(20) | cell(14) | cell(30)))) {
        score -= 2;
        if (atCell(wc, 12))
            if (atCell(wc, 11))
                score--;
                    else
                score++;

        if (atCell(wc, 7))
            if (atCell(wc, 3))
                score--;
                    else
                score++;
    }


    score -= bitCount(bc & (cell(15) | cell(8) | cell(28)));
    if (atCell(bc, 23)) score++;
    if (atCell(bc, 16)) score++;
    if (atCell(bc, 3) && atCell(free, 1)) score++;

    score += bitCount(bc & (cell(16) | cell(23) | cell(3)));
    if (atCell(bc, 8)) score--;
    if (atCell(bc, 15)) score--;
    if (atCell(bc, 28) && atCell(free, 30)) score--;

    if (atCell(bc, 12)) {
        if (!opening) {
            score++;
            if (atCell(bc, 19))
                if (atCell(bc, 25))
                    score++;
                else
                    score--;

            if (atCell(bc, 13))
                if (atCell(bc, 14))
                    score++;
                else
                    score--;
        } else {
            score -= 4;
        }
    }


    /*
       8,      7,      6,      5,
  13,     12,     11,      10,
    17,     16,     15,     14,
  22,     21,     20,     19,
     26,     25,     24,     23,
  31,     30,     29,     28,
     35,     34,     33,     32,
  40,     39,     38,     37
*/


/*
            22,      27,    30,    31,
        15,     21,     26,    29,
            14,     20,     25,    28,
        8,     13,     19,     24,
            7,     12,     18,     23,
        3,     6,     11,     17,
            2,     5,     10,      16,
        0,     1,     4,        9
*/
    if (atCell(wc, 19)) {
        if (!opening) {
            score--;
            if (atCell(wc, 18))
                if (atCell(wc, 17))
                    score--;
                else
                    score++;

            if (atCell(wc, 12))
                if (atCell(wc, 6))
                    score--;
                else
                    score++;
        } else {
            score += 4;
        }
    }

    return score;
}



short eval(
        _board wc,
        _board bc,
        _board wq,
        _board bq
) {
    if ((wc | wq) == 0)
        return -30000;
    if ((bc | bq) == 0)
        return 30000;

    _ui count = bitCount(wc | wq | bc | bq);
    if (count <= 8) {
        return -endGameEval(wc, bc, wq, bq);
    }
    return -evalMiddle(wc, bc, wq, bq, count > 19);
}

