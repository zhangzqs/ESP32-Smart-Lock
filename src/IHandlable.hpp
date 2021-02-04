#pragma once

class IHandlable {
public:
    explicit IHandlable() = default;
    virtual ~IHandlable() = default;
    virtual void handle() = 0;
};