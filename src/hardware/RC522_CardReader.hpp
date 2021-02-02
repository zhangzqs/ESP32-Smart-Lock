#pragma once

#include <MFRC522.h>
#include <SPI.h>           //SPI总线驱动
#include <Arduino.h>
#include <FunctionalInterrupt.h>    //函数式中断服务函数
#include "ICardReader.hpp"

/**
 * @brief 读卡器的硬件驱动
 */
class RC522_CardReader:public ICardReader{
private:
    MFRC522 *rc522;
    uint8_t irq;    //中断引脚
    Ticker *resetTicker;    //定时复位
public:

    /**
     * @brief 读卡器的构造函数
     * @param sda 数据线
     * @param irq IRQ中断引脚
     * @param rst RST复位引脚
     */
    explicit RC522_CardReader(uint8_t sda,uint8_t rst,uint8_t irq):irq(irq){
        rc522 = new MFRC522(sda,rst);    //使用SPI总线与RC522通信
        //SPI.begin(18,19,23,sda);
        resetTicker = new Ticker([this](){
           rc522->PCD_Init();
        },5000,0,MILLIS);   //构造定时器，用于定时初始化读卡器，执行无数次，每次间隔时间5s

        SPI.begin();
        Serial.println("开始监听卡片！");
        startListening();
    }

    virtual ~RC522_CardReader(){
        resetTicker->stop();
        delete resetTicker;
        delete rc522;
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

public:
    /**
     * @brief 开始监听卡片
     * 
     */
    void startListening(){
        pinMode(irq,INPUT | PULLUP);
        digitalWrite(irq,HIGH);
        //Serial.println("注册中断服务函数");

        rc522->PCD_Init(); //初始化PN532
        resetTicker->start();
        //注册中断服务函数
        attachInterrupt(irq,serveFunction,FALLING);
    }

private:
    //默认刷卡间隔为1000ms
    int DELAY_BETWEEN_CARDS = 3000;

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

    void handle() override{
        resetTicker->update();
        //如果读卡器被禁用了，说明之前已经刷过卡了
        //此时，如距离上次刷卡时间超过某个值时，就再次启用监听
        if(readerDisabled){
            if(millis()-timeLastCardRead > DELAY_BETWEEN_CARDS){
                readerDisabled = false;
                startListening();
            }
        }
        Serial.println("RC522 handle");
        //搜索卡片
        if (!rc522->PICC_IsNewCardPresent()) {
            return;
        }
        //验证卡的可读性
        if(!rc522->PICC_ReadCardSerial()){
            return;
        }
        if(detected){
            MFRC522::Uid uid_buffer = rc522->uid;
            uint32_t uid;
            if(uid_buffer.size == 4){
                uid = 
                (uid_buffer.uidByte[0]<<24)|
                (uid_buffer.uidByte[1]<<16)|
                (uid_buffer.uidByte[2]<<8)|
                (uid_buffer.uidByte[3]);
            }else{
                Serial.println("不支持的卡片");
                return;
            }
            
            onCardDetected(uid);
                
            timeLastCardRead = millis();    //上一次检测到卡的时间
            readerDisabled = true;  //如果检测到一次卡，那么就禁用读卡器
            detected = false;
        }
    }
};