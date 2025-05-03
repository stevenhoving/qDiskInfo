#pragma once

#include "DeviceIdentity.h"
#include <span>
#include <cstdint>
#include <vector>

enum class SMARTValueID
{
    CriticalWarning,
    TemperatureInKelvin, // Composite Temperature
    AvailableSpare,
    PercentageUsed,
    DataUnitsReadInLBA,
    DataUnitsWrittenInLBA,
    HostReadCommands,
    HostWriteCommands,
    ControllerBusyTime,
    PowerCycles,
    PowerOnHours,
    UnsafeShutdowns,
    MediaErrors,
    NumberOfErrorInformationLogEntries,
    TemperatureSensor1 = 0xC8,
    TemperatureSensor2,
    TemperatureSensor3,
    TemperatureSensor4,
    TemperatureSensor5,
    TemperatureSensor6,
    TemperatureSensor7,
    TemperatureSensor8
};

struct SMARTValueEntry
{
    uint8_t id;
    uint8_t current;
    uint8_t wurst;
    uint8_t threshold;
    uint64_t raw;
};

using SMARTValueList = std::vector<SMARTValueEntry>;

class NVMeBufferInterpreter
{
public:
    auto BufferToIdentifyDeviceResult(std::span<uint8_t> buffer) -> IdentifyDeviceResult;
    auto BufferToSMARTValueList(std::span<uint8_t> buffer) -> SMARTValueList;

private:
    auto GetFirmwareFromBuffer(std::span<uint8_t> buffer) -> std::string;
    auto GetModelFromBuffer(std::span<uint8_t> buffer) -> std::string;
    auto GetSerialFromBuffer(std::span<uint8_t> buffer) -> std::string;
    auto GetDataSetManagementSupported(std::span<uint8_t> buffer) -> bool;
    auto GetLBASizeFromBuffer() -> uint32_t;
    auto GetLBASize(std::span<uint8_t> buffer) -> int;

    auto SeperateCriticalWarningFrom(std::span<uint8_t> buffer) -> SMARTValueEntry;
    auto SeperateTemperatureFrom(std::span<uint8_t> buffer) -> SMARTValueEntry;
    auto SeperateAvailableSpareFrom(std::span<uint8_t> buffer) -> SMARTValueEntry;
    auto SeperatePercentageUsedFrom(std::span<uint8_t> buffer) -> SMARTValueEntry;
    auto SeperateDataUnitsReadFrom(std::span<uint8_t> buffer) -> SMARTValueEntry;
    auto SeperateDataUnitsWrittenFrom(std::span<uint8_t> buffer) -> SMARTValueEntry;
    auto SeperateHostReadCommandsFrom(std::span<uint8_t> buffer) -> SMARTValueEntry;
    auto SeperateHostWriteCommandsFrom(std::span<uint8_t> buffer) -> SMARTValueEntry;
    auto SeperateControllerBusyTimeFrom(std::span<uint8_t> buffer) -> SMARTValueEntry;
    auto SeperatePowerCyclesFrom(std::span<uint8_t> buffer) -> SMARTValueEntry;
    auto SeperatePowerOnHoursFrom(std::span<uint8_t> buffer) -> SMARTValueEntry;
    auto SeperateUnsafeShutdownsFrom(std::span<uint8_t> buffer) -> SMARTValueEntry;
    auto SeperateMediaErrorsFrom(std::span<uint8_t> buffer) -> SMARTValueEntry;
    auto SeperateNumberOfErrorsFrom(std::span<uint8_t> buffer) -> SMARTValueEntry;
    auto SeperateTemperatureSensorsFrom(std::span<uint8_t> buffer) -> std::vector<SMARTValueEntry>;
};
