#pragma once

#include "IHandlable.hpp"
#include <list>
/**
 * @brief 任务组
 * 
 */
class HandlableTasks:public IHandlable{
private:
    std::list<IHandlable*> handlableTasks;

public:
    void addHandlable(IHandlable *handlableTask){
        handlableTasks.push_back(handlableTask);
    }

    void handle() override{
        for(auto &task:handlableTasks){
            task->handle();
        }
    }
};