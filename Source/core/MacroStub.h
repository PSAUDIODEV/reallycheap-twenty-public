#pragma once

namespace ReallyCheap
{

// Temporary stub for MacroController during testing
class MacroStub
{
public:
    float wobbleDepthGain() const noexcept { return 1.0f; }
    float wobbleFlutterGain() const noexcept { return 1.0f; }
    float magneticCompGain() const noexcept { return 1.0f; }
    float magneticSatGain() const noexcept { return 1.0f; }
    float distortDriveAddDb() const noexcept { return 0.0f; }
    float digitalBitsFloor() const noexcept { return 0.0f; }
    float digitalSRFloorHz() const noexcept { return 48000.0f; }
    float spaceMixCap() const noexcept { return 1.0f; }
    float noiseLevelAddDb() const noexcept { return 0.0f; }
    float noiseAgeGain() const noexcept { return 1.0f; }
};

}