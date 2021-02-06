#pragma once

#include <OneButton.h>
#include "IHandlable.hpp"

/**
 * @brief 使用OneButton库来实现更加强大的按钮类
 * 
 */
class OneButtonHandle:public IHandlable,public OneButton{

public:
    explicit OneButtonHandle(int pin, boolean activeLow = true, bool pullupActive = true):OneButton(pin,activeLow,pullupActive){

    }

    void handle() override {
        tick();
    }
};