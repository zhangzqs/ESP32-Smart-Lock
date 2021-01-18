#pragma once
#include <Arduino.h>
#include <FunctionalInterrupt.h>

/**
 * @brief 按键的抽象类
 * 封装了按键中断，派生类需继承该类并覆盖其中的
 * onKeyDown和onKeyUp纯虚函数来实现具体的按键触发事件
 * 注意：该按键抽象类中的按键事件函数由于是通过中断触发的
 * 所以其事件
 */
class AbstractKey {
private:
    uint8_t pin;

protected:
    /**
     * @brief 该函数为虚函数，当按键被按下时，该函数被调用
     * 派生类需覆盖该方法以实现自己的按键按下事件
     */
    virtual void onKeyDown() = 0;

    /**
     * @brief 该函数为虚函数，当按键被松开时，该函数被调用
     * 派生类需覆盖该方法以实现自己的按键松开事件
     */
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
    /**
     * @brief 构造一个按键的对象，并初始化pin引脚为输入引脚
     * 上拉它,并且注册一个下降沿中断
     * @param pin 按键所对应的引脚编号
     */
    explicit AbstractKey(uint8_t pin)
        : pin(pin)
    {
        pinMode(pin, INPUT | PULLUP); //初始化pin引脚为输入引脚并且上拉它等待其被下拉以触发下降沿中断
        attachInterrupt(pin, keyDown, FALLING); //按键被按下时，是下降沿被触发
    }
};