#include "libdiskinfo/DiskInfo.h"

// I just hack these things together...
#include "WindowsDiskIoctl.h"
#include "DeviceIdentity.h"
#include "NVMeSMARTInterpreter.h"

//#include <winioctl.h>

inline bool GetNVMeIdentify(HANDLE device, BYTE nsid, std::span<BYTE> output)
{
    XX_STORAGE_PROTOCOL_SPECIFIC_DATA spsd{};

    spsd.PropertyId = STORAGE_ADAPTER_PROTOCOL_SPECIFIC_PROPERTY;
    spsd.QueryType = PROPERTY_STANDARD_QUERY;
    spsd.ProtocolType = PROTOCOL_TYPE_NVME;
    spsd.DataType = NVME_DATA_TYPE_IDENTIFY;
    spsd.ProtocolDataRequestValue = 1; // CNS value for Identify Controller or Namespace
    spsd.ProtocolDataRequestSubValue = nsid;

    // Must subtract 8 from the offset to account for putting PropertyID and QueryType into
    // Protocol Specific Data (they are actually part of Storage Property Query Structure)
    const auto protocolDataOffset = sizeof(XX_STORAGE_PROTOCOL_SPECIFIC_DATA) - 8 - NVME_MAX_LOG_SIZE;
    spsd.ProtocolDataOffset = protocolDataOffset;
    spsd.ProtocolDataLength = static_cast<ULONG>(output.size());

    DWORD bytesReturned = 0;
    if (!DeviceIoControlWrapper(device, IOCTL_STORAGE_QUERY_PROPERTY, &spsd, sizeof(spsd), &spsd, sizeof(spsd),
                                &bytesReturned))
    {
        std::cerr << "DeviceIoControl failed for NVMe Identify with error: " << GetLastError() << "\n";
        return false;
    }

    std::copy_n(spsd.logbuffer.begin(), output.size(), output.begin());
    //std::memcpy(output.data(), spsd.logbuffer.data(), output.size());
    return true;
}

inline bool GetNVMeLog(HANDLE device, BYTE logPageId, BYTE nsid, std::span<BYTE> output)
{
    STORAGE_PROTOCOL_SPECIFIC_QUERY_WITH_BUFFER spsd{};

    spsd.PropertyQuery.PropertyId = StorageAdapterProtocolSpecificProperty;     // STORAGE_ADAPTER_PROTOCOL_SPECIFIC_PROPERTY;
    spsd.PropertyQuery.QueryType = PropertyStandardQuery;       // PROPERTY_STANDARD_QUERY;
    spsd.ProtocolSpecific.ProtocolType = ProtocolTypeNvme;      // PROTOCOL_TYPE_NVME;
    spsd.ProtocolSpecific.DataType = NVMeDataTypeLogPage;       // NVME_DATA_TYPE_LOG_PAGE;
    spsd.ProtocolSpecific.ProtocolDataRequestValue = logPageId;
    spsd.ProtocolSpecific.ProtocolDataRequestSubValue = nsid;

    // \todo fix this ugly hack
    // Must subtract 8 from the offset to account for putting PropertyID and QueryType into
    // Protocol Specific Data(they are actually part of Storage Property Query Structure)
    //const auto protocolDataOffset = sizeof(XX_STORAGE_PROTOCOL_SPECIFIC_DATA) - 8 - NVME_MAX_LOG_SIZE;

    constexpr auto queryOffset = offsetof(STORAGE_PROTOCOL_SPECIFIC_QUERY_WITH_BUFFER, ProtocolSpecific);
    constexpr auto bufferOffset = offsetof(STORAGE_PROTOCOL_SPECIFIC_QUERY_WITH_BUFFER, logbuffer) - queryOffset;
    spsd.ProtocolSpecific.ProtocolDataOffset = bufferOffset;
    spsd.ProtocolSpecific.ProtocolDataLength = static_cast<ULONG>(output.size());

    DWORD bytesReturned = 0;
    if (!DeviceIoControlWrapper(device, IOCTL_STORAGE_QUERY_PROPERTY, &spsd, sizeof(spsd), &spsd, sizeof(spsd),
                                &bytesReturned))
    {
        std::cerr << "DeviceIoControl failed for NVMe log with error: " << GetLastError() << "\n";
        return false;
    }

    //std::copy_n(spsd.logbuffer.begin(), output.size(), output.begin());

    std::memcpy(output.data(), spsd.logbuffer.data(), output.size());
    return true;
}

class DiskInfoImpl
{
public:
    DiskInfoImpl(int deviceIndex)
        : handle_(CreateDiskHandle(L"\\\\.\\PhysicalDrive" + std::to_wstring(deviceIndex)))
    {
    }

    auto GetTemperature() -> int
    {
        LogBuffer nvmeLog{};
        if (GetNVMeLog(handle_.get(), 0x02, 1, nvmeLog))
        {
            auto smart = NVMeBufferInterpreter{}.BufferToSMARTValueList(nvmeLog);

            // the narrowing cast is acceptable
            // 273.15
            return static_cast<int>(static_cast<int64_t>(smart[1].raw) - 273);
        }
        throw std::runtime_error("Failed to get temperature");
    }

private:
    HandleWrapper handle_;
};

DiskInfo::DiskInfo(int deviceIndex)
    : pImpl_(std::make_unique<DiskInfoImpl>(deviceIndex))
{
}

DiskInfo::~DiskInfo() = default;

auto DiskInfo::GetTemperature() -> int
{
    return pImpl_->GetTemperature();
}
