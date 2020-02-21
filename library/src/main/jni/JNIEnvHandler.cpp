//
// Created by Key.Yao on 2020-02-20.
//

#include "JNIEnvHandler.h"

using namespace std;

struct ThreadMessage {
    ThreadMessage(int i, void* m, bool flag) {
        what = i;
        msg = m;
        exit = flag;
    }
    int what;
    void* msg;
    bool exit;
};

JNIEnvHandler::JNIEnvHandler(const char *threadName) {
    _threadName = threadName;
    _thread = nullptr;
    _exitFlag = false;
    _vm = nullptr;
    _callback = nullptr;
}

JNIEnvHandler::~JNIEnvHandler() {
    _thread = nullptr;
    _callback = nullptr;
    _instance = nullptr;
    _vm = nullptr;
}

bool JNIEnvHandler::init(JavaVM *vm, jobject instance, handleMessage callback) {
    if (!_thread) {
        _thread = new thread(&JNIEnvHandler::process, this);
    }
    _vm = vm;
    _instance = instance;
    _callback = callback;
    return true;
}

void JNIEnvHandler::exit() {
    if (!_thread) {
        return;
    }

    auto *msg = new ThreadMessage(-1, nullptr, true);

    {
        lock_guard<mutex> lock(_mutex);
        _queue.push(msg);
        _cv.notify_one();
    }

    _thread->join();
    delete _thread;
    _thread = nullptr;
    _exitFlag = true;
}

std::thread::id JNIEnvHandler::getThreadId() {
    return _thread->get_id();
}

void JNIEnvHandler::post(int what, void *obj) {
    auto *msg = new ThreadMessage(what, obj, false);
    unique_lock<mutex> lock(_mutex);
    _queue.push(msg);
    _cv.notify_one();
}

void JNIEnvHandler::process() {
    _exitFlag = false;

    JNIEnv *env = nullptr;
    if (_vm) {
        _vm->AttachCurrentThread(&env, nullptr);
    }

    while (true) {
        ThreadMessage *msg = nullptr;
        {
            unique_lock<mutex> lock(_mutex);
            while (_queue.empty()) {
                _cv.wait(lock);
            }

            if (_queue.empty()) {
                continue;
            }

            msg = _queue.front();
            _queue.pop();

        }

        if (msg->exit) {
            delete msg;
            break;
        }

        if (env && _instance) {
            _callback(msg->what, msg->msg, env, _instance);
        }

        delete msg;
    }

    env->DeleteGlobalRef(_instance);

    if (_vm) {
        _vm->DetachCurrentThread();
    }
}
