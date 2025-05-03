#pragma once

#include <memory>

class DiskInfo
{
public:
    DiskInfo(int deviceIndex);
    ~DiskInfo();

    auto GetTemperature() -> int;

private:
    std::unique_ptr<class DiskInfoImpl> pImpl_;
};
