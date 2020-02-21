//
// Created by Key.Yao on 2020-02-21.
//

#include "mosquitto_wrapper.h"

#define UNUSED(A) (void)(A)

namespace mqttclient {

    static void on_connect_wrapper(struct mosquitto *mosq, void *userdata, int rc)
    {
        auto *m = (class mosquitto_wrapper *)userdata;
        UNUSED(mosq);
        if (m->on_connect_callback) {
            m->on_connect_callback(m, rc);
        }
    }

    static void on_connect_with_flags_wrapper(struct mosquitto *mosq, void *userdata, int rc, int flags)
    {
        auto *m = (class mosquitto_wrapper *)userdata;
        UNUSED(mosq);
        if (m->on_connect_with_flag_callback) {
            m->on_connect_with_flag_callback(m, rc, flags);
        }
    }

    static void on_disconnect_wrapper(struct mosquitto *mosq, void *userdata, int rc)
    {
        auto *m = (class mosquitto_wrapper *)userdata;
        UNUSED(mosq);
        if (m->on_disconnect_callback) {
            m->on_disconnect_callback(m, rc);
        }
    }

    static void on_publish_wrapper(struct mosquitto *mosq, void *userdata, int mid)
    {
        auto *m = (class mosquitto_wrapper *)userdata;
        UNUSED(mosq);
        if (m->on_publish_callback) {
            m->on_publish_callback(m, mid);
        }
    }

    static void on_message_wrapper(struct mosquitto *mosq, void *userdata, const struct mosquitto_message *message)
    {
        auto *m = (class mosquitto_wrapper *)userdata;
        UNUSED(mosq);
        if (m->on_message_callback) {
            m->on_message_callback(m, message);
        }
    }

    static void on_subscribe_wrapper(struct mosquitto *mosq, void *userdata, int mid, int qos_count, const int *granted_qos)
    {
        auto *m = (class mosquitto_wrapper *)userdata;
        UNUSED(mosq);
        if (m->on_subscribe_callback) {
            m->on_subscribe_callback(m, mid, qos_count, granted_qos);
        }
    }

    static void on_unsubscribe_wrapper(struct mosquitto *mosq, void *userdata, int mid)
    {
        auto *m = (class mosquitto_wrapper *)userdata;
        UNUSED(mosq);
        if (m->on_unsubscribe_callback) {
            m->on_unsubscribe_callback(m, mid);
        }
    }


    static void on_log_wrapper(struct mosquitto *mosq, void *userdata, int level, const char *str)
    {
        auto *m = (class mosquitto_wrapper *)userdata;
        UNUSED(mosq);
        if (m->on_log_callback) {
            m->on_log_callback(m, level, str);
        }
    }


    int lib_init()
    {
        return mosquitto_lib_init();
    }

    int lib_cleanup()
    {
        return mosquitto_lib_cleanup();
    }

    const char* strerror(int mosq_errno)
    {
        return mosquitto_strerror(mosq_errno);
    }

    mosquitto_wrapper::mosquitto_wrapper(const char *id, bool clean_session)
    {
        m_mosq = mosquitto_new(id, clean_session, this);
        mosquitto_connect_callback_set(m_mosq, on_connect_wrapper);
        mosquitto_connect_with_flags_callback_set(m_mosq, on_connect_with_flags_wrapper);
        mosquitto_disconnect_callback_set(m_mosq, on_disconnect_wrapper);
        mosquitto_publish_callback_set(m_mosq, on_publish_wrapper);
        mosquitto_message_callback_set(m_mosq, on_message_wrapper);
        mosquitto_subscribe_callback_set(m_mosq, on_subscribe_wrapper);
        mosquitto_unsubscribe_callback_set(m_mosq, on_unsubscribe_wrapper);
        mosquitto_log_callback_set(m_mosq, on_log_wrapper);
    }

    mosquitto_wrapper::~mosquitto_wrapper()
    {
        mosquitto_destroy(m_mosq);
    }

    int mosquitto_wrapper::connect(const char *host, int port, int keepalive) {
        return mosquitto_connect(m_mosq, host, port, keepalive);
    }

    int mosquitto_wrapper::reconnect() {
        return mosquitto_reconnect(m_mosq);
    }

    int mosquitto_wrapper::disconnect() {
        return mosquitto_disconnect(m_mosq);
    }

    int mosquitto_wrapper::publish(int *mid, const char *topic, int payloadlen, const void *payload,
                                   int qos, bool retain) {
        return mosquitto_publish(m_mosq, mid, topic, payloadlen, payload, qos, retain);
    }

    int mosquitto_wrapper::subscribe(int *mid, const char *sub, int qos) {
        return mosquitto_subscribe(m_mosq, mid, sub, qos);
    }

    int mosquitto_wrapper::unsubscribe(int *mid, const char *sub) {
        return mosquitto_unsubscribe(m_mosq, mid, sub);
    }

    int mosquitto_wrapper::username_pw_set(const char *username, const char *password) {
        return mosquitto_username_pw_set(m_mosq, username, password);
    }

    int mosquitto_wrapper::tls_set(const char *cafile, const char *capath, const char *certfile,
                                   const char *keyfile,
                                   int (*pw_callback)(char *, int, int, void *)) {
        return mosquitto_tls_set(m_mosq, cafile, capath, certfile, keyfile, pw_callback);
    }

    int mosquitto_wrapper::tls_opts_set(int cert_reqs, const char *tls_version, const char *ciphers) {
        return mosquitto_tls_opts_set(m_mosq, cert_reqs, tls_version, ciphers);
    }

    int mosquitto_wrapper::tls_insecure_set(bool value) {
        return mosquitto_tls_insecure_set(m_mosq, value);
    }

    int mosquitto_wrapper::loop(int timeout, int max_packets) {
        return mosquitto_loop(m_mosq, timeout, max_packets);
    }


}

