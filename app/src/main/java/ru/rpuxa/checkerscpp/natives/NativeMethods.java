package ru.rpuxa.checkerscpp.natives;


public class NativeMethods {

    public static native void prepareEngine();

    public static native void prepareEndGame(String path);

    public static native void prepareDebut(String pathFile);

    public static native void getBestMove(
            int whiteCheckers,
            int blackCheckers,
            int whiteQueens,
            int blackQueens,
            short previousMove,
            boolean isWhiteMove,
            short analyzeDepth,
            short[] evalAndMoves
    );

    public static native void getAvailableMoves(
            int whiteCheckers,
            int blackCheckers,
            int whiteQueens,
            int blackQueens,
            boolean isTurnWhite,
            short previousMove,
            short[] movesArray
    );

    public static native void makeMove(
            int whiteCheckers,
            int blackCheckers,
            int whiteQueens,
            int blackQueens,
            short move,
            int[] changedPosition
    );

    public static native void stopSearching();


    static {
        System.loadLibrary("receiver");
    }
}