#pragma once

#include "DeviceIdentity.h"
#include <memory>

class DiskInfo
{
public:
    DiskInfo(int deviceIndex);
    ~DiskInfo();

    auto GetIdentity() -> IdentifyDeviceResult;
    auto GetTemperature() -> int;

private:
    std::unique_ptr<class DiskInfoImpl> pImpl_;
};
