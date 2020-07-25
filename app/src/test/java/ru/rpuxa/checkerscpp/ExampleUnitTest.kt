package ru.rpuxa.checkerscpp

import org.junit.Test

import org.junit.Assert.*
import ru.rpuxa.checkerscpp.natives.NativeEngine

/**
 * Example local unit test, which will execute on the development machine (host).
 *
 * See [testing documentation](http://d.android.com/tools/testing).
 */
class ExampleUnitTest {
    @Test
    fun addition_isCorrect() {
        NativeEngine.prepareEngine()
    }
}