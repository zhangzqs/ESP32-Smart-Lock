#pragma once
#include <Arduino.h>
#include <FunctionalInterrupt.h>
/*
 * 按键的抽象类
 */
class AbstractKey
{
private:
    uint8_t pin;

protected:
    //纯虚函数,该事件交给其子类实现
    virtual void onKeyDown() = 0;
    virtual void onKeyUp() = 0;

private:
    std::function<void()> keyUp = [&]() {
        onKeyUp();
        attachInterrupt(pin, keyDown, FALLING); //当被松手时，立马注册当按键按下时的下降沿触发
    };

    std::function<void()> keyDown = [&]() {
        onKeyDown();
        attachInterrupt(pin, keyUp, RISING); //当被按下时，立马注册当按键松手时的上升沿触发
    };

public:
    explicit AbstractKey(uint8_t pin) : pin(pin)
    {
        pinMode(pin, INPUT | PULLUP);           //初始化pin引脚为输入引脚并且上拉它等待其被下拉以触发下降沿中断
        attachInterrupt(pin, keyDown, FALLING); //按键被按下时，是下降沿被触发
    }
};