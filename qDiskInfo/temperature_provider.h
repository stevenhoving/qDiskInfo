#pragma once

#include <memory>

class ITemperature {
public:
    virtual ~ITemperature() = default;
    virtual int GetTemperature() = 0;
};

// Dummy implementation for testing
class DummyTemperature : public ITemperature {
public:
    int GetTemperature() override;
};

class SmartTemperature : public ITemperature
{
public:
    SmartTemperature(const int deviceIndex);
    ~SmartTemperature() override;

    int GetTemperature() override;
private:
    std::unique_ptr<class SmartTemperatureImpl> pImpl_;

};
