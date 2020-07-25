package ru.rpuxa.checkerscpp

import android.Manifest
import android.content.Intent
import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import androidx.core.app.ActivityCompat
import kotlinx.android.synthetic.main.activity_start.*
import java.io.File

class StartActivity : AppCompatActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_start)


        white.setOnClickListener {
            start(true)

        }
        black.setOnClickListener {
            start(false)
        }
    }

    private fun start(white: Boolean) {
        startActivity(Intent(this, MainActivity::class.java).apply {
            putExtra("white", white)
            putExtra("diagonal", diagonal.isChecked)
        })
    }
}