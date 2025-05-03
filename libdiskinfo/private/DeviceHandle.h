#pragma once

#include <string>
#include <windows.h>

class DeviceHandle
{
public:
    DeviceHandle();

    DeviceHandle(const std::string& devicePath);

    DeviceHandle(DeviceHandle&& other) noexcept;
    auto operator = (DeviceHandle&& other) noexcept -> DeviceHandle&;

    ~DeviceHandle();

    HANDLE get() const noexcept;

private:
    HANDLE handle_{ INVALID_HANDLE_VALUE };
};
