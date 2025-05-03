#pragma once

#include <string>
#include <cstdint>

enum class StorageInterface
{
    Probing,
    ATA,
    SAT,
    SCSI,
    NVMe,
    UnknownInterface
};

enum class SataSpeed
{
    NotSATA,
    UnknownSATASpeed,
    SATA150,
    SATA300,
    SATA600
};

struct RotationRate
{
    bool Supported{false};
    int Value{0};
};

enum class PCIeSpecification
{
    PCIeUnknownSpec = 0,
    PCIe1d0 = 1,
    PCIe2d0 = 2,
    PCIe3d0 = 3
};

enum class PCIeDataWidth
{
    PCIeUnknownDataWidth = 0,
    PCIex1 = 1,
    PCIex2 = 2,
    PCIex4 = 4,
    PCIex8 = 8,
    PCIex12 = 12,
    PCIex16 = 16,
    PCIex32 = 32
};

struct SlotSpeed
{
    PCIeSpecification SpecVersion{PCIeSpecification::PCIeUnknownSpec};
    PCIeDataWidth LinkWidth{PCIeDataWidth::PCIeUnknownDataWidth};
};

struct SlotMaxCurrSpeed
{
    SlotSpeed maximum;
    SlotSpeed current;
};

struct IdentifyDeviceResult
{
    std::string model;
    std::string firmware;
    std::string serial;
    uint64_t userSizeInKB{0};
    SataSpeed SATASpeed;
    SlotSpeed slotspeed;
    uint32_t lbaSize{0};
    RotationRate rotationRate;
    StorageInterface storageInterface{StorageInterface::UnknownInterface};
    bool IsDataSetManagementSupported{false};
};
