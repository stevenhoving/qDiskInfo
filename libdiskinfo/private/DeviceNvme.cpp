#include "DeviceNvme.h"
#include <winioctl.h>

// IOCTL Defines
// http://www.ioctls.net/
static constexpr auto IOCTL_ATA_PASS_THROUGH_DIRECT = 0x4d030;
// static constexpr auto IOCTL_STORAGE_QUERY_PROPERTY = 0x002d1400;

// https://docs.microsoft.com/en-us/windows/win32/api/winioctl/ne-winioctl-storage_query_type
static constexpr auto PROPERTY_STANDARD_QUERY = 0;

// https://docs.microsoft.com/en-us/windows/win32/api/winioctl/ne-winioctl-storage_property_id
static constexpr auto STORAGE_ADAPTER_PROTOCOL_SPECIFIC_PROPERTY = 49;
static constexpr auto STORAGE_DEVICE_PROTOCOL_SPECIFIC_PROPERTY = 50;

// https://docs.microsoft.com/en-us/windows/win32/api/winioctl/ne-winioctl-storage_protocol_type
static constexpr auto PROTOCOL_TYPE_ATA = 0x02;
// static constexpr auto PROTOCOL_TYPE_NVME = 0x03;

// https://docs.microsoft.com/en-us/windows/win32/api/winioctl/ne-winioctl-storage_protocol_nvme_data_type
static constexpr auto NVME_DATA_TYPE_IDENTIFY = 1;
static constexpr auto NVME_DATA_TYPE_LOG_PAGE = 2;

// https://docs.microsoft.com/en-us/windows/win32/api/winioctl/ne-winioctl-storage_protocol_ata_data_type
static constexpr auto ATA_DATA_TYPE_IDENTIFY = 1;
static constexpr auto ATA_DATA_TYPE_LOG_PAGE = 2;

static constexpr auto NVME_MAX_LOG_SIZE = 4096;
static constexpr auto ATA_MAX_LOG_SIZE = 512;
static constexpr auto MAX_LOG_SIZE = 4096;
static constexpr auto LOG_BUF = MAX_LOG_SIZE;


DeviceNvme::DeviceNvme(DeviceHandle handle)
    : handle_(std::move(handle))
{
}

enum class Scope
{
    ADAPTER_SCOPE,
    DEVICE_SCOPE
};

struct STORAGE_PROTOCOL_SPECIFIC_QUERY_WITH_BUFFER
{
    struct
    { // STORAGE_PROPERTY_QUERY without AdditionalsParameters[1]
        STORAGE_PROPERTY_ID PropertyId;
        STORAGE_QUERY_TYPE QueryType;
    } propertyQuery;
    STORAGE_PROTOCOL_SPECIFIC_DATA protocolSpecific;
    BYTE DataBuffer[NVME_MAX_LOG_SIZE];
};

void DeviceNvme::Identify()
{
    //const auto scope = Scope::ADAPTER_SCOPE;
    const auto scope = Scope::DEVICE_SCOPE;
    auto specificdata = STORAGE_PROTOCOL_SPECIFIC_QUERY_WITH_BUFFER();

    if (scope == Scope::ADAPTER_SCOPE)
        specificdata.propertyQuery.PropertyId = StorageDeviceProtocolSpecificProperty;
    else if (scope == Scope::DEVICE_SCOPE)
        specificdata.propertyQuery.PropertyId = StorageAdapterProtocolSpecificProperty;

    specificdata.propertyQuery.QueryType = PropertyStandardQuery;
    specificdata.protocolSpecific.ProtocolType = ProtocolTypeNvme;
    specificdata.protocolSpecific.DataType = NVME_DATA_TYPE_IDENTIFY;
    specificdata.protocolSpecific.ProtocolDataRequestValue = 1;
    specificdata.protocolSpecific.ProtocolDataRequestSubValue = 0;

    // Must subtract 8 from the offset to account for putting PropertyID and QueryType into
    // Protocol Specific Data(they are actually part of Storage Property Query Structure)
    const auto protocolDataOffset =
        sizeof(STORAGE_PROTOCOL_SPECIFIC_DATA) - 8 - NVME_MAX_LOG_SIZE;

    specificdata.protocolSpecific.ProtocolDataOffset = protocolDataOffset;
    specificdata.protocolSpecific.ProtocolDataLength = NVME_MAX_LOG_SIZE;

    DWORD bytesReturned = 0;
    const auto ret = DeviceIoControl(handle_.get(), IOCTL_STORAGE_QUERY_PROPERTY,
        &specificdata, sizeof(specificdata), &specificdata,
                    sizeof(specificdata), &bytesReturned, nullptr);

    if (!ret)
    {
        const auto error = GetLastError();
        //return specificdata.DataBuffer;
    }

        // DeviceIoControl(filehandle) as dctl : status,
    //        junk = dctl.ioctl(IOCTL_STORAGE_QUERY_PROPERTY, ctypes.byref(specificdata),
    //                          ctypes.sizeof(STORAGE_PROTOCOL_SPECIFIC_DATA), ctypes.byref(specificdata),
    //                          ctypes.sizeof(STORAGE_PROTOCOL_SPECIFIC_DATA)) if status
    //: return specificdata.logbuffer else : error = windll.kernel32.GetLastError() logging.error(
    //           'GetNVMeIdentify({0}, {1}) Failed to IOCTL_STORAGE_QUERY_PROPERTY {2}. GetLastError(): {3} - {4}'
    //               .format(disk_number, scope, dctl, error, FormatError(error))) return 0

    // IoControl(handle_, IOCTL_STORAGE_QUERY_PROPERTY, )

    // StorageQuery::TStorageQueryWithBuffer nptwb;
    // bool bRet = 0;
    // ZeroMemory(&nptwb, sizeof(nptwb));
    //
    // nptwb.ProtocolSpecific.ProtocolType = StorageQuery::ProtocolTypeNvme;
    // nptwb.ProtocolSpecific.DataType = StorageQuery::NVMeDataTypeIdentify;
    // nptwb.ProtocolSpecific.ProtocolDataOffset = sizeof(StorageQuery::TStorageProtocolSpecificData);
    // nptwb.ProtocolSpecific.ProtocolDataLength = 4096;
    // nptwb.ProtocolSpecific.ProtocolDataRequestValue = 0;
    // nptwb.ProtocolSpecific.ProtocolDataRequestSubValue = 1;
    // nptwb.Query.PropertyId = StorageQuery::StorageAdapterProtocolSpecificProperty;
    // nptwb.Query.QueryType = StorageQuery::PropertyStandardQuery;
    // DWORD dwReturned = 0;
    //
    // bRet = DeviceIoControl(hIoCtrl, IOCTL_STORAGE_QUERY_PROPERTY, &nptwb, sizeof(nptwb), &nptwb, sizeof(nptwb),
    //                        &dwReturned, NULL);
    //
    // if (bRet)
    //{
    //     ULONG64 totalLBA = *(ULONG64*)&nptwb.Buffer[0];
    //     int sectorSize = 1 << nptwb.Buffer[130];
    //     *diskSize = (DWORD)(totalLBA * sectorSize / 1000 / 1000);
    // }
    //
    // ZeroMemory(&nptwb, sizeof(nptwb));
    // nptwb.ProtocolSpecific.ProtocolType = StorageQuery::ProtocolTypeNvme;
    // nptwb.ProtocolSpecific.DataType = StorageQuery::NVMeDataTypeIdentify;
    // nptwb.ProtocolSpecific.ProtocolDataOffset = sizeof(StorageQuery::TStorageProtocolSpecificData);
    // nptwb.ProtocolSpecific.ProtocolDataLength = 4096;
    // nptwb.Query.PropertyId = StorageQuery::StorageAdapterProtocolSpecificProperty;
    // nptwb.Query.QueryType = StorageQuery::PropertyStandardQuery;
    // nptwb.ProtocolSpecific.ProtocolDataRequestValue = 1; /*NVME_IDENTIFY_CNS_CONTROLLER*/
    // nptwb.ProtocolSpecific.ProtocolDataRequestSubValue = 0;
    // dwReturned = 0;
    //
    // bRet = DeviceIoControl(hIoCtrl, IOCTL_STORAGE_QUERY_PROPERTY, &nptwb, sizeof(nptwb), &nptwb, sizeof(nptwb),
    //                        &dwReturned, NULL);
    //::CloseHandle(hIoCtrl);
    //
    // memcpy_s(data, sizeof(NVME_IDENTIFY_DEVICE), nptwb.Buffer, sizeof(NVME_IDENTIFY_DEVICE));
    //
    // return bRet;
}

#if 0
int main()
{
    //const auto path = std::format(L"\\\\.\\PhysicalDrive{}", physicalDriveId);
    DeviceHandle handle("\\\\.\\PhysicalDrive0");
    DeviceNvme deviceNvme(std::move(handle));
    try
    {
        deviceNvme.Identify();
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}

#endif