#pragma once

#include <ArduinoOTA.h>
#include <WiFi.h>
#include "IHandlable.hpp"

//空中升级
class OtaUploadHandle:public IHandlable{

public:
    explicit OtaUploadHandle(){

    }

    void handle() override{
        if(WiFi.isConnected()){
            ArduinoOTA.handle();
        }
    }    
};