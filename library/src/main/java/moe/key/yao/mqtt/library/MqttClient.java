package moe.key.yao.mqtt.library;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.annotation.WorkerThread;

import java.util.List;

/**
 * Created by key on 2020-02-20.
 *
 * @author key
 */
public class MqttClient {

    static {
        System.loadLibrary("mqtt");
    }

    public interface MqttCallback {
        @WorkerThread
        void onMessage(@NonNull String topic, @NonNull String message);
        @WorkerThread
        void onLog(String str);
    }

    private static MqttClient mInstance;

    public static MqttClient getInstance() {
        if (mInstance == null) {
            synchronized (MqttClient.class) {
                mInstance = new MqttClient();
            }
        }
        return mInstance;
    }

    private MqttClient() {

    }

    private long mPtr = 0;
    private MqttCallback mCallback = null;

    /**
     * 设置消息回调
     */
    public void setOnMqttCallback(@Nullable MqttCallback callback) {
        mCallback = callback;
    }

    /**
     * 启动mqtt服务
     * @param host  服务端地址
     * @param port  服务端口
     * @param uuid  客户端uuid
     * @param clearSession  是否清除上次回话
     */
    public void start(@NonNull String host, int port, @NonNull String uuid, boolean clearSession) {
        startInternal(host, port, uuid, clearSession, false, null, null, null);
    }

    /**
     * 启动mqtt服务（使用ssl）
     * @param host  服务端地址
     * @param port  服务端口
     * @param uuid  客户端uuid
     * @param clearSession  是否清除上次回话
     * @param caFilePath    证书文件路径
     * @param username  验证用户名
     * @param password  验证密码
     */
    public void start(@NonNull String host, int port, @NonNull String uuid, boolean clearSession, @NonNull String caFilePath, @NonNull String username, @NonNull String password) {
        startInternal(host, port, uuid, clearSession, true, caFilePath, username, password);
    }

    private void startInternal(@NonNull String host, int port, @NonNull String uuid, boolean clearSession, boolean isTLS, @Nullable String caFilePath, @Nullable String username, @Nullable String password) {
        if (mPtr == 0) {
            mPtr = _init(host,
                    port,
                    uuid,
                    clearSession,
                    isTLS,
                    caFilePath == null ? "" : caFilePath,
                    username == null ? "" : username,
                    password == null ? "" : password);
        }
        _start(mPtr);
    }

    /**
     * 重连服务
     */
    public void reconnect() {
        if (mPtr == 0) {
            return;
        }
        _reconnect(mPtr);
    }

    /**
     * 订阅topic
     */
    public void subscribe(String topic) {
        if (mPtr == 0) {
            return;
        }
        _subscribe(mPtr, new String[]{topic}, new int[]{2});
    }

    /**
     * 订阅topic
     */
    public void subscribe(String topic, int qos) {
        if (mPtr == 0) {
            return;
        }
        _subscribe(mPtr, new String[]{topic}, new int[]{qos});
    }

    /**
     * 订阅topic
     */
    public void subscribe(List<String> topicList) {
        if (mPtr == 0) {
            return;
        }
        String[] topicArray = new String[topicList.size()];
        int[] qosArray = new int[topicList.size()];
        for (int i = 0; i < topicList.size(); i++) {
            topicArray[i] = topicList.get(i);
            qosArray[i] = 2;
        }
        _subscribe(mPtr, topicArray, qosArray);
    }

    /**
     * 订阅topic
     */
    public void subscribe(List<String> topicList, List<Integer> qosList) {
        if (mPtr == 0) {
            return;
        }
        String[] topicArray = new String[topicList.size()];
        int[] qosArray = new int[topicList.size()];
        for (int i = 0; i < topicList.size(); i++) {
            topicArray[i] = topicList.get(i);
            qosArray[i] = qosList.get(i);
        }
        _subscribe(mPtr, topicArray, qosArray);

    }

    /**
     * 取消订阅topic
     */
    public void unsubscribe(String topic) {
        if (mPtr == 0) {
            return;
        }
        _unsubscribe(mPtr, new String[]{topic});
    }

    /**
     * 取消订阅topic
     */
    public void unsubscribe(List<String> topicList) {
        if (mPtr == 0) {
            return;
        }
        String[] topicArray = new String[topicList.size()];
        for (int i = 0; i < topicList.size(); i++) {
            topicArray[i] = topicList.get(i);
        }
        _unsubscribe(mPtr, topicArray);
    }

    /**
     * 发布消息
     * @param topic 目标topic
     * @param message 消息内容
     */
    public void publish(String topic, String message) {
        if (mPtr == 0) {
            return;
        }
        _publish(mPtr, topic, message, 0);
    }

    /**
     * 发布消息
     * @param topic 目标topic
     * @param message 消息内容
     * @param qos   qos
     */
    public void publish(String topic, String message, int qos) {
        if (mPtr == 0) {
            return;
        }
        _publish(mPtr, topic, message, qos);
    }

    // ==== callback
    @WorkerThread
    private void onMessage(String topic, String payload) {
        if (mCallback != null) {
            mCallback.onLog("Mqtt onMessage: " + topic + " | " + payload);
            mCallback.onMessage(topic, payload);
        }
    }

    @WorkerThread
    private void onConnect() {
        if (mCallback != null) {
            mCallback.onLog("Mqtt onConnect");
        }
    }

    @WorkerThread
    private void onConnectWithFlag(int flags) {
        if (mCallback != null) {
            mCallback.onLog("Mqtt onConnect with flags: " + flags);
        }
    }

    @WorkerThread
    private void onDisconnect() {
        if (mCallback != null) {
            mCallback.onLog("Mqtt onDisconnect");
        }
    }

    @WorkerThread
    private void onPublish() {
        if (mCallback != null) {
            mCallback.onLog("Mqtt onPublish");
        }
    }

    @WorkerThread
    private void onSubscribe() {
        if (mCallback != null) {
            mCallback.onLog("Mqtt onSubscribe");
        }
    }

    @WorkerThread
    private void onUnsubscribe() {
        if (mCallback != null) {
            mCallback.onLog("Mqtt onUnsubscribe");
        }
    }

    @WorkerThread
    private void onLog(int level, String str) {
        /*
            #define MOSQ_LOG_NONE			0
            #define MOSQ_LOG_INFO			(1<<0)
            #define MOSQ_LOG_NOTICE			(1<<1)
            #define MOSQ_LOG_WARNING		(1<<2)
            #define MOSQ_LOG_ERR			(1<<3)
            #define MOSQ_LOG_DEBUG			(1<<4)
            #define MOSQ_LOG_SUBSCRIBE		(1<<5)
            #define MOSQ_LOG_UNSUBSCRIBE	(1<<6)
            #define MOSQ_LOG_WEBSOCKETS		(1<<7)
            #define MOSQ_LOG_INTERNAL		0x80000000
            #define MOSQ_LOG_ALL			0x7FFFFFFF
         */
        if (mCallback != null) {
            mCallback.onLog("Mqtt onLog: " + str + " | level: " + level);
        }
    }

    // ==== native
    private native long _init(String host, int port, String uuid, boolean clearSession, boolean isTLS, String caFilePath, String userName, String password);

    private native void _start(long ptr);

    private native void _reconnect(long ptr);

    private native void _subscribe(long ptr, String[] topicArray, int[] qosArray);

    private native void _unsubscribe(long ptr, String[] topArray);

    private native void _publish(long ptr, String topic, String message, int qos);


}
