//
// Created by Key.Yao on 2020-02-20.
//

#ifndef MQTT_CLIENT_ANDROID_JNIENVHANDLER_H
#define MQTT_CLIENT_ANDROID_JNIENVHANDLER_H

#include <jni.h>

#include <thread>
#include <queue>
#include <atomic>
#include <condition_variable>

typedef void(*handleMessage)(int what, void* obj, JNIEnv *env, jobject instance);

struct ThreadMessage;

class JNIEnvHandler {

public:
    JNIEnvHandler(const char* threadName);

    ~JNIEnvHandler();

    bool init(JavaVM *vm, jobject instance, handleMessage callback);

    void exit();

    std::thread::id getThreadId();

    void post(int what, void *obj = nullptr);


private:
    JNIEnvHandler(const JNIEnvHandler&);
    JNIEnvHandler&operator=(const JNIEnvHandler&);

    void process();

    JavaVM *_vm;
    jobject _instance;
    std::thread *_thread;
    std::queue<ThreadMessage*> _queue;
    std::mutex _mutex;
    std::condition_variable _cv;
    std::atomic<bool> _exitFlag;
    handleMessage _callback;
    const char* _threadName;

};


#endif //MQTT_CLIENT_ANDROID_JNIENVHANDLER_H
