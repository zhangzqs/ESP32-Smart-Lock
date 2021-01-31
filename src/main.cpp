#include <Arduino.h>

#include "hardware/CallbackKey.hpp"
//#include "hardware/Handlable.hpp"
#include "hardware/Lock.hpp"
#include "hardware/CallbackCardReader.hpp"

CallbackKey *key;
Lock *lock;
CallbackCardReader *reader;

void Initialize(){
    key = new CallbackKey(15);
    lock = new Lock(4);
    reader = new CallbackCardReader(21,22,19,18);
}

void onKeyDown(){
    Serial.println("Key Down");
    lock->open();
}
void onKeyUp(){
    Serial.println("Key Up");
    lock->close();
}

void onCardSensed(uint32_t uid){
    Serial.printf("UID: %d \n",uid);
    lock->unlock();
}

void SetupAllCallback(){
    key->setOnKeyDownCallback(onKeyDown);
    key->setOnKeyUpCallback(onKeyUp);
    reader->setCardCallback(onCardSensed);
}
void setup() {
    Serial.begin(115200);
    Serial.println("Wire init");
    Wire.begin(21,22);
    Initialize();
    SetupAllCallback();
}

void loop() {
    //Handlable::loop();
    key->handle();
    reader->handle();
}