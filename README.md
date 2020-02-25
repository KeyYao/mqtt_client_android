# Android Mqtt Client
基于[mosquitto](https://github.com/eclipse/mosquitto)的android mqtt客户端
* 使用[mosquitto/lib](https://github.com/eclipse/mosquitto/tree/master/lib)的代码，用c++封装接口
* 对外提供java接口，简单易用
* 支持OpenSSL加密
* 实现订阅、解除订阅、发布消息等功能

## 接口
MqttClient.java
```java
/**
 * 获取单例
 */
public static MqttClient getInstance()
```
```java 
/**
 * 启动mqtt服务
 */
public void start(@NonNull String host, int port, @NonNull String uuid, boolean clearSession)
public void start(@NonNull String host, int port, @NonNull String uuid, boolean clearSession, @NonNull String caFilePath, @NonNull String username, @NonNull String password) 
```
```java
/**
 * 重连服务
 */
public void reconnect()
```
```java
/**
 * 订阅主题
 */
public void subscribe(String topic)
public void subscribe(String topic, int qos)
public void subscribe(List<String> topicList)
public void subscribe(List<String> topicList, List<Integer> qosList)
```
```java
/**
 * 解除订阅
 */
public void unsubscribe(String topic)
public void unsubscribe(List<String> topicList)
```
```java
/**
 * 发布消息
 */
public void publish(String topic, String message)
public void publish(String topic, String message, int qos)
```
```java
/**
 * mqtt事件回调接口
 */
public interface MqttCallback {
    @WorkerThread
    void onMessage(@NonNull String topic, @NonNull String message);
    @WorkerThread
    void onLog(String str);
}
```
```java
/**
 * 设置mqtt事件回调
 */
public void setOnMqttCallback(@Nullable MqttCallback callback)
```

## 使用
```
- 导入Module: Android Studio -> File -> New -> Import Module -> 选中“library”文件夹

- 修改app/build.gradle文件，添加ndk abiFilters设置
android {
    ...
    defaultConfig {
        ...
        ndk {
            abiFilters 'armeabi-v7a', 'x86', 'arm64-v8a'
        }
    }
}

- 添加依赖
dependencies {
    implementation project(':library')
}
```

## 编译问题
因为默认支持OpenSSL，依赖静态库只添加了三个abi版本，所以项目编译默认只支持```'armeabi-v7a', 'x86', 'arm64-v8a'```，如需支持其他cpu abi，请自行编译OpenSSL静态库
* 把```libssl.a libcrypto.a```文件添加到```/library/src/main/jni/lib```目录下，并用文件夹区分abi类型
* 修改```library/build.gradle```，添加对应的abi类型
* 修改```app/build.gradle```，添加对应的abi类型

## 禁用OpenSSL
修改```/library/src/main/jni/Android.mk```文件，注释掉下面的代码
```
...

# ssl lib
#include $(CLEAR_VARS)
#LOCAL_MODULE    := ssl
#LOCAL_SRC_FILES := lib/$(TARGET_ARCH_ABI)/libssl.a
#include $(PREBUILT_STATIC_LIBRARY)

# crypto lib
#include $(CLEAR_VARS)
#LOCAL_MODULE    := ssl_crypto
#LOCAL_SRC_FILES := lib/$(TARGET_ARCH_ABI)/libcrypto.a
#include $(PREBUILT_STATIC_LIBRARY)

...

#LOCAL_CFLAGS += -DWITH_TLS_PSK
#LOCAL_CFLAGS += -DWITH_TLS

...

# link lib
#LOCAL_STATIC_LIBRARIES := ssl ssl_crypto

...
```

## 例子
app module只实现了简单的调用，相关日志输出到了logcat