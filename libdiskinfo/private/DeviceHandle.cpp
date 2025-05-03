#include "DeviceHandle.h"

DeviceHandle::DeviceHandle() = default;

DeviceHandle::DeviceHandle(const std::string& devicePath)
{
    // Ensure that we do not detect errors from other steps in our process
    SetLastError(ERROR_SUCCESS);

    handle_ = CreateFileA(devicePath.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

    if (handle_ == INVALID_HANDLE_VALUE)
        throw std::runtime_error("Failed to open device handle");

    if (const auto lastError = GetLastError(); lastError != ERROR_SUCCESS)
        throw std::runtime_error(std::format("Failed to open device handle - last error: {}", lastError));
}

DeviceHandle::DeviceHandle(DeviceHandle&& other) noexcept
    : handle_(other.handle_)
{
    other.handle_ = INVALID_HANDLE_VALUE;
}

auto DeviceHandle::operator=(DeviceHandle&& other) noexcept -> DeviceHandle&
{
    if (this != &other)
    {
        std::swap(handle_, other.handle_);
    }

    return *this;
}

DeviceHandle::~DeviceHandle()
{
    if (handle_ != INVALID_HANDLE_VALUE)
    {
        CloseHandle(handle_);
    }
}

HANDLE DeviceHandle::get() const noexcept
{
    return handle_;
}
