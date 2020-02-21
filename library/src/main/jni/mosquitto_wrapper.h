//
// Created by Key.Yao on 2020-02-21.
//

#ifndef MQTT_CLIENT_ANDROID_MOSQUITTO_WRAPPER_H
#define MQTT_CLIENT_ANDROID_MOSQUITTO_WRAPPER_H

#include "mosquitto.h"

namespace mqttclient {

    typedef void(*MOSQ_CONNECT_CALLBACK)(void *instance, int rc);
    typedef void(*MOSQ_CONNECT_WITH_FLAG_CALLBACK)(void *instance, int rc, int flags);
    typedef void(*MOSQ_DISCONNECT_CALLBACK)(void *instance, int rc);
    typedef void(*MOSQ_PUBLISH_CALLBACK)(void *instance, int mid);
    typedef void(*MOSQ_MESSAGE_CALLBACK)(void *instance, const struct mosquitto_message * message);
    typedef void(*MOSQ_SUBSCRIBE_CALLBACK)(void *instance, int mid, int qos_count, const int * granted_qos);
    typedef void(*MOSQ_UNSUBSCRIBE_CALLBACK)(void *instance, int mid);
    typedef void(*MOSQ_LOG_CALLBACK)(void *instance, int level, const char * str);

    int lib_init();
    int lib_cleanup();
    const char * strerror(int mosq_errno);

    class mosquitto_wrapper {

    public:
        mosquitto_wrapper(const char *id = NULL, bool clean_session = true);
        ~mosquitto_wrapper();

        // 附加数据
        void *extra;

        MOSQ_CONNECT_CALLBACK on_connect_callback = NULL;
        MOSQ_CONNECT_WITH_FLAG_CALLBACK  on_connect_with_flag_callback = NULL;
        MOSQ_DISCONNECT_CALLBACK on_disconnect_callback = NULL;
        MOSQ_PUBLISH_CALLBACK on_publish_callback = NULL;
        MOSQ_MESSAGE_CALLBACK on_message_callback = NULL;
        MOSQ_SUBSCRIBE_CALLBACK  on_subscribe_callback = NULL;
        MOSQ_UNSUBSCRIBE_CALLBACK on_unsubscribe_callback = NULL;
        MOSQ_LOG_CALLBACK on_log_callback = NULL;


        int connect(const char *host, int port = 1883, int keepalive = 60);
        int reconnect();
        int disconnect();
        int publish(int *mid, const char *topic, int payloadlen = 0, const void *payload = NULL, int qos = 0, bool retain = false);
        int subscribe(int *mid, const char *sub, int qos = 0);
        int unsubscribe(int *mid, const char *sub);
        int username_pw_set(const char *username, const char *password = NULL);
        int tls_set(const char *cafile, const char *capath = NULL, const char *certfile = NULL, const char *keyfile = NULL, int (*pw_callback)(char *buf, int size, int rwflag, void *userdata) = NULL);
        int tls_opts_set(int cert_reqs, const char *tls_version = NULL, const char *ciphers = NULL);
        int tls_insecure_set(bool value);

        int loop(int timeout = -1, int max_packets = 1);

    private:
        struct mosquitto *m_mosq;
    };

}


#endif //MQTT_CLIENT_ANDROID_MOSQUITTO_WRAPPER_H
