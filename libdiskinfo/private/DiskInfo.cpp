#include "libdiskinfo/DiskInfo.h"

// I just hack these things together...
#include "WindowsDiskIoctl.h"
#include "BufferInterpreter/NVMeSMARTInterpreter.h"

#include <winioctl.h>
#include <expected>
#include <format>

inline bool GetNVMeIdentify(HANDLE device, BYTE nsid, std::span<BYTE> output)
{
}

inline bool GetNVMeLog(HANDLE device, BYTE logPageId, BYTE nsid, std::span<BYTE> output)
{
    STORAGE_PROTOCOL_SPECIFIC_QUERY_WITH_BUFFER spsd{};

    spsd.propertyQuery.PropertyId =
        StorageAdapterProtocolSpecificProperty;            // STORAGE_ADAPTER_PROTOCOL_SPECIFIC_PROPERTY;
    spsd.propertyQuery.QueryType = PropertyStandardQuery;  // PROPERTY_STANDARD_QUERY;
    spsd.protocolSpecific.ProtocolType = ProtocolTypeNvme; // PROTOCOL_TYPE_NVME;
    spsd.protocolSpecific.DataType = NVMeDataTypeLogPage;  // NVME_DATA_TYPE_LOG_PAGE;
    spsd.protocolSpecific.ProtocolDataRequestValue = logPageId;
    spsd.protocolSpecific.ProtocolDataRequestSubValue = nsid;

    // \todo fix this ugly hack
    // Must subtract 8 from the offset to account for putting PropertyID and QueryType into
    // Protocol Specific Data(they are actually part of Storage Property Query Structure)
    // const auto protocolDataOffset = sizeof(XX_STORAGE_PROTOCOL_SPECIFIC_DATA) - 8 - NVME_MAX_LOG_SIZE;

    constexpr auto queryOffset = offsetof(STORAGE_PROTOCOL_SPECIFIC_QUERY_WITH_BUFFER, protocolSpecific);
    constexpr auto bufferOffset = offsetof(STORAGE_PROTOCOL_SPECIFIC_QUERY_WITH_BUFFER, logbuffer) - queryOffset;
    spsd.protocolSpecific.ProtocolDataOffset = bufferOffset;
    spsd.protocolSpecific.ProtocolDataLength = static_cast<ULONG>(output.size());

    // just ensure we don't get a error that is not the result of something else.
    SetLastError(0);
    DWORD bytesReturned = 0;
    if (!DeviceIoControlWrapper(device, IOCTL_STORAGE_QUERY_PROPERTY, &spsd, sizeof(spsd), &spsd, sizeof(spsd),
                                &bytesReturned))
    {
        auto err = GetLastError();
        std::cerr << "DeviceIoControl failed for NVMe log with error: " << err << "\n";
        return false;
    }

    // std::copy_n(spsd.logbuffer.begin(), output.size(), output.begin());

    std::memcpy(output.data(), spsd.logbuffer.data(), output.size());
    return true;
}

enum class DeviceErr
{
    success,
    failure
};

// interface for abstracting away the different kind of implementations needed to retrieve the smart info from various
// device types
class IDevice
{
public:
    virtual auto GetIdentity() -> std::expected<IdentifyDeviceResult, DeviceErr> = 0;
    virtual auto GetTemperature() -> std::expected<int, DeviceErr> = 0;
};

class IoControlDevice
{
public:
    IoControlDevice(int deviceIndex)
        : device_(CreateDiskHandle(L"\\\\.\\PhysicalDrive" + std::to_wstring(deviceIndex)))
    {
    }

    struct IoControlBufferPair
    {
        template <typename TIn, typename TOut>
        IoControlBufferPair(TIn& in, TOut& out)
            : inputPtr(&in)
            , inputSize(sizeof(TIn))
            , outputPtr(&out)
            , outputSize(sizeof(TOut))
        {
        }

        void* inputPtr;
        DWORD inputSize;
        void* outputPtr;
        DWORD outputSize;
    };

    auto IoControl(uint32_t controlCode, IoControlBufferPair ioBuffer) -> uint32_t
    {
        // blegh,.. just make sure that we don't check old error codes...
        SetLastError(ERROR_SUCCESS);

        auto bytesReturned = uint32_t{0};
        auto ret = ::DeviceIoControl(device_.get(), (DWORD)controlCode, ioBuffer.inputPtr, ioBuffer.inputSize,
                                     ioBuffer.outputPtr, ioBuffer.inputSize, reinterpret_cast<DWORD*>(&bytesReturned),
                                     nullptr);
        if (!ret)
        {
            // todo refactor to use std::expected
            throw std::runtime_error(std::format("Device IoControl failed with error code: {}", GetLastError()));
        }

        return bytesReturned;
    }

private:
    HandleWrapper device_;
};

// \todo now write the samsung specific version that uses SCSI passthrough but secretly needs to be parsed as if it is normal OS style nvme get log buffer
 
// device that has the default windows os stuff implemented in there driver.
// \todo try to test this one
class DeviceWithOsDriver : public IDevice
{
public:
    DeviceWithOsDriver(int deviceIndex)
        : device_(deviceIndex)
    {
    }

    auto GetIdentity() -> std::expected<IdentifyDeviceResult, DeviceErr> override
    {
        auto buffer = GetCommonBuffer();
        buffer.protocolSpecific.DataType = NVMeDataTypeIdentify;

        // \note it seems that we don't have to specify any protocol data request stuff...
        // buffer.protocolSpecific.ProtocolDataRequestValue = 1;
        // buffer.protocolSpecific.ProtocolDataRequestSubValue = 1;
        auto size = device_.IoControl(IOCTL_STORAGE_QUERY_PROPERTY, {buffer, buffer});

        return NVMeBufferInterpreter{}.BufferToIdentifyDeviceResult({buffer.logbuffer.data(), size});
    }

    // nvme log page identifiers
    enum class LogPageId : uint32_t
    {
        ErrorInfo = 0x1,
        SmartInfo = 0x2,
    };

    // nvme spec 2.1 recommends using 0xFFFFFFFF for compatibility with v1.4 spec
    static constexpr auto GlobalLogPage = 0xFFFFFFFF;

    /*!
     * Get a list of all the smart values
     *
     * \note this function can be generalized to retrieve any kind of NVME data type log page
     */
    auto GetSmartList() -> std::expected<SMARTValueList, DeviceErr>
    {
        auto buffer = GetCommonBuffer();
        buffer.protocolSpecific.DataType = NVMeDataTypeLogPage;
        buffer.protocolSpecific.ProtocolDataRequestValue = std::to_underlying(LogPageId::SmartInfo);
        buffer.protocolSpecific.ProtocolDataRequestSubValue = GlobalLogPage;

        auto size = device_.IoControl(IOCTL_STORAGE_QUERY_PROPERTY, {buffer, buffer});
        return NVMeBufferInterpreter{}.BufferToSMARTValueList({buffer.logbuffer.data(), size});
    }

    auto GetTemperature() -> std::expected<int, DeviceErr> override
    {
        auto smart = GetSmartList();
        if (!smart)
            return std::unexpected(DeviceErr::failure);

        return static_cast<int>(static_cast<int64_t>(smart->at(1).raw) - 273);
    }

private:
    // this functions creates a default STORAGE_PROTOCOL_SPECIFIC_QUERY_WITH_BUFFER instance
    auto GetCommonBuffer() -> STORAGE_PROTOCOL_SPECIFIC_QUERY_WITH_BUFFER
    {
        auto buffer = STORAGE_PROTOCOL_SPECIFIC_QUERY_WITH_BUFFER{};

        buffer.propertyQuery.PropertyId = StorageAdapterProtocolSpecificProperty;
        buffer.propertyQuery.QueryType = PropertyStandardQuery;
        buffer.protocolSpecific.ProtocolType = ProtocolTypeNvme;
        buffer.protocolSpecific.DataType = NVMeDataTypeUnknown;
        buffer.protocolSpecific.ProtocolDataRequestValue = 0;
        buffer.protocolSpecific.ProtocolDataRequestSubValue = 0;

        static constexpr auto queryOffset = offsetof(STORAGE_PROTOCOL_SPECIFIC_QUERY_WITH_BUFFER, protocolSpecific);
        static constexpr auto bufferOffset =
            offsetof(STORAGE_PROTOCOL_SPECIFIC_QUERY_WITH_BUFFER, logbuffer) - queryOffset;
        buffer.protocolSpecific.ProtocolDataOffset = bufferOffset;
        buffer.protocolSpecific.ProtocolDataLength = static_cast<ULONG>(buffer.logbuffer.size());

        return buffer;
    }

    IoControlDevice device_;
};

class DiskInfoImpl
{
public:
    DiskInfoImpl(int deviceIndex)
        : device_(deviceIndex)
    {
    }

    auto GetIdentity() -> IdentifyDeviceResult
    {
        auto id = device_.GetIdentity();
        if (!id)
            throw std::runtime_error("Failed to get device identity");

        return *id;
    }

    auto GetTemperature() -> int
    {
        auto temp = device_.GetTemperature();
        if (!temp)
            throw std::runtime_error("Failed to get temperature");
        return *temp;
    }

private:
    DeviceWithOsDriver device_;
};

DiskInfo::DiskInfo(int deviceIndex)
    : pImpl_(std::make_unique<DiskInfoImpl>(deviceIndex))
{
}

DiskInfo::~DiskInfo() = default;

auto DiskInfo::GetIdentity() -> IdentifyDeviceResult
{
    return pImpl_->GetIdentity();
}

auto DiskInfo::GetTemperature() -> int
{
    return pImpl_->GetTemperature();
}
