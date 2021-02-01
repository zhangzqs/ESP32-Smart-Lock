#pragma once

#include <Arduino.h>
#include "IHandlable.hpp"
class ICardReader:public IHandlable{

protected:
    virtual void onCardDetected(uint32_t uid){
        Serial.printf("检测到一张卡片！%d \n",uid);
    }

};