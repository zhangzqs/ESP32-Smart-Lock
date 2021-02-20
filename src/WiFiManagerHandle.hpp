#pragma once

#include "IHandlable.hpp"
#include <WiFiManager.h>

class WiFiManagerHandle : public IHandlable {
private:
    static WiFiManagerHandle* instance;
    WiFiManager* wm;
    explicit WiFiManagerHandle()
    {
        wm = new WiFiManager();
    }

public:
    static WiFiManagerHandle* getInstance()
    {
        if (instance == nullptr) {
            instance = new WiFiManagerHandle();
        }
        return instance;
    }

    WiFiManager* getWiFiManager()
    {
        return wm;
    }

private:
    bool activeHandle = false; //激活事件

private:
    std::function<void()> successCallback = nullptr;
    std::function<void()> failCallback = nullptr;

public:
    void attachSuccessCallback(std::function<void()> successCallback)
    {
        this->successCallback = std::move(successCallback);
    }

    void attachFailCallback(std::function<void()> failCallback)
    {
        this->failCallback = std::move(failCallback);
    }
    void begin()
    {
        xTaskCreatePinnedToCore([](void* pvParameters) {
            WiFi.mode(WIFI_STA);
            auto wmh = WiFiManagerHandle::getInstance();
            wmh->getWiFiManager()->autoConnect("SmartLock", "88888888");
            wmh->activeHandle = true;
            if (!WiFi.isConnected()) {
                Serial.println("Connect Failed");
            }
            vTaskDelete(NULL);
        },
            "wifiManagerTask", 8192, NULL, 1, NULL, CONFIG_ARDUINO_RUNNING_CORE);
    }

    void handle() override
    {
        if (activeHandle) {
            if (WiFi.isConnected()) {
                if (successCallback != nullptr) {
                    successCallback();
                }
            }
            activeHandle = false;
        }
    }
};

WiFiManagerHandle* WiFiManagerHandle::instance = nullptr;