package moe.key.yao.mqtt.client

import android.Manifest
import android.content.pm.PackageManager
import android.os.Bundle
import android.widget.Toast
import androidx.appcompat.app.AppCompatActivity
import androidx.core.app.ActivityCompat
import androidx.core.content.ContextCompat
import com.google.common.io.ByteStreams
import kotlinx.android.synthetic.main.activity_main.*
import moe.key.yao.mqtt.library.MqttClient
import java.io.File
import java.io.FileOutputStream

class MainActivity : AppCompatActivity(), MqttClient.MqttCallback {

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)
        MqttClient.getInstance().setOnMqttCallback(this)
        initLayout()
        initPermission()
    }

    override fun onDestroy() {
        super.onDestroy()
        MqttClient.getInstance().setOnMqttCallback(null)
    }

    private fun initLayout() {
        btn_start.setOnClickListener {
            startMqttService()
        }
        btn_subscribe.setOnClickListener {
            subscribeTopic()
        }
        btn_unsubscribe.setOnClickListener {
            unsubscribeTopic()
        }
        btn_publish.setOnClickListener {
            publishMessage()
        }
    }

    private fun initPermission() {
        val hasPermission: Int = ContextCompat.checkSelfPermission(this, Manifest.permission.READ_EXTERNAL_STORAGE)
        if (hasPermission != PackageManager.PERMISSION_GRANTED) {
            ActivityCompat.requestPermissions(this, arrayOf(Manifest.permission.READ_EXTERNAL_STORAGE, Manifest.permission.WRITE_EXTERNAL_STORAGE), 1)
            return
        }
    }

    private fun startMqttService() {
        // copy ca file to sdcard
        /*val caFilePath = "${getExternalFilesDir(null)}/ca.crt"
        val file = File(caFilePath)
        if (!file.exists()) {
            file.createNewFile()
            val inStream = assets.open("ca.crt")
            val outStream = FileOutputStream(file)
            ByteStreams.copy(inStream, outStream)
            inStream.close()
            outStream.close()
        }*/

        MqttClient.getInstance().start("192.168.1.101", 1883, "mqtt_android_client", false)
        //MqttClient.getInstance().start("192.168.1.101", 8883, "mqtt_android_client", false, caFilePath, "username", "password")
    }

    private fun subscribeTopic() {
        val topic = "/android/test/topicA"
        MqttClient.getInstance().subscribe(topic)
    }

    private fun unsubscribeTopic() {
        val topic = "/android/test/topicA"
        MqttClient.getInstance().unsubscribe(topic)
    }

    private fun publishMessage() {
        val topic = "/android/test/topicA"
        val message = "test message"
        MqttClient.getInstance().publish(topic, message)
    }

    override fun onMessage(topic: String, message: String) {
        println("on message: $topic | $message")
    }

    override fun onLog(str: String?) {
        println("on log: $str")
    }

    override fun onRequestPermissionsResult(
        requestCode: Int,
        permissions: Array<out String>,
        grantResults: IntArray
    ) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults)
        if (requestCode == 1) {
            var flag = false
            for (i in permissions.indices) {
                if (permissions[i] == Manifest.permission.READ_EXTERNAL_STORAGE && grantResults[i] == PackageManager.PERMISSION_GRANTED) {
                    flag = true
                    break
                }
            }
            if (!flag) {
                Toast.makeText(this, "获取权限失败", Toast.LENGTH_SHORT).show()
            }
        }
    }

}
