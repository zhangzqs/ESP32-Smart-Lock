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
    Ticker *resetTicker;    //复位定时器
    Ticker *betweenCardTicker; //刷卡间隔的定时器

public:

    /**
     * @brief 读卡器的构造函数
     * @param sda 数据线
     * @param rst RST复位引脚
     */
    explicit RC522_CardReader(uint8_t sda,uint8_t rst){
        rc522 = new MFRC522(sda,rst);    //使用SPI总线与RC522通信
        resetTicker = new Ticker([this](){
           initRC522();
           Serial.println("RC522_Init");
        },60000,0,MILLIS);   //构造定时器，用于定时初始化读卡器，执行无数次，每次间隔时间60s

        betweenCardTicker = new Ticker([this](){
            startListening = true;   //只要时间一到，就重新开始监听
            betweenCardTicker->stop();
        },3000,1,MILLIS);//默认刷卡间隔为3000ms

        SPI.begin();
        Serial.println("开始监听卡片！");
        initRC522();
        resetTicker->start();
    }

    virtual ~RC522_CardReader(){
        resetTicker->stop();
        betweenCardTicker->stop();
        delete resetTicker;
        delete betweenCardTicker;
        //delete rc522;
    }
public:
    void initRC522(){
        rc522->PCD_Reset();
        rc522->PCD_Init();
    }

public:
    /**
     * @brief 设置在相邻两张卡之间的读卡间隔
     * 
     * @param ms 毫秒数
     */
    void setDelayBetweenCards(int ms){
        betweenCardTicker->interval(ms);
    }

private:
    bool startListening = true;
public:

    void handle() override{
        resetTicker->update();
        betweenCardTicker->update();

        if(!startListening){
            //如果没在运行，就开始启用监听
            if(betweenCardTicker->state() != RUNNING){
                betweenCardTicker->start();
            }
            return; //没有启用监听就直接退出
        }

        //搜索卡片
        if (!rc522->PICC_IsNewCardPresent()) {
            return;
        }
        //验证卡的可读性
        if(!rc522->PICC_ReadCardSerial()){
            return;
        }

        //正式读取到卡信息
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
        //rc522->PCD_Init();
        //如果检测到一次卡，那么就禁用读卡器
        startListening = false;
    }
};