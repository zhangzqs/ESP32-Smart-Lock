#include <Arduino.h>

#include "HandlableTasks.hpp"
#include "MqttClientHandle.hpp"
#include "OtaUploadHandle.hpp"
#include "WiFiManagerHandle.hpp"
#include "hardware/Jy61.hpp"
#include "hardware/Lock.hpp"
#include "hardware/OneButtonHandle.hpp"
#include "hardware/RC522_CallbackCardReader.hpp"
#include <SPI.h>
OneButtonHandle* key;
Lock* lock;
RC522_CallbackCardReader* reader;
WiFiManagerHandle* wmh;
LockServer* ls;
Jy61* jy61;
OtaUploadHandle* ota;

HandlableTasks hts;

// 初始化所有
void Initialize()
{
    Serial.begin(115200);
    Serial2.begin(115200);
    key = new OneButtonHandle(15);
    lock = new Lock(4);
    reader = new RC522_CallbackCardReader(5, 2);
    wmh = WiFiManagerHandle::getInstance();
    ls = new LockServer();
    jy61 = new Jy61(Serial2);
    ota = new OtaUploadHandle();
    hts.addHandlable(key);
    hts.addHandlable(lock);
    hts.addHandlable(reader);
    hts.addHandlable(wmh);
    hts.addHandlable(ls);
    hts.addHandlable(jy61);
    hts.addHandlable(ota);
}
//注册mqtt路由
void RegisterForMqttRouter()
{
    String prefix = "/smartlock/lock/";
    prefix += ls->getClientId();
    ls->subscribeMsg(prefix + "/servo");
    ls->subscribeMsg(prefix + "/led");
    ls->subscribeMsg(prefix + "/sound");
    ls->subscribeMsg(prefix + "/esp");
}

void onButtonClicked()
{
    lock->unlock();
    Serial.println("Button Clicked");
    ls->publishButton("click");
}

void onButtonDoubleClicked()
{
    Serial.println("Button Double Clicked");
    ls->publishButton("double_click");
}

void onButtonLongPressed()
{
    Serial.println("Button Long Pressed");
    ls->publishButton("long_press");
    lock->open();
}

void onCardSensed(uint32_t uid)
{
    ls->publishUID(uid);
}

void onWiFiSuccess()
{
    Serial.println("WiFi 链接成功");
    ls->begin(); //开始连接mqtt服务器
    RegisterForMqttRouter();
}

void onReceive(const String& topic, const String& message)
{
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, message);
    JsonObject obj = doc.as<JsonObject>();
    if (topic.endsWith("servo")) {
        int agree = obj["agree"].as<int>();
        switch (agree) {
        case -3:
            lock->open();
            break;
        case -2:
            lock->close();
            break;
        case -1:
            lock->unlock();
            break;
        default:
            lock->write(agree);
            break;
        }
    } else if (topic.endsWith("led")) {
        int bv = obj["brightness_value"].as<int>();
        Serial.printf("调节亮度: %d%\n", bv);
    } else if (topic.endsWith("sound")) {
        Serial.println("声音");
    } else if (topic.endsWith("esp")) {
        String msg = obj["msg"].as<String>();
        if (msg.equals("restart")) {
            Serial.println("程序即将重启");
            ESP.restart();
        }
    }
}

void SetupAllCallback()
{
    key->attachClick(onButtonClicked);
    key->attachDoubleClick(onButtonDoubleClicked);
    key->attachLongPressStart(onButtonLongPressed);

    reader->setCardCallback(onCardSensed);

    wmh->attachSuccessCallback(onWiFiSuccess);

    ls->attachCallback(onReceive);
}

//定时发布姿态信息
Ticker posePubTicker([]() {
    ls->publishPose(jy61);
},
    15000, 0, MILLIS);

#include "WiFiManagerHandle.hpp"
void setup()
{
    Initialize();
    SetupAllCallback();
    WiFiManagerHandle::getInstance()->begin(); //开始联网
    posePubTicker.start();
}

void loop()
{
    hts.handle();
    posePubTicker.update();
}