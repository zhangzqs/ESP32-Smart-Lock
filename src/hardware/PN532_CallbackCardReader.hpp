#pragma once

#include "PN532_CardReader.hpp"

class PN532_CallbackCardReader:public PN532_CardReader{

public:
    using CardCallback = std::function<void(uint32_t)>;
private:
    CardCallback callback;
public:
    void setCardCallback(CardCallback callback){
        this->callback = std::move(callback);
    }

protected:
    void onCardDetected(uint32_t uid) override{
        callback(uid);
    }

public:
    explicit PN532_CallbackCardReader(uint8_t sda,uint8_t scl,uint8_t irq,uint8_t rst):PN532_CardReader(sda,scl,irq,rst){

    }

};