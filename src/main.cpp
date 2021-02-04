#include <Arduino.h>

#include "hardware/CallbackKey.hpp"
#include "hardware/Lock.hpp"
#include "hardware/RC522_CallbackCardReader.hpp"
#include "HandlableTasks.hpp"
#include <PubSubClient.h>
#include <SPI.h>
#include <WiFiManager.h>

CallbackKey* key;
Lock* lock;
RC522_CallbackCardReader* reader;
HandlableTasks hts;

WiFiManager wm;

WiFiClient wifiClient; //TCP客户端
PubSubClient mqttClient(wifiClient); //MQTT客户端

void connectWiFi()
{
    WiFi.mode(WIFI_STA);
    bool success = wm.autoConnect("SmartLock", "88888888");
    if (success) {
        Serial.println("Connect successful");
    } else {
        Serial.println("Connect Failed");
    }
}
void receiveCallback(char* topic, uint8_t* payload, uint32_t length)
{
    Serial.printf("Message received [%s]\n", topic);
    char* message = new char[length + 1];
    message[length] = '\0';
    for (int i = 0; i < length; i++) {
        message[i] = payload[i];
    }
    Serial.printf("Message content [%s]\n", message);
    Serial.printf("Message Length %d\n", length);
    lock->unlock();
}
void SubMqttMsg()
{
    String topicString = "Unlock";
    if (mqttClient.subscribe(topicString.c_str())) {
        Serial.printf("Subscribe Topic:%s \n", topicString.c_str());
    } else {
        Serial.println("Subscribe fail");
    }
}

void PubMqttMsg(String& topic, String& message)
{
    if (mqttClient.publish(topic.c_str(), message.c_str())) {
        Serial.printf("Publish topic: %s\n", topic.c_str());
        Serial.printf("Publish message: %s\n", message.c_str());
    } else {
        Serial.println("Publish Failed");
    }
}

void PubUID(uint32_t uid)
{
    String topicString = "UID";
    String messageString = "";
    messageString += uid;
    PubMqttMsg(topicString, messageString);
}

void connectMQTTServer()
{
    //生成唯一客户端ID
    String clientId = "ESP32-" + WiFi.macAddress();
    mqttClient.setServer("test.ranye-iot.net", 1883);
    mqttClient.setCallback(receiveCallback);
    if (mqttClient.connect(clientId.c_str())) {
        Serial.println("MQTT Server connected.");
        Serial.printf("ClientId: %s\n", clientId.c_str());
        SubMqttMsg();
    } else {
        Serial.printf("MQTT Server connect failed. Client state:%d\n", mqttClient.state());
    }
}

void Initialize()
{
    key = new CallbackKey(15);
    lock = new Lock(4);
    reader = new RC522_CallbackCardReader(21, 22);
    hts.addHandlable(key);
    hts.addHandlable(lock);
    hts.addHandlable(reader);
}

void onKeyDown()
{
    Serial.println("Key Down");
    lock->unlock();
}
void onKeyUp()
{
    Serial.println("Key Up");
    //reader->initRC522();
}

void onCardSensed(uint32_t uid)
{
    Serial.printf("UID: %d \n", uid);
    PubUID(uid);
}

void SetupAllCallback()
{
    key->setOnKeyDownCallback(onKeyDown);
    key->setOnKeyUpCallback(onKeyUp);

    reader->setCardCallback(onCardSensed);
}
void setup()
{
    Serial.begin(115200);
    Initialize();
    SetupAllCallback();
    connectWiFi();
    connectMQTTServer();
}

void loop()
{
    hts.handle();
    if (mqttClient.connected()) {
        mqttClient.loop();
    } else {
        connectMQTTServer();
        delay(1000);
    }
}