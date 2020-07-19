package ru.rpuxa.checkerscpp

import android.content.Intent
import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import kotlinx.android.synthetic.main.activity_start.*

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