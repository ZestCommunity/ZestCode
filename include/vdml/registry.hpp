#ifndef VDML_REGISTRY_HPP
#define VDML_REGISTRY_HPP

#include <cstdint>
#include <optional>

namespace zest {
namespace vdml {
enum class DeviceType {
	NONE = 0,
	MOTOR = 2,
	ROTATION = 4,
	IMU = 6,
	DISTANCE = 7,
	RADIO = 8,
	VISION = 11,
	ADI = 12,
	OPTICAL = 16,
	GPS = 20,
	AIVISION = 29,
	SERIAL = 129,
	UNDEFINED = 255
};

struct DeviceInfo {
	DeviceType type;
	uint32_t data;
};

extern std::array<std::optional<DeviceInfo>, 32> device_registry;

void update_registry();
std::optional<DeviceInfo> get_device_info(int port);
bool validate_device(int port, DeviceType expected);
}  // namespace vdml
}  // namespace zest

#endif