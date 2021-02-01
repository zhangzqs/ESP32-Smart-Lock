#pragma once
#include "AKey.hpp"

//支持回调的Key
class CallbackKey : public AKey
{

public:
    using Callback = std::function<void()>;

private:
    //当按钮按下时的回调函数
    Callback onKeyDownCallback;

    //当按钮松开时的回调函数
    Callback onKeyUpCallback;

protected:
    //当按钮按下时被调用
    void onKeyDown() override{
        onKeyDownCallback();
    }

    //当按钮松开时被调用
    void onKeyUp() override{
        onKeyUpCallback();
    };

public:
    explicit CallbackKey(uint8_t pin) : AKey(pin)
    {

    }

    inline void setOnKeyDownCallback(Callback callback){
        onKeyDownCallback = callback;
    }

    inline void setOnKeyUpCallback(Callback callback){
        onKeyUpCallback = callback;
    }
};