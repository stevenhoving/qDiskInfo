#include "WindowsDiskIoctl.h"

HandleWrapper CreateDiskHandle(const std::wstring& path)
{
    HANDLE handle = CreateFileW(path.c_str(), GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING,
                                FILE_ATTRIBUTE_NORMAL, nullptr);
    return HandleWrapper(handle);
}

bool DeviceIoControlWrapper(HANDLE device, DWORD ioctl, void* inbuf, DWORD inbufSize, void* outbuf, DWORD outbufSize,
                            DWORD* bytesReturned)
{
    return ::DeviceIoControl(device, ioctl, inbuf, inbufSize, outbuf, outbufSize, bytesReturned, nullptr);
}
