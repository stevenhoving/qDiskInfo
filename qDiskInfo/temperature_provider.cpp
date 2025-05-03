#include "temperature_provider.h"
#include "libdiskinfo/DiskInfo.h"
#include <cstdlib>
#include <ctime>
#include <qdebug>

int DummyTemperature::GetTemperature()
{
    return 20 + (std::rand() % 10); // Simulates 20-29 degrees
}

class SmartTemperatureImpl
{
public:
    SmartTemperatureImpl(const int deviceIndex)
        : diskInfo_(deviceIndex)
    {
    }
    int GetTemperature()
    {
        return diskInfo_.GetTemperature();
    }

    auto GetIdentity() -> IdentifyDeviceResult
    {
        return diskInfo_.GetIdentity();
    }

private:
    DiskInfo diskInfo_;
};

SmartTemperature::SmartTemperature(const int deviceIndex)
    : pImpl_(std::make_unique<SmartTemperatureImpl>(deviceIndex))
{
    auto id = pImpl_->GetIdentity();
    qDebug() << id.model;
}

SmartTemperature::~SmartTemperature() = default;

int SmartTemperature::GetTemperature()
{
    return pImpl_->GetTemperature();
}
