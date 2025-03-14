#include "system/dev/vfs.hpp"

#include "pros/rtos.hpp"

#include <array>
#include <mutex>
#include <optional>

constexpr static size_t MAX_FILE_DESCRIPTORS = 64;

static constinit pros::RecursiveMutex vfs_mut{};
static constinit std::array<std::optional<zest::fs::FileEntry>, MAX_FILE_DESCRIPTORS> fd_data{};

zest::fs::FileDescriptor zest::fs::FileDriver::add_vfs_entry(std::any data) {
    std::lock_guard lock{vfs_mut};
    for (auto it = fd_data.begin(); it != fd_data.end(); ++it) {
        if (!it->has_value()) {
            *it = {.driver = this->shared_from_this(), .data = data};
            return it - fd_data.begin();
        }
    }
    return -1;
}

int32_t zest::fs::FileDriver::update_vfs_entry(zest::fs::FileDescriptor fd, std::any data) {
    std::lock_guard lock{vfs_mut};
    if (fd < 0 || fd >= fd_data.size())
        return -1;
    if (fd_data[static_cast<int>(fd)].has_value())
        return -1;
    fd_data[static_cast<int>(fd)] = {.driver = this->shared_from_this(), .data = data};
    return 0;
}