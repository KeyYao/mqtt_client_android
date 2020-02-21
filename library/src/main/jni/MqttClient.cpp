//
// Created by Key.Yao on 2020-02-20.
//

#include "MqttClient.h"

#include <jni.h>
#include <android/log.h>
#include <pthread.h>
#include <unistd.h>
#include "mosquitto_wrapper.h"
#include "JNIEnvHandler.h"

#define ALOG(...) ((void)__android_log_print(ANDROID_LOG_INFO, "MqttClientJNI", __VA_ARGS__))

#ifndef UNUSED
#define UNUSED(A) (void)(A)
#endif

#ifndef NELEM
#define NELEM(x) ((int) (sizeof(x) / sizeof((x)[0])))
#endif

#ifndef MQTTCLIENT_JNICALL
#define MQTTCLIENT_JNICALL
#endif

#define TYPE_ON_MESSAGE 1
#define TYPE_ON_CONNECT 2
#define TYPE_ON_CONNECT_WITH_FLAG 3
#define TYPE_ON_DISCONNECT 4
#define TYPE_ON_PUBLISH 5
#define TYPE_ON_SUBSCRIBE 6
#define TYPE_ON_UNSUBSCRIBE 7
#define TYPE_ON_LOG 8

using namespace std;
using namespace mqttclient;

#ifdef __cplusplus
extern  "C" {
#endif
// thread callback method define
void* mqtt_thread(void *p);
void* mqtt_reconnect_thread(void *p);

// mosquitto callback method define
void mqtt_on_message(void *instance, const struct mosquitto_message *message);
void mqtt_on_connect(void *instance, int rc);
void mqtt_on_connect_with_flag(void *instance, int rc, int flags);
void mqtt_on_disconnect(void *instance, int rc);
void mqtt_on_publish(void *instance, int rc);
void mqtt_on_subscribe(void *instance, int mid, int qos_count, const int *granted_qos);
void mqtt_on_unsubscribe(void *instance, int rc);
void mqtt_on_log(void *instance, int level, const char *str);

// callback to java define
void callback2Java(int what, void* obj, JNIEnv *env, jobject instance);

// jni method define
jlong MQTTCLIENT_JNICALL mqtt_native_init(JNIEnv *env, jobject instance, jstring hostString, jint portInt, jstring uuidString, jboolean clearSession, jboolean isTSL, jstring caFilePathString, jstring usernameString, jstring passwordString);
void MQTTCLIENT_JNICALL mqtt_native_start(JNIEnv *env, jobject instance, jlong ptr);
void MQTTCLIENT_JNICALL mqtt_native_reconnect(JNIEnv *env, jobject instance, jlong ptr);
void MQTTCLIENT_JNICALL mqtt_native_subscribe(JNIEnv *env, jobject instance, jlong ptr, jobjectArray topicJArray, jintArray qosJArray);
void MQTTCLIENT_JNICALL mqtt_native_unsubscribe(JNIEnv *env, jobject instance, jlong ptr, jobjectArray topicJArray);
void MQTTCLIENT_JNICALL mqtt_native_publish(JNIEnv *env, jobject instance, jlong ptr, jstring topicString, jstring messageString, jint qosInt);
JNINativeMethod mqtt_methods[] = {
        {
                "_init",
                "(Ljava/lang/String;ILjava/lang/String;ZZLjava/lang/String;Ljava/lang/String;Ljava/lang/String;)J",
                (void*) mqtt_native_init
        },
        {
                "_start",
                "(J)V",
                (void*) mqtt_native_start
        },
        {
                "_reconnect",
                "(J)V",
                (void*) mqtt_native_reconnect
        },
        {
                "_subscribe",
                "(J[Ljava/lang/String;[I)V",
                (void*) mqtt_native_subscribe
        },
        {
                "_unsubscribe",
                "(J[Ljava/lang/String;)V",
                (void*) mqtt_native_unsubscribe
        },
        {
                "_publish",
                "(JLjava/lang/String;Ljava/lang/String;I)V",
                (void*) mqtt_native_publish
        }
};


#ifdef __cplusplus
}
#endif

class ExtraData {
public:
    string *host = nullptr;
    int port = 0;
    JNIEnvHandler *handler = nullptr;
    JavaVM *javaVM = nullptr;
    bool exitFlag = false;
    bool running = false;
    pthread_mutex_t mutexReconnect;
};

class MqttMsg {
public:
    string *topic = nullptr;
    string *payload = nullptr;
};

class MqttConnectFlag {
public:
    int flags = 0;
};

class MqttLog {
public:
    string *str = nullptr;
    int level = 0;
};

static JavaVM *mVM;


JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {
    UNUSED(reserved);

    mVM = vm;
    JNIEnv *env = nullptr;
    if (vm->GetEnv((void **) &env, JNI_VERSION_1_6) != JNI_OK) {
        return JNI_ERR;
    }

    jclass clazz = env->FindClass("moe/key/yao/mqtt/library/MqttClient");
    if (clazz == nullptr) {
        return JNI_ERR;
    }

    if (env->RegisterNatives(clazz, mqtt_methods, NELEM(mqtt_methods)) < 0) {
        env->DeleteLocalRef(clazz);
        return JNI_ERR;
    } else {
        env->DeleteLocalRef(clazz);
    }

    // lib init
    mqttclient::lib_init();

    return JNI_VERSION_1_6;
}

JNIEXPORT void JNICALL JNI_OnUnload(JavaVM *vm, void *reserved) {
    UNUSED(vm);
    UNUSED(reserved);

    // lib clean up
    mqttclient::lib_cleanup();
}


// jni method
jlong MQTTCLIENT_JNICALL mqtt_native_init(JNIEnv *env, jobject instance, jstring hostString, jint portInt, jstring uuidString, jboolean clearSession, jboolean isTSL, jstring caFilePathString, jstring usernameString, jstring passwordString) {
    const char *host = env->GetStringUTFChars(hostString, nullptr);
    const char *uuid = env->GetStringUTFChars(uuidString, nullptr);
    const char *caFilePath = env->GetStringUTFChars(caFilePathString, nullptr);
    const char *username = env->GetStringUTFChars(usernameString, nullptr);
    const char *password = env->GetStringUTFChars(passwordString, nullptr);

    bool sessionFlag = clearSession != 0;
    auto *mosq = new mosquitto_wrapper(uuid, sessionFlag);

    mosq->on_connect_callback = mqtt_on_connect;
    mosq->on_connect_with_flag_callback = mqtt_on_connect_with_flag;
    mosq->on_disconnect_callback = mqtt_on_disconnect;
    mosq->on_publish_callback = mqtt_on_publish;
    mosq->on_message_callback = mqtt_on_message;
    mosq->on_subscribe_callback = mqtt_on_subscribe;
    mosq->on_unsubscribe_callback = mqtt_on_unsubscribe;
    mosq->on_log_callback = mqtt_on_log;


    if (isTSL) {
        mosq->tls_insecure_set(true);
        mosq->tls_opts_set(1, "tlsv1", nullptr);
        if (strlen(caFilePath) != 0) {
            mosq->tls_set(caFilePath);
        }
        if (strlen(username) != 0 && strlen(password) != 0) {
            mosq->username_pw_set(username, password);
        }
    }

    auto *extra = new ExtraData();

    extra->host = new string(host);
    extra->port = portInt;
    extra->javaVM = mVM;

    extra->handler = new JNIEnvHandler("callback");
    extra->handler->init(mVM, env->NewGlobalRef(instance), callback2Java);

    pthread_mutex_init(&extra->mutexReconnect, nullptr);

    mosq->extra = extra;

    env->ReleaseStringUTFChars(hostString, host);
    env->ReleaseStringUTFChars(caFilePathString, caFilePath);
    env->ReleaseStringUTFChars(uuidString, uuid);

    return reinterpret_cast<jlong>(mosq);
}

void MQTTCLIENT_JNICALL mqtt_native_start(JNIEnv *env, jobject instance, jlong ptr) {
    UNUSED(env);
    UNUSED(instance);

    auto *mosq = reinterpret_cast<mosquitto_wrapper*>(ptr);
    if (!mosq) {
        return;
    }
    auto *extra = reinterpret_cast<ExtraData*>(mosq->extra);
    if (!extra) {
        return;
    }
    if (!extra->running) {
        pthread_t tid;
        pthread_create(&tid, nullptr, mqtt_thread, mosq);
        pthread_detach(tid);
    } else {
        // running
        ALOG("mqtt thread running");
    }
}

void MQTTCLIENT_JNICALL mqtt_native_reconnect(JNIEnv *env, jobject instance, jlong ptr) {
    UNUSED(env);
    UNUSED(instance);

    auto *mosq = reinterpret_cast<mosquitto_wrapper*>(ptr);
    if (!mosq) {
        return;
    }
    pthread_t tid;
    pthread_create(&tid, nullptr, mqtt_reconnect_thread, mosq);
    pthread_detach(tid);
}

void MQTTCLIENT_JNICALL mqtt_native_subscribe(JNIEnv *env, jobject instance, jlong ptr, jobjectArray topicJArray, jintArray qosJArray) {
    UNUSED(instance);

    jint arrayLen = env->GetArrayLength(topicJArray);
    jint* qosList = env->GetIntArrayElements(qosJArray, nullptr);
    string *topics[arrayLen];

    for (int i = 0; i < arrayLen; i++) {
        auto itemString = (jstring) env->GetObjectArrayElement(topicJArray, i);
        const char *item = env->GetStringUTFChars(itemString, nullptr);
        topics[i] = new string(item);
        env->ReleaseStringUTFChars(itemString, item);
    }

    auto *mosq = reinterpret_cast<mosquitto_wrapper*>(ptr);
    for (int i = 0 ; i < arrayLen; i ++) {
        int mid;
        mosq->subscribe(&mid, topics[i]->c_str(), qosList[i]);
        delete topics[i];
        topics[i] = nullptr;
    }
}

void MQTTCLIENT_JNICALL mqtt_native_unsubscribe(JNIEnv *env, jobject instance, jlong ptr, jobjectArray topicJArray) {
    UNUSED(instance);

    jint arrayLen = env->GetArrayLength(topicJArray);
    string *topics[arrayLen];

    for (int i = 0; i < arrayLen; i++) {
        auto itemString = (jstring) env->GetObjectArrayElement(topicJArray, i);
        const char *item = env->GetStringUTFChars(itemString, nullptr);
        topics[i] = new string(item);
        env->ReleaseStringUTFChars(itemString, item);
    }

    auto *mosq = reinterpret_cast<mosquitto_wrapper*>(ptr);
    for (int i = 0 ; i < arrayLen; i ++) {
        int mid;
        mosq->unsubscribe(&mid, topics[i]->c_str());
        delete topics[i];
        topics[i] = nullptr;
    }
}

void MQTTCLIENT_JNICALL mqtt_native_publish(JNIEnv *env, jobject instance, jlong ptr, jstring topicString, jstring messageString, jint qosInt) {
    UNUSED(instance);

    const char *topic = env->GetStringUTFChars(topicString, nullptr);
    const char *message = env->GetStringUTFChars(messageString, nullptr);

    // publish
    auto *mosq = reinterpret_cast<mosquitto_wrapper*>(ptr);
    mosq->publish(nullptr, topic, (int) strlen(message), message, qosInt);

    // release string
    env->ReleaseStringUTFChars(topicString, topic);
    env->ReleaseStringUTFChars(messageString, message);
}

// thread callback method
void* mqtt_thread(void *p) {
    auto *mosq = reinterpret_cast<mosquitto_wrapper*>(p);
    if (!mosq) {
        return nullptr;
    }

    auto *extra = reinterpret_cast<ExtraData*>(mosq->extra);
    if (!extra) {
        return nullptr;
    }

    JNIEnv *env;
    extra->javaVM->AttachCurrentThread(&env, nullptr);

    extra->running = true;

    mosq->connect(extra->host->c_str(), extra->port);

    while (true) {
        int rc = mosq->loop();
        if (extra->exitFlag) {
            break;
        }
        if (rc != MOSQ_ERR_SUCCESS) {
            // error
            const char *errorMsg = mqttclient::strerror(rc);
            ALOG("mqtt connect error ========>> %s", errorMsg);
        }
        if (rc) {
            sleep(10);
            mosq->reconnect();
        }
    }

    extra->javaVM->DetachCurrentThread();

    extra->running = false;

    pthread_mutex_destroy(&extra->mutexReconnect);

    extra->javaVM = nullptr;
    delete extra->host;

    extra->handler->exit();
    delete extra->handler;
    extra->handler = nullptr;

    delete extra;
    mosq->extra = nullptr;

    delete mosq;
    return nullptr;
}

void* mqtt_reconnect_thread(void *p) {
    auto *mosq = reinterpret_cast<mosquitto_wrapper*>(p);
    if (!mosq) {
        return nullptr;
    }

    auto *extra = reinterpret_cast<ExtraData*>(mosq->extra);
    if (!extra) {
        return nullptr;
    }
    pthread_mutex_lock(&extra->mutexReconnect);
    mosq->disconnect();
    pthread_mutex_unlock(&extra->mutexReconnect);
    return nullptr;
}

// mosquitto callback method define
void mqtt_on_message(void *instance, const struct mosquitto_message *message) {
    if (!instance) {
        return;
    }
    auto mosq = reinterpret_cast<mosquitto_wrapper*>(instance);
    if (!mosq->extra) {
        return;
    }
    auto extra = reinterpret_cast<ExtraData*>(mosq->extra);
    if (!extra) {
        return;
    }
    if (!extra->handler) {
        return;
    }

    auto msg = new MqttMsg();
    msg->topic = new string(message->topic);
    msg->payload = new string(reinterpret_cast<char*>(message->payload));

    extra->handler->post(TYPE_ON_MESSAGE, msg);
}

void mqtt_on_connect(void *instance, int rc) {
    UNUSED(rc);

    if (!instance) {
        return;
    }
    auto mosq = reinterpret_cast<mosquitto_wrapper*>(instance);
    if (!mosq->extra) {
        return;
    }
    auto extra = reinterpret_cast<ExtraData*>(mosq->extra);
    if (!extra) {
        return;
    }
    if (!extra->handler) {
        return;
    }
    extra->handler->post(TYPE_ON_CONNECT);
}

void mqtt_on_connect_with_flag(void *instance, int rc, int flags) {
    UNUSED(rc);

    if (!instance) {
        return;
    }
    auto mosq = reinterpret_cast<mosquitto_wrapper*>(instance);
    if (!mosq->extra) {
        return;
    }
    auto extra = reinterpret_cast<ExtraData*>(mosq->extra);
    if (!extra) {
        return;
    }
    if (!extra->handler) {
        return;
    }
    auto connectFlag = new MqttConnectFlag();
    connectFlag->flags = flags;
    extra->handler->post(TYPE_ON_CONNECT_WITH_FLAG, connectFlag);
}

void mqtt_on_disconnect(void *instance, int rc) {
    UNUSED(rc);

    if (!instance) {
        return;
    }
    auto mosq = reinterpret_cast<mosquitto_wrapper*>(instance);
    if (!mosq->extra) {
        return;
    }
    auto extra = reinterpret_cast<ExtraData*>(mosq->extra);
    if (!extra) {
        return;
    }
    if (!extra->handler) {
        return;
    }
    extra->handler->post(TYPE_ON_DISCONNECT);
}

void mqtt_on_publish(void *instance, int rc) {
    UNUSED(rc);

    if (!instance) {
        return;
    }
    auto mosq = reinterpret_cast<mosquitto_wrapper*>(instance);
    if (!mosq->extra) {
        return;
    }
    auto extra = reinterpret_cast<ExtraData*>(mosq->extra);
    if (!extra) {
        return;
    }
    if (!extra->handler) {
        return;
    }
    extra->handler->post(TYPE_ON_PUBLISH);
}

void mqtt_on_subscribe(void *instance, int mid, int qos_count, const int *granted_qos) {
    UNUSED(mid);
    UNUSED(qos_count);
    UNUSED(granted_qos);

    if (!instance) {
        return;
    }
    auto mosq = reinterpret_cast<mosquitto_wrapper*>(instance);
    if (!mosq->extra) {
        return;
    }
    auto extra = reinterpret_cast<ExtraData*>(mosq->extra);
    if (!extra) {
        return;
    }
    if (!extra->handler) {
        return;
    }
    extra->handler->post(TYPE_ON_SUBSCRIBE);
}

void mqtt_on_unsubscribe(void *instance, int rc) {
    UNUSED(rc);

    if (!instance) {
        return;
    }
    auto mosq = reinterpret_cast<mosquitto_wrapper*>(instance);
    if (!mosq->extra) {
        return;
    }
    auto extra = reinterpret_cast<ExtraData*>(mosq->extra);
    if (!extra) {
        return;
    }
    if (!extra->handler) {
        return;
    }
    extra->handler->post(TYPE_ON_UNSUBSCRIBE);
}

void mqtt_on_log(void *instance, int level, const char *str) {
    if (!instance) {
        return;
    }
    auto mosq = reinterpret_cast<mosquitto_wrapper*>(instance);
    if (!mosq->extra) {
        return;
    }
    auto extra = reinterpret_cast<ExtraData*>(mosq->extra);
    if (!extra) {
        return;
    }
    if (!extra->handler) {
        return;
    }

    auto log = new MqttLog();
    log->level = level;
    log->str = new string(str);
    extra->handler->post(TYPE_ON_LOG, log);
}


// callback to java
void callback2Java(int what, void* obj, JNIEnv *env, jobject instance) {
    auto clazz = env->GetObjectClass(instance);
    jmethodID methodId = nullptr;

    switch (what) {
        case TYPE_ON_MESSAGE: {
            if (!obj) {
                break;
            }
            methodId = env->GetMethodID(clazz, "onMessage", "(Ljava/lang/String;Ljava/lang/String;)V");
            auto msg = reinterpret_cast<MqttMsg*>(obj);
            jstring topicString = env->NewStringUTF(msg->topic->c_str());
            jstring payloadString = env->NewStringUTF(msg->payload->c_str());
            env->CallVoidMethod(instance, methodId, topicString, payloadString);
            env->DeleteLocalRef(topicString);
            env->DeleteLocalRef(payloadString);
            delete msg->topic;
            delete msg->payload;
            delete msg;
            break;
        };
        case TYPE_ON_CONNECT: {
            methodId = env->GetMethodID(clazz, "onConnect", "()V");
            env->CallVoidMethod(instance, methodId);
            break;
        }
        case TYPE_ON_CONNECT_WITH_FLAG: {
            methodId = env->GetMethodID(clazz, "onConnectWithFlag", "(I)V");
            auto log = reinterpret_cast<MqttConnectFlag*>(obj);
            int flags = log->flags;
            env->CallVoidMethod(instance, methodId, flags);
            delete log;
            break;
        }
        case TYPE_ON_DISCONNECT: {
            methodId = env->GetMethodID(clazz, "onDisconnect", "()V");
            env->CallVoidMethod(instance, methodId);
            break;
        }
        case TYPE_ON_PUBLISH: {
            methodId = env->GetMethodID(clazz, "onPublish", "()V");
            env->CallVoidMethod(instance, methodId);
            break;
        }
        case TYPE_ON_SUBSCRIBE: {
            methodId = env->GetMethodID(clazz, "onSubscribe", "()V");
            env->CallVoidMethod(instance, methodId);
            break;
        }
        case TYPE_ON_UNSUBSCRIBE: {
            methodId = env->GetMethodID(clazz, "onUnsubscribe", "()V");
            env->CallVoidMethod(instance, methodId);
            break;
        }
        case TYPE_ON_LOG: {
            if (!obj) {
                return;
            }
            methodId = env->GetMethodID(clazz, "onLog", "(ILjava/lang/String;)V");
            auto log = reinterpret_cast<MqttLog*>(obj);
            jstring msgString = env->NewStringUTF(log->str->c_str());
            env->CallVoidMethod(instance, methodId, log->level, msgString);
            env->DeleteLocalRef(msgString);
            delete log->str;
            delete log;
            break;
        }
        default:
            break;
    }
    env->DeleteLocalRef(clazz);
}


