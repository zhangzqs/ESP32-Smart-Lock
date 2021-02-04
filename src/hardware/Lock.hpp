#pragma once
#include <Arduino.h>
#include <Servo.h>
#include <Ticker.h>
#include <IHandlable.hpp>
class Lock:public IHandlable
{
private:
    uint8_t pin;
    int startDegree; //起始角度
    int endDegree;   //终止角度
    Servo servo;
    Ticker *ticker; //控制舵机开门间隔时间的定时器
    int intervalMs = 1000; //unlock函数中舵机的开门到关门的间隔时间

public:
    explicit Lock(uint8_t pin, int startDegree = 0, int endDegree = 200) //
        : pin(pin), startDegree(startDegree), endDegree(endDegree)      //初始化各个参数
    {
        servo.attach(pin); //分配舵机资源
        ticker = new Ticker([&](){
            this->close();
        },intervalMs,1,MILLIS);
    }

    virtual ~Lock()
    {
        servo.detach(); //释放舵机对象持有的资源
        ticker->stop(); //停止定时器任务
        delete ticker; 
    }

    //开锁
    void open()
    {
        servo.write(endDegree);
    }

    //关锁
    void close()
    {
        servo.write(startDegree);
    }

    void write(int d){
        servo.write(d);
    }

    /**
     * @brief unlock函数中舵机的开门到关门的间隔时间
     * @param ms 间隔时间
     */
    void setIntervalTime(int ms){
        intervalMs = ms;
        ticker->interval(ms);
    }

    //先开再关,即开门
    void unlock()
    {
        open();
        ticker->start();
    }

public:
    void handle() override{
        ticker->update();
    }
};
