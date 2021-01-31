#pragma once
#include <list>

class Handlable{
// private:
//     static std::list<Handlable*> handlableTasks;

// public:
//     static void loop(){
//         for(auto& handlableTask:handlableTasks){
//             handlableTask->handle();
//         }
//     }

public:
    explicit Handlable(){
        //Handlable::handlableTasks.push_back(this);
        //Serial.println("A Handlable Created!!!");
    }

    virtual void handle() = 0;

};