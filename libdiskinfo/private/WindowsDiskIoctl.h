#pragma once

#include <Windows.h>
#include <ntddscsi.h>
#include <string>
#include <stdexcept>
#include <array>


constexpr size_t NVME_MAX_LOG_SIZE = 4096;
constexpr size_t ATA_MAX_LOG_SIZE = 512;
constexpr size_t MAX_LOG_SIZE = 4096;

// Scopes
constexpr int ADAPTER_SCOPE = 0;
constexpr int DEVICE_SCOPE = 1;

constexpr DWORD PROPERTY_STANDARD_QUERY = 0;
constexpr DWORD STORAGE_ADAPTER_PROTOCOL_SPECIFIC_PROPERTY = 49;
constexpr DWORD STORAGE_DEVICE_PROTOCOL_SPECIFIC_PROPERTY = 50;

constexpr DWORD PROTOCOL_TYPE_ATA = 0x02;
constexpr DWORD PROTOCOL_TYPE_NVME = 0x03;

constexpr DWORD NVME_DATA_TYPE_IDENTIFY = 1;
constexpr DWORD NVME_DATA_TYPE_LOG_PAGE = 2;

constexpr DWORD ATA_DATA_TYPE_IDENTIFY = 1;
constexpr DWORD ATA_DATA_TYPE_LOG_PAGE = 2;

constexpr BYTE ATA_READ_LOG_EXT = 0x2F;

constexpr ULONG ATA_CMD_TIMEOUT_SECONDS = 5;

using LogBuffer = std::array<BYTE, MAX_LOG_SIZE>;

#pragma pack(push, 4)

struct XX_ATA_PASS_THROUGH_DIRECT
{
    USHORT Length;
    USHORT AtaFlags;
    CHAR PathId;
    CHAR TargetId;
    CHAR Lun;
    CHAR ReservedAsUchar;
    ULONG DataTransferLength;
    ULONG TimeOutValue;
    ULONG ReservedAsUlong;
    PVOID DataBuffer;
    CHAR Features;
    CHAR LBACount;
    CHAR LBANumber;
    CHAR ICC;
    CHAR Auxiliary;
    CHAR Device;
    CHAR Command;
    CHAR Reserved;
    CHAR Features2;
    CHAR LBACount2;
    CHAR LBANumber2;
    CHAR ICC2;
    CHAR Auxiliary2;
    CHAR Device2;
    CHAR Command2;
    CHAR Reserved2;
    LogBuffer logbuffer;
};

/*
typedef struct _STORAGE_PROTOCOL_SPECIFIC_DATA {
    STORAGE_PROTOCOL_TYPE ProtocolType;
    DWORD   DataType;                     // The value will be protocol specific, as defined in STORAGE_PROTOCOL_NVME_DATA_TYPE or STORAGE_PROTOCOL_ATA_DATA_TYPE.

    DWORD   ProtocolDataRequestValue;
    DWORD   ProtocolDataRequestSubValue;  // Data sub request value

    DWORD   ProtocolDataOffset;           // The offset of data buffer is from beginning of this data structure.
    DWORD   ProtocolDataLength;

    DWORD   FixedProtocolReturnData;
    DWORD   ProtocolDataRequestSubValue2; // First additional data sub request value

    DWORD   ProtocolDataRequestSubValue3; // Second additional data sub request value
    DWORD   ProtocolDataRequestSubValue4; // Third additional data sub request value

} STORAGE_PROTOCOL_SPECIFIC_DATA, *PSTORAGE_PROTOCOL_SPECIFIC_DATA;
*/

struct XX_STORAGE_PROTOCOL_SPECIFIC_DATA
{
    DWORD PropertyId;
    DWORD QueryType;

    // STORAGE_PROTOCOL_SPECIFIC_DATA
    DWORD ProtocolType;
    ULONG DataType;
    ULONG ProtocolDataRequestValue;
    ULONG ProtocolDataRequestSubValue;
    ULONG ProtocolDataOffset;
    ULONG ProtocolDataLength;
    ULONG FixedProtocolReturnData;
    ULONG ProtocolDataRequestSubValue2;
    ULONG ProtocolDataRequestSubValue3;
    ULONG ProtocolDataRequestSubValue4;
    LogBuffer logbuffer;
};

struct STORAGE_PROPERTY_QUERY_EX
{
    STORAGE_PROPERTY_ID PropertyId;
    STORAGE_QUERY_TYPE QueryType;
};

/*!
 * How this works:
 * A IOCTL_STORAGE_QUERY_PROPERTY expects a STORAGE_PROPERTY_QUERY structure with or without additional data at the end.
 * In this particular case the data is a STORAGE_PROTOCOL_SPECIFIC_DATA with itself also can have additional data
 * 
 */
struct STORAGE_PROTOCOL_SPECIFIC_QUERY_WITH_BUFFER
{
    STORAGE_PROPERTY_QUERY_EX PropertyQuery;
    STORAGE_PROTOCOL_SPECIFIC_DATA ProtocolSpecific;
    LogBuffer logbuffer;
};

#pragma pack(pop)

class HandleWrapper
{
public:
    explicit HandleWrapper(HANDLE handle)
        : handle_(handle)
    {
        if (handle_ == INVALID_HANDLE_VALUE)
        {
            throw std::runtime_error("Invalid handle");
        }
    }

    ~HandleWrapper()
    {
        if (handle_ && handle_ != INVALID_HANDLE_VALUE)
        {
            CloseHandle(handle_);
        }
    }

    HANDLE get() const
    {
        return handle_;
    }

private:
    HANDLE handle_;
};

HandleWrapper CreateDiskHandle(const std::wstring& path);

bool DeviceIoControlWrapper(HANDLE device, DWORD ioctl, void* inbuf, DWORD inbufSize, void* outbuf, DWORD outbufSize,
                            DWORD* bytesReturned);
