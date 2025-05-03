#include "NvmeSmartInterpreter.h"
#include <algorithm>

auto NVMeBufferInterpreter::BufferToSMARTValueList(std::span<uint8_t> buffer) -> SMARTValueList
{
    SMARTValueList list;
    list.reserve(14);
    list.push_back(SeperateCriticalWarningFrom(buffer));
    list.push_back(SeperateTemperatureFrom(buffer));
    list.push_back(SeperateAvailableSpareFrom(buffer));
    list.push_back(SeperatePercentageUsedFrom(buffer));
    list.push_back(SeperateDataUnitsReadFrom(buffer));
    list.push_back(SeperateDataUnitsWrittenFrom(buffer));
    list.push_back(SeperateHostReadCommandsFrom(buffer));
    list.push_back(SeperateHostWriteCommandsFrom(buffer));
    list.push_back(SeperateControllerBusyTimeFrom(buffer));
    list.push_back(SeperatePowerCyclesFrom(buffer));
    list.push_back(SeperatePowerOnHoursFrom(buffer));
    list.push_back(SeperateUnsafeShutdownsFrom(buffer));
    list.push_back(SeperateMediaErrorsFrom(buffer));
    list.push_back(SeperateNumberOfErrorsFrom(buffer));
    return list;
}

auto NVMeBufferInterpreter::SeperateCriticalWarningFrom(std::span<uint8_t> buffer) -> SMARTValueEntry
{
    auto result = SMARTValueEntry{};
    result.id = static_cast<int>(SMARTValueID::CriticalWarning);
    result.raw = buffer[0];
    return result;
}

auto NVMeBufferInterpreter::SeperateTemperatureFrom(std::span<uint8_t> buffer) -> SMARTValueEntry
{
    auto result = SMARTValueEntry{};
    result.id = static_cast<int>(SMARTValueID::TemperatureInKelvin);
    result.raw = (static_cast<uint64_t>(buffer[2]) << 8) | buffer[1];
    return result;
}

auto NVMeBufferInterpreter::SeperateAvailableSpareFrom(std::span<uint8_t> buffer) -> SMARTValueEntry
{
    auto result = SMARTValueEntry{};
    result.id = static_cast<int>(SMARTValueID::AvailableSpare);
    result.raw = buffer[3];
    result.threshold = buffer[4];
    return result;
}

auto NVMeBufferInterpreter::SeperatePercentageUsedFrom(std::span<uint8_t> buffer) -> SMARTValueEntry
{
    auto result = SMARTValueEntry{};
    result.id = static_cast<int>(SMARTValueID::PercentageUsed);
    result.raw = buffer[5];
    return result;
}

template<typename T>
auto ReadValue(std::span<uint8_t> buffer, int start, int end) -> T
{
    T raw = 0;
    for (int i = end; i >= start; --i)
    {
        raw = (raw << 8) + buffer[i];
    }
    return raw;
}

auto Extract64bitValue(std::span<uint8_t> buffer, int start, int end, SMARTValueID id) -> SMARTValueEntry
{
    auto result = SMARTValueEntry{};
    result.id = static_cast<int>(id);
    result.raw = ReadValue<uint64_t>(buffer, start, end);
    return result;
}

auto NVMeBufferInterpreter::SeperateDataUnitsReadFrom(std::span<uint8_t> buffer) -> SMARTValueEntry
{
    return Extract64bitValue(buffer, 32, 47, SMARTValueID::DataUnitsReadInLBA);
}

auto NVMeBufferInterpreter::SeperateDataUnitsWrittenFrom(std::span<uint8_t> buffer) -> SMARTValueEntry
{
    return Extract64bitValue(buffer, 48, 63, SMARTValueID::DataUnitsWrittenInLBA);
}

auto NVMeBufferInterpreter::SeperateHostReadCommandsFrom(std::span<uint8_t> buffer) -> SMARTValueEntry
{
    return Extract64bitValue(buffer, 64, 79, SMARTValueID::HostReadCommands);
}

auto NVMeBufferInterpreter::SeperateHostWriteCommandsFrom(std::span<uint8_t> buffer) -> SMARTValueEntry
{
    return Extract64bitValue(buffer, 80, 95, SMARTValueID::HostWriteCommands);
}

auto NVMeBufferInterpreter::SeperateControllerBusyTimeFrom(std::span<uint8_t> buffer) -> SMARTValueEntry
{
    return Extract64bitValue(buffer, 96, 111, SMARTValueID::ControllerBusyTime);
}

auto NVMeBufferInterpreter::SeperatePowerCyclesFrom(std::span<uint8_t> buffer) -> SMARTValueEntry
{
    return Extract64bitValue(buffer, 112, 127, SMARTValueID::PowerCycles);
}

auto NVMeBufferInterpreter::SeperatePowerOnHoursFrom(std::span<uint8_t> buffer) -> SMARTValueEntry
{
    return Extract64bitValue(buffer, 128, 143, SMARTValueID::PowerOnHours);
}

auto NVMeBufferInterpreter::SeperateUnsafeShutdownsFrom(std::span<uint8_t> buffer) -> SMARTValueEntry
{
    return Extract64bitValue(buffer, 144, 159, SMARTValueID::UnsafeShutdowns);
}

auto NVMeBufferInterpreter::SeperateMediaErrorsFrom(std::span<uint8_t> buffer) -> SMARTValueEntry
{
    return Extract64bitValue(buffer, 160, 175, SMARTValueID::MediaErrors);
}

auto NVMeBufferInterpreter::SeperateNumberOfErrorsFrom(std::span<uint8_t> buffer) -> SMARTValueEntry
{
    return Extract64bitValue(buffer, 176, 191, SMARTValueID::NumberOfErrorInformationLogEntries);
}

auto NVMeBufferInterpreter::SeperateTemperatureSensorsFrom(std::span<uint8_t> buffer) -> std::vector<SMARTValueEntry>
{
    auto result = std::vector<SMARTValueEntry>{};
    //for ()
    return result;
}

auto NVMeBufferInterpreter::GetLBASize(std::span<uint8_t> buffer) -> int
{
    return ReadValue<int>(buffer, 8, 11);
}

auto NVMeBufferInterpreter::ReadLBASize() -> uint32_t
{
    return 512; // ATA_LBA_SIZE
}

auto ExtractStringFromBuffer(std::span<uint8_t> buffer, int start, int end) -> std::string
{
    const auto length = std::max(0, (end - start) + 1);
    const auto str = buffer.subspan(start, static_cast<size_t>(length));

    auto result = std::string{};
    result.reserve(length);
    for (const auto ch : str)
    {
        result += static_cast<char>(ch);
    }

    result.erase(std::remove_if(result.begin(), result.end(), ::isspace), result.end());
    return result;
}

auto NVMeBufferInterpreter::ReadModel(std::span<uint8_t> buffer) -> std::string
{
    return ExtractStringFromBuffer(buffer, 24, 63);
}

auto NVMeBufferInterpreter::ReadFirmware(std::span<uint8_t> buffer) -> std::string
{
    return ExtractStringFromBuffer(buffer, 64, 71);
}

auto NVMeBufferInterpreter::ReadSerial(std::span<uint8_t> buffer) -> std::string
{
    return ExtractStringFromBuffer(buffer, 4, 23);
}

auto NVMeBufferInterpreter::ReadDataSetManagementSupported(std::span<uint8_t> buffer) -> bool
{
    static constexpr auto dataSetManagementStart = 520;
    return (buffer[dataSetManagementStart] & 0x04) == 4;
}

auto NVMeBufferInterpreter::BufferToIdentifyDeviceResult(std::span<uint8_t> buffer) -> IdentifyDeviceResult
{
    auto result = IdentifyDeviceResult{};
    result.model = ReadModel(buffer);
    result.firmware = ReadFirmware(buffer);
    result.serial = ReadSerial(buffer);
    result.userSizeInKB = 0;
    result.SATASpeed = SataSpeed::NotSATA;
    result.lbaSize = ReadLBASize();
    result.IsDataSetManagementSupported = ReadDataSetManagementSupported(buffer);
    return result;
}

// TIdentifyDeviceResult TNVMeBufferInterpreter::BufferToCapacityAndLBA(std::span<uint8_t> buffer)
//{
//     uint64_t resultInByte = 0;
//     for (int i = 0; i <= 7; ++i)
//     {
//         resultInByte = (resultInByte << 8) + buffer[i];
//     }
//     int lbaSize = GetLBASize(buffer);
//     TIdentifyDeviceResult result;
//     result.LBASize = lbaSize;
//     result.UserSizeInKB = static_cast<uint64_t>((resultInByte * lbaSize) / 1000);
//     return result;
// }
