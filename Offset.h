#include <Windows.h>
#include <vector>

namespace Offsets {
    inline constexpr uintptr_t EntlistAdresse = 0x19AF3778CD0;
    inline constexpr uintptr_t ViewMatrixAdresse = 0x19AEC5D0F20;
	inline constexpr uintptr_t SSystemGlobalEnvironment = 0xD2F0;
	inline constexpr uintptr_t CEntitySystem = 0xA0;
}

namespace Signatures {
    inline const std::vector<int> ViewMatrixFunc = { 0xF3, 0x0F, 0x11, 0x96, 0xC0, 0x00, 0x00, 0x00, 0x0F };
}
