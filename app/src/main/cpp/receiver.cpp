#include <jni.h>
#include "board.h"
#include "board.cpp"
#include "bitutils.h"
#include "engine.h"
#include "engine.cpp"

const _move END_MOVES_FLAG = ~static_cast<const _move>(0);


extern "C"
JNIEXPORT void JNICALL
Java_ru_rpuxa_checkerscpp_natives_NativeMethods_prepareEngine(JNIEnv *env, jclass type) {
    gen();
}

extern "C"
JNIEXPORT void JNICALL
Java_ru_rpuxa_checkerscpp_natives_NativeMethods_getBestMove(
        JNIEnv *env,
        jclass type,
        jint whiteCheckers,
        jint blackCheckers,
        jint whiteQueens,
        jint blackQueens,
        jint multiTake,
        jboolean isWhiteMove,
        jshort analyzeDepth,
        jshortArray evalAndMoves_
) {

    jshort *evalAndMoves = env->GetShortArrayElements(evalAndMoves_, 0);


    auto wc = static_cast<_board>(whiteCheckers);
    auto bc = static_cast<_board>(blackCheckers);
    auto wq = static_cast<_board>(whiteQueens);
    auto bq = static_cast<_board>(blackQueens);
    _board w = wc | wq;
    _board b = bc | bq;

    std::vector<_move> moves;
    short eval;

    getBestMove(
            wc,
            bc,
            wq,
            bq,
            rotateBoard(w),
            rotateBoard(b),
            multiTake,
            isWhiteMove,
            analyzeDepth,
            moves,
            eval
    );

    evalAndMoves[0] = eval;
    int i = 1;
    for (_move move : moves) {
        evalAndMoves[i++] = move;
    }

    evalAndMoves[i] = END_MOVES_FLAG;

    env->ReleaseShortArrayElements(evalAndMoves_, evalAndMoves, 0);
}


extern "C"
JNIEXPORT void JNICALL
Java_ru_rpuxa_checkerscpp_natives_NativeMethods_getAvailableMoves(JNIEnv *env, jclass type,
                                                                  jint whiteCheckers,
                                                                  jint blackCheckers,
                                                                  jint whiteQueens,
                                                                  jint blackQueens,
                                                                  jboolean isWhiteTurn,
                                                                  jshortArray movesArray_) {
    jshort *movesArray = env->GetShortArrayElements(movesArray_, 0);

    _board wc = static_cast<_board>(whiteCheckers);
    _board bc = static_cast<_board>(blackCheckers);
    _board wq = static_cast<_board>(whiteQueens);
    _board bq = static_cast<_board>(blackQueens);
    _board w = wc | wq;
    _board b = bc | bq;
    getMoves(
            wc,
            bc,
            wq,
            bq,
            rotateBoard(w),
            rotateBoard(b),
            isWhiteTurn,
            false,
            -1
    );

    _move size = *currentMoves;

    for (int i = 1; i <= size; ++i) {
        movesArray[i - 1] = currentMoves[i];
    }

    movesArray[size] = END_MOVES_FLAG;

    env->ReleaseShortArrayElements(movesArray_, movesArray, 0);
}

extern "C"
JNIEXPORT void JNICALL
Java_ru_rpuxa_checkerscpp_natives_NativeMethods_makeMove(JNIEnv *env, jclass type,
                                                         jint whiteCheckers,
                                                         jint blackCheckers, jint whiteQueens,
                                                         jint blackQueens,
                                                         jshort move, jintArray changedPosition_) {
    jint *changedPosition = env->GetIntArrayElements(changedPosition_, 0);

    _board wc = static_cast<_board>(whiteCheckers);
    _board bc = static_cast<_board>(blackCheckers);
    _board wq = static_cast<_board>(whiteQueens);
    _board bq = static_cast<_board>(blackQueens);
    _board w = wc | wq;
    _move m = static_cast<_move>(move);

    _ui tmp1, tmp2;

    bool isWhiteMove = static_cast<bool>(getBit(w, static_cast<_ci>((move >> 1) & 0b11111)));

    _hash tmp;
    makeMove(wc, bc, wq, bq, tmp1, tmp2, m, tmp, isWhiteMove);

    changedPosition[0] = wc;
    changedPosition[1] = bc;
    changedPosition[2] = wq;
    changedPosition[3] = bq;

    env->ReleaseIntArrayElements(changedPosition_, changedPosition, 0);
}


extern "C"
JNIEXPORT void JNICALL
Java_ru_rpuxa_checkerscpp_natives_NativeMethods_stopSearching(JNIEnv *env, jclass clazz) {
    stopSearching();
}