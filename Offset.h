#include <Windows.h>
#include <vector>

namespace Offsets {
    inline constexpr DWORD PlayerHealth = 0x12345678;
    inline constexpr DWORD PlayerPosition = 0x87654321;
}

namespace Signatures {
    inline const std::vector<int> PlayerHealthSignature = { 0x8B, 0x45, -1, 0x8B, 0x4D, -1, 0x89, 0x45, -1 };
    inline const std::vector<int> PlayerPositionSignature = { 0xA1, -1, -1, -1, -1, 0x8B, 0x40, 0x04, 0x89, 0x45, -1 };
}
