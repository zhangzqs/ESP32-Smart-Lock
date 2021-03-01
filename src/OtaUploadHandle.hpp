#pragma once

#include <ArduinoOTA.h>
#include <WiFi.h>
#include "IHandlable.hpp"

//空中升级
class OtaUploadHandle:public IHandlable{
private:
    bool first = true;
public:

    void handle() override{
        if(WiFi.isConnected()){
            if(first){
                ArduinoOTA.begin();
                ArduinoOTA.setPassword("123456");
                first = false;
            }else{
                ArduinoOTA.handle();
            }
        }
    }    
};