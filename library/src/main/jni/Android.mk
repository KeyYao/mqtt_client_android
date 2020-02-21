LOCAL_PATH := $(call my-dir)

# ssl lib
include $(CLEAR_VARS)
LOCAL_MODULE    := ssl
LOCAL_SRC_FILES := lib/$(TARGET_ARCH_ABI)/libssl.a
include $(PREBUILT_STATIC_LIBRARY)

# crypto lib
include $(CLEAR_VARS)
LOCAL_MODULE    := ssl_crypto
LOCAL_SRC_FILES := lib/$(TARGET_ARCH_ABI)/libcrypto.a
include $(PREBUILT_STATIC_LIBRARY)

# build flag
include $(CLEAR_VARS)
LOCAL_CFLAGS += -fexceptions
LOCAL_CPPFLAGS += -std=c++11
LOCAL_LDLIBS += -llog -landroid

# include
LOCAL_C_INCLUDES += $(LOCAL_PATH)
LOCAL_C_INCLUDES += $(LOCAL_PATH)/openssl/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/uthash/src
LOCAL_C_INCLUDES += $(LOCAL_PATH)/mosquitto
LOCAL_C_INCLUDES += $(LOCAL_PATH)/mosquitto/lib
LOCAL_C_INCLUDES += $(LOCAL_PATH)/mosquitto/lib/cpp
LOCAL_C_INCLUDES += $(LOCAL_PATH)/src

# mosquitto lib
LOCAL_CFLAGS += -DWITH_SOCKS
LOCAL_CFLAGS += -DWITH_EC
LOCAL_CFLAGS += -DWITH_UUID
LOCAL_CFLAGS += -DWITH_SYS_TREE
LOCAL_CFLAGS += -DWITH_MENORY_TRACKING
LOCAL_CFLAGS += -DWITH_PERSISTENCE
LOCAL_CFLAGS += -DWITH_BRIDGE
LOCAL_CFLAGS += -DWITH_THREADING
LOCAL_CFLAGS += -DWITH_TLS_PSK
LOCAL_CFLAGS += -DWITH_TLS

LOCAL_SRC_FILES += mosquitto/lib/actions.c
LOCAL_SRC_FILES += mosquitto/lib/alias_mosq.c
LOCAL_SRC_FILES += mosquitto/lib/callbacks.c
LOCAL_SRC_FILES += mosquitto/lib/connect.c
LOCAL_SRC_FILES += mosquitto/lib/handle_auth.c
LOCAL_SRC_FILES += mosquitto/lib/handle_connack.c
LOCAL_SRC_FILES += mosquitto/lib/handle_disconnect.c
LOCAL_SRC_FILES += mosquitto/lib/handle_ping.c
LOCAL_SRC_FILES += mosquitto/lib/handle_pubackcomp.c
LOCAL_SRC_FILES += mosquitto/lib/handle_publish.c
LOCAL_SRC_FILES += mosquitto/lib/handle_pubrec.c
LOCAL_SRC_FILES += mosquitto/lib/handle_pubrel.c
LOCAL_SRC_FILES += mosquitto/lib/handle_suback.c
LOCAL_SRC_FILES += mosquitto/lib/handle_unsuback.c
LOCAL_SRC_FILES += mosquitto/lib/helpers.c
LOCAL_SRC_FILES += mosquitto/lib/logging_mosq.c
LOCAL_SRC_FILES += mosquitto/lib/loop.c
LOCAL_SRC_FILES += mosquitto/lib/memory_mosq.c
LOCAL_SRC_FILES += mosquitto/lib/messages_mosq.c
LOCAL_SRC_FILES += mosquitto/lib/mosquitto.c
LOCAL_SRC_FILES += mosquitto/lib/net_mosq_ocsp.c
LOCAL_SRC_FILES += mosquitto/lib/net_mosq.c
LOCAL_SRC_FILES += mosquitto/lib/options.c
LOCAL_SRC_FILES += mosquitto/lib/packet_datatypes.c
LOCAL_SRC_FILES += mosquitto/lib/packet_mosq.c
LOCAL_SRC_FILES += mosquitto/lib/property_mosq.c
LOCAL_SRC_FILES += mosquitto/lib/read_handle.c
LOCAL_SRC_FILES += mosquitto/lib/send_connect.c
LOCAL_SRC_FILES += mosquitto/lib/send_disconnect.c
LOCAL_SRC_FILES += mosquitto/lib/send_mosq.c
LOCAL_SRC_FILES += mosquitto/lib/send_publish.c
LOCAL_SRC_FILES += mosquitto/lib/send_subscribe.c
LOCAL_SRC_FILES += mosquitto/lib/send_unsubscribe.c
LOCAL_SRC_FILES += mosquitto/lib/socks_mosq.c
LOCAL_SRC_FILES += mosquitto/lib/srv_mosq.c
LOCAL_SRC_FILES += mosquitto/lib/thread_mosq.c
LOCAL_SRC_FILES += mosquitto/lib/time_mosq.c
LOCAL_SRC_FILES += mosquitto/lib/tls_mosq.c
LOCAL_SRC_FILES += mosquitto/lib/utf8_mosq.c
LOCAL_SRC_FILES += mosquitto/lib/util_mosq.c
LOCAL_SRC_FILES += mosquitto/lib/util_topic.c
LOCAL_SRC_FILES += mosquitto/lib/will_mosq.c

LOCAL_SRC_FILES += mosquitto_wrapper.cpp
LOCAL_SRC_FILES += JNIEnvHandler.cpp
LOCAL_SRC_FILES += MqttClient.cpp

# link lib
LOCAL_STATIC_LIBRARIES := ssl ssl_crypto

# module name
LOCAL_MODULE := mqtt

include $(BUILD_SHARED_LIBRARY)
