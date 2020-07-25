package ru.rpuxa.checkerscpp

import android.Manifest
import android.content.pm.PackageManager
import android.os.Environment
import androidx.core.app.ActivityCompat
import androidx.core.content.ContextCompat
import androidx.test.ext.junit.runners.AndroidJUnit4
import androidx.test.rule.ActivityTestRule
import org.junit.Rule

import org.junit.Test
import org.junit.runner.RunWith

import ru.rpuxa.checkerscpp.game.board.*
import ru.rpuxa.checkerscpp.natives.*
import java.io.DataOutputStream
import java.io.File
import java.io.FileOutputStream
import kotlin.math.pow

/**
 * Instrumented test, which will execute on an Android device.
 *
 * See [testing documentation](http://d.android.com/tools/testing).
 */


class ExampleInstrumentedTest {

    @Rule
    @JvmField
    var rule = ActivityTestRule<StartActivity>(StartActivity::class.java)


}


