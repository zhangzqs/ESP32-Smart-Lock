#include <Arduino.h>

#include "hardware/CallbackKey.hpp"
#include "hardware/Lock.hpp"
#include "hardware/RC522_CallbackCardReader.hpp"
#include <SPI.h>
CallbackKey *key;
Lock *lock;
RC522_CallbackCardReader *reader;

void Initialize(){
    key = new CallbackKey(15);
    lock = new Lock(4);
    reader = new RC522_CallbackCardReader(21,22);
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
    Initialize();
    SetupAllCallback();
}

void loop() {
    key->handle();
    reader->handle();
    lock->handle();
}