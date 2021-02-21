#pragma once

#include <HardwareSerial.h>
#include "IHandlable.hpp"
/**
 * @brief Jy61陀螺仪驱动代码
 * 
 */
class Jy61: public IHandlable{

public:
    explicit Jy61(HardwareSerial& serial);

    /*刷新三次陀螺仪数据，涵盖了加速度，角度，角速度*/
    void handle() override;

    float getAccX();
    float getAccY();
    float getAccZ();

    float getWX();
    float getWY();
    float getWZ();

    float getRoll();
    float getPitch();
    float getYaw();

    float getTemperature();
private:
    HardwareSerial *ser;

    uint8_t acc_buffer[6];
    uint8_t w_buffer[6];
    uint8_t angle_buffer[6];

    uint8_t temperature_buffer[2];
    /*获取一个数据包*/
    void getData();

};

Jy61::Jy61(HardwareSerial& serial){
    serial.setRxBufferSize(33);
    this->ser = &serial;
}

float Jy61::getAccX() {
    int16_t a = acc_buffer[1] << 8 | acc_buffer[0];
    return float(a) / 32768.0 * 16.0;
}

float Jy61::getAccY() {
    int16_t a = acc_buffer[3] << 8 | acc_buffer[2];
    return float(a) / 32768.0 * 16.0;
}

float Jy61::getAccZ() {
    int16_t a = acc_buffer[5] << 8 | acc_buffer[4];
    return float(a) / 32768.0 * 16.0;
}

float Jy61::getWX() {
    int16_t a = w_buffer[1] << 8 | w_buffer[0];
    return float(a)/ 32768.0 * 2000.0;
}

float Jy61::getWY() {
    int16_t a = w_buffer[3] << 8 | w_buffer[2];
    return float(a)/ 32768.0 * 2000.0;
}

float Jy61::getWZ() {
    int16_t a = w_buffer[5] << 8 | w_buffer[4];
    return float(a)/ 32768.0 * 2000.0;
}

float Jy61::getRoll() {
    int16_t a = angle_buffer[1] << 8 | w_buffer[0];
    return float(a) / 32768.0 * 180.0;
}

float Jy61::getPitch() {
    int16_t a = angle_buffer[3] << 8 | w_buffer[2];
    return float(a) / 32768.0 * 180.0;
}

float Jy61::getYaw() {
    int16_t a = angle_buffer[5] << 8 | w_buffer[4];
    return float(a) / 32768.0 * 180.0;
}

float Jy61::getTemperature() {
    int16_t a = temperature_buffer[1] << 8 | temperature_buffer[0];
    return float(a) / 340 +36.53;
}

void Jy61::getData() {
    if(!ser->available()){
        return;
    }
    uint8_t byte;
    while(byte != 0x55){
        ser->readBytes(&byte,1);
    }
    //此时byte的值为0x55

    //下一个应当为数据包类型标识字节

    ser->readBytes(&byte,1);

    switch (byte) {
        case 0x51:
            //加速度
            ser->readBytes(acc_buffer,6);
            ser->readBytes(temperature_buffer,2);
            break;
        case 0x52:
            //角速度
            ser->readBytes(w_buffer,6);
            ser->readBytes(temperature_buffer,2);
            break;
        case 0x53:
            //欧拉角
            ser->readBytes(angle_buffer,6);
            ser->readBytes(temperature_buffer,2);
            break;
    }
}

void Jy61::handle() {
    for(int i=0;i<3;i++){
        getData();
    }
}