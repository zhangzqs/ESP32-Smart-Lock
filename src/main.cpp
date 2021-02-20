#include <Arduino.h>

#include "hardware/OneButtonHandle.hpp"
#include "hardware/Lock.hpp"
#include "hardware/RC522_CallbackCardReader.hpp"
#include "HandlableTasks.hpp"
#include <SPI.h>
#include "WiFiManagerHandle.hpp"
#include "MqttClientHandle.hpp"

OneButtonHandle* key;
Lock* lock;
RC522_CallbackCardReader* reader;
WiFiManagerHandle* wmh;
LockServer* ls;

HandlableTasks hts;

// 初始化所有
void Initialize()
{
    key = new OneButtonHandle(15);
    lock = new Lock(4);
    reader = new RC522_CallbackCardReader(21, 22);
    wmh = WiFiManagerHandle::getInstance();
    ls = new LockServer();
    hts.addHandlable(key);
    hts.addHandlable(lock);
    hts.addHandlable(reader);
    hts.addHandlable(wmh);
    hts.addHandlable(ls);
}
//注册mqtt路由
void RegisterForMqttRouter(){
    String prefix = "/smartlock/lock/";
    prefix += ls->getClientId();
    ls->subscribeMsg(prefix+"/servo");
    ls->subscribeMsg(prefix+"/led");
    ls->subscribeMsg(prefix+"/sound");
    ls->subscribeMsg(prefix+"/esp");
}

void onButtonClicked()
{
    Serial.println("Button Clicked");
    ls->publishButton("click");
    lock->unlock();
}

void onButtonDoubleClicked()
{
    Serial.println("Button Double Clicked");
    ls->publishButton("double_click");
    lock->open();
}

void onButtonLongPressed(){
    Serial.println("Button Long Pressed");
    ls->publishButton("long_press");
    ESP.restart();
}

void onCardSensed(uint32_t uid)
{
    ls->publishUID(uid);
}

void onWiFiSuccess(){
    Serial.println("WiFi 链接成功");
    ls->begin();        //开始连接mqtt服务器
    RegisterForMqttRouter();
}

void onReceive(const String& topic,const String& message){
    DynamicJsonDocument doc(1024);
    deserializeJson(doc,message);
    JsonObject obj = doc.as<JsonObject>();
    if(topic.endsWith("servo")){
        int agree = obj["agree"].as<int>();
        switch (agree)
        {
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
    }else if(topic.endsWith("led")){
        int bv = obj["brightness_value"].as<int>();
        Serial.printf("调节亮度: %d%\n",bv);
    }else if(topic.endsWith("sound")){
        Serial.println("声音");
    }else if(topic.endsWith("esp")){
        String msg = obj["msg"].as<String>();
        if(msg.equals("restart")){
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

#include "WiFiManagerHandle.hpp"
void setup()
{
    Serial.begin(115200);
    Initialize();
    SetupAllCallback();
    WiFiManagerHandle::getInstance()->begin();  //开始联网
}

void loop()
{
    hts.handle();
}