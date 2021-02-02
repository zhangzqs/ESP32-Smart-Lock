#pragma once

#include "RC522_CardReader.hpp"

class RC522_CallbackCardReader:public RC522_CardReader{

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
    explicit RC522_CallbackCardReader(uint8_t sda,uint8_t rst):RC522_CardReader(sda,rst){

    }

};