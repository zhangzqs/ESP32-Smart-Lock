#pragma once
#include <Arduino.h>
#include <Servo.h>

class Lock
{
private:
    uint8_t pin;
    int startDegree; //起始角度
    int endDegree;   //终止角度
    Servo servo;

public:
    explicit Lock(uint8_t pin, int startDegree = 0, int endDegree = 20) //
        : pin(pin), startDegree(startDegree), endDegree(endDegree)      //初始化各个参数
    {
        servo.attach(pin); //分配舵机资源
    }

    virtual ~Lock()
    {
        servo.detach(); //释放舵机对象持有的资源
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

    //先开再关,即开门
    void unlock()
    {
        open();
        delay(1000);
        close();
    }
};
