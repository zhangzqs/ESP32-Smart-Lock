#pragma once
#include <list>

class IHandlable{
// private:
//     static std::list<IHandlable*> handlableTasks;

// public:
//     static void loop(){
//         for(auto& handlableTask:handlableTasks){
//             handlableTask->handle();
//         }
//     }

public:
    explicit IHandlable(){
        //IHandlable::handlableTasks.push_back(this);
        //Serial.println("A IHandlable Created!!!");
    }

    virtual void handle() = 0;

};