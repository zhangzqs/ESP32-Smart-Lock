#pragma once
#include <PubSubClient.h>
#include <WiFi.h>
#include "IHandlable.hpp"
#include <ArduinoJson.h>
class MqttClientHandle:public IHandlable{
protected:
    String clientId;    //客户端唯一ID信息
    
    virtual void onReceiveMsg(String topic,String message) {
        Serial.println("收到消息");
        Serial.printf("Topic:%s\nMessage:%s\n",topic.c_str(),message.c_str());
    }

    
private:
    WiFiClient tcpClient;  //TCP客户端
    PubSubClient mqttClient;    //MQTT客户端

    const String willTopic = "/smartlock/server/device_status";
    String offlineMsg;  //离线消息json字符串
    String onlineMsg;   //在线消息json字符串

public:
    explicit MqttClientHandle():mqttClient(tcpClient){
        //生成唯一客户端ID
        clientId = getClientId();

        StaticJsonDocument<200> doc;
        doc["device_id"] = clientId;
        doc["status"]="online";
        serializeJson(doc,onlineMsg);   //生成在线消息字符串

        doc["status"]="offline";
        serializeJson(doc,offlineMsg);  //生成离线消息字符串
    }

    /**
     * @brief 开始连接mqtt服务器
     * 
     * @param server 服务器，默认为公有mqtt服务器
     * @param port 端口号，默认为1883
     */
    void begin(String server = "test.ranye-iot.net",uint16_t port = 1883){
        mqttClient.setServer(server.c_str(), port);
        mqttClient.setCallback([this](char* topic, uint8_t* payload, uint32_t length){
            auto cs = new char[length+1];
            cs[length] = '\0';
            memcpy(cs,payload,length);
            this->onReceiveMsg(topic,cs);
            delete[] cs;
        });
        mqttClient.setKeepAlive(15);    //MQTT心跳15秒

        bool isConnected = mqttClient.connect(
            clientId.c_str(),                    //客户端ID
            willTopic.c_str(),     //遗嘱话题名
            MQTTQOS0,     //遗嘱Qos
            true,   //
            offlineMsg.c_str()      //遗嘱消息
        );
        if (isConnected) {
            Serial.println("MQTT Server connected.");
            Serial.printf("ClientId: %s\n", clientId.c_str());
            publishOnlineMsg();
        } else {
            Serial.printf("MQTT Server connect failed. Client state:%d\n", mqttClient.state());
        }
    }
    

    /**
     * @brief 订阅Mqtt话题
     * 
     * @param topicString 订阅的话题名
     */
    void subscribeMsg(String topicString)
    {
        if (mqttClient.subscribe(topicString.c_str())) {
            Serial.printf("Subscribe Topic:%s Successful!\n", topicString.c_str());
        } else {
            Serial.printf("Subscribe Topic:%s failed!\n",topicString.c_str());
        }
    }

    /**
     * @brief 发布当前设备的在线消息
     * 
     */
    void publishOnlineMsg(){
        if (mqttClient.publish(willTopic.c_str(), onlineMsg.c_str())) {
            Serial.println("Online message published.");
        } else {
            Serial.println("Failed to publish online message");
        }
    }

    /**
     * @brief 发布Mqtt消息
     * 
     * @param topic 消息话题
     * @param message 消息内容
     */
    void publishMsg(String& topic, String& message)
    {
        if (mqttClient.publish(topic.c_str(), message.c_str())) {
            Serial.printf("Publish topic: %s\n", topic.c_str());
            Serial.printf("Publish message: %s\n", message.c_str());
            publishOnlineMsg();
        } else {
            Serial.printf("Publish Failed Topic: %s\n",topic.c_str());
        }
    }

    static String getClientId(){
        uint8_t mac[6];
        char macStr[18] = { 0 };
        if(WiFiGenericClass::getMode() == WIFI_MODE_NULL){
            esp_read_mac(mac, ESP_MAC_WIFI_STA);
        }
        else{
            esp_wifi_get_mac(WIFI_IF_STA, mac);
        }
        sprintf(macStr, "%02X%02X%02X%02X%02X%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
        return String("ESP32-SmartLock-")+macStr;
    }

    void handle() override{
        if (mqttClient.connected()) {
            mqttClient.loop();
        } else {
            if(WiFi.isConnected()){ //如果WiFi已连接
                begin();    //尝试重连
            }
        }
    }
};

class LockServer:public MqttClientHandle{
protected:
    std::function<void(String,String)> callback;
    void onReceiveMsg(String topic,String message)override{
        MqttClientHandle::onReceiveMsg(topic,message);
        callback(topic,message);
    }
public:
    explicit LockServer() = default;

    void attachCallback(decltype(callback) callback){
        this->callback = std::move(callback);
    }

    /**
     * @brief 发布卡片的UID信息
     * 
     * @param uid 
     */
    void publishUID(uint32_t uid)
    {
        char uid_cstr[10];
        sprintf(uid_cstr,"%08x",uid);
        StaticJsonDocument<200> obj;
        obj["device_id"] = clientId;
        obj["uid"] = uid_cstr;

        String topicString = "/smartlock/server/uid";
        String messageString;
        serializeJson(obj,messageString);
        
        publishMsg(topicString, messageString);
    }

    void publishButton(String type)
    {
        StaticJsonDocument<200> obj;
        obj["device_id"] = clientId;
        obj["type"] = type;

        String topicString = "/smartlock/server/button";
        String messageString;
        serializeJson(obj,messageString);
        
        publishMsg(topicString, messageString);
    }
};