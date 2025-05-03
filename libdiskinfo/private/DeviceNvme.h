#pragma once

#include "DeviceHandle.h"

class DeviceNvme
{
public:
    DeviceNvme(DeviceHandle handle);

    void Identify();

private:
    DeviceHandle handle_;
};
