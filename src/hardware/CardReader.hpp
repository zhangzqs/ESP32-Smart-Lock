#pragma once

#include <Adafruit_PN532.h> //PN532驱动
#include <Wire.h>           //I2C总线驱动
#include <Arduino.h>
#include <FunctionalInterrupt.h>    //函数式中断服务函数

/**
 * @brief 读卡器的硬件驱动
 */
class CardReader{
private:
    Adafruit_PN532 *pn532;
    uint8_t irq;
    
    //UID数组和长度
    uint8_t *uid;
    uint8_t uidLength;

public:

    /**
     * @brief 读卡器的构造函数
     * 
     * @param sda I2C的SDA数据引脚
     * @param scl I2C的SCL时钟引脚
     * @param irq IRQ中断引脚
     * @param rst RST复位引脚
     */
    explicit CardReader(uint8_t sda,uint8_t scl,uint8_t irq,uint8_t rst):irq(irq){
        //Wire.begin(sda,scl);    //初始化I2C总线
        pn532 = new Adafruit_PN532(irq,rst);    //使用I2C总线与PN532通信
        Wire.begin(sda,scl);    //初始化I2C总线
        pn532->begin(); //初始化PN532
        uid = new uint8_t[6];   //初始化长度为6的UID数组(实际大多数情况只有四个)

        if(!PN532_Online()){
            Serial.println("找不到PN532读卡器");
        }

        uint32_t versionData = getFirmwareVersion();
        Serial.print("找到芯片PN532");
        Serial.println((versionData >> 24) & 0xFF, HEX);
        Serial.print("Firmware ver. ");
        Serial.print((versionData >> 16) & 0xFF, DEC);
        Serial.print('.');
        Serial.println((versionData >> 8) & 0xFF, DEC);

        pn532->SAMConfig(); //配置PN532去读取RFID标签

        
        Serial.println("开始监听卡片！");
        startListening();
    }

    /**
     * @brief 获取PN532的固件版本数据
     * 
     * @return uint32_t 以uint32_t类型返回固件版本
     */
    uint32_t getFirmwareVersion(){
        return pn532->getFirmwareVersion();
    }

    /**
     * @brief 检查PN532读卡器是否连接成功
     * @return bool 读卡器在线状态
     */
    bool PN532_Online(){
        return getFirmwareVersion() != 0;
    }

protected:
    virtual void onCardDetected(uint32_t uid){
        Serial.printf("检测到一张卡片！%d \n",uid);
    }

private:
    bool detected = false;
    //中断服务函数
    std::function<void()> serveFunction = [&](){
        //Serial.println("中断服务函数被调用");
        //当读到卡时，立马置detected值为true
        detected = true;
        attachInterrupt(irq,serveFunction,FALLING);
    };

    /**
     * @brief 开始监听卡片
     * 
     */
    void startListening(){
        pinMode(irq,INPUT | PULLUP);
        digitalWrite(irq,HIGH);
        //开启检测M1卡片
        pn532->startPassiveTargetIDDetection(PN532_MIFARE_ISO14443A);
        
        //Serial.println("注册中断服务函数");
        //注册中断服务函数
        attachInterrupt(irq,serveFunction,FALLING);
    }

private:
    //默认刷卡间隔为1000ms
    int DELAY_BETWEEN_CARDS = 1000;

    /**
     * @brief 设置在相邻两张卡之间的读卡间隔
     * 
     * @param ms 毫秒数
     */
    void setDelayBetweenCards(int ms){
        DELAY_BETWEEN_CARDS = ms;
    }

private:
    bool readerDisabled = false;
    long timeLastCardRead = 0;

public:

    void handle(){
        //如果读卡器被禁用了，说明之前已经刷过卡了
        //此时，如距离上次刷卡时间超过某个值时，就再次启用监听
        if(readerDisabled){
            if(millis()-timeLastCardRead > DELAY_BETWEEN_CARDS){
                readerDisabled = false;
                startListening();
            }
        }

        if(detected){
            bool success = pn532->readDetectedPassiveTargetID(uid,&uidLength);
            if(success){    //如果成功检测到了
                if(uidLength == 4){
                    //如果UID长度为4
                    uint32_t cardId = uid[0];
                    cardId <<= 8;
                    cardId |= uid[1];
                    cardId <<= 8;
                    cardId |= uid[2];
                    cardId <<= 8;
                    cardId |= uid[3];
                    onCardDetected(cardId);
                }
                timeLastCardRead = millis();    //上一次检测到卡的时间
            }
            readerDisabled = true;  //如果检测到一次卡，那么就禁用读卡器
            detected = false;
        }
    }
};