#include "system/dev/vfs.hpp"

#include "pros/rtos.hpp"

#include <array>
#include <cerrno>
#include <mutex>
#include <optional>
#include <string>
#include <unordered_map>

constexpr static size_t MAX_FILE_DESCRIPTORS = 64;

static constinit pros::RecursiveMutex vfs_mut{};
static constinit std::array<std::optional<zest::fs::FileEntry>, MAX_FILE_DESCRIPTORS> fd_data{};
static constinit std::string working_directory = "/usd";
// FIXME: fix static initialization order fiasco
static std::unordered_map<std::string, std::shared_ptr<zest::fs::FileDriver>> drivers{};

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

// newlib fs stubs: these must be extern "C" to prevent name mangling,
// which would prevent newlib from linking to these functions properly

extern "C" {

int _open(const char* file, int flags, int mode) {
    std::lock_guard lock{vfs_mut};
    struct _reent* r = _REENT;

    std::string path = file;
    if (!path.starts_with("/")) path = working_directory + path;

    auto idx = path.find("/", 1);
    if (idx == std::string::npos) {
        idx = path.size() - 1;
    }

    std::string_view trimmed_path = path;
    trimmed_path.remove_prefix(idx + 1);
    std::string driver_name = path.substr(1, idx);

    auto driver = [&] {
        auto it = drivers.find(driver_name);
        if (it != drivers.end()) return it->second;
        return std::shared_ptr<zest::fs::FileDriver>{nullptr};
    } ();

    if (driver == nullptr) {
        r->_errno = ENOENT;
        return -1;
    }

    return static_cast<int>(driver->open(trimmed_path.cbegin(), flags, mode));
}

int chdir(const char* path) {
    std::lock_guard lock{vfs_mut};
    std::string_view new_path = path;
    if (!new_path.ends_with('/')) {
        // TODO: should we append a slash?
        errno = ENOTDIR;
        return -1;
    }
    if (new_path.starts_with('/')) {
        working_directory = new_path;
    } else {
        working_directory += new_path;
    }
    return 0;
}

char* getcwd(char* buf, size_t size) {
    std::lock_guard lock{vfs_mut};
    if (size == 0) {
        // TODO: implement glibc extension?
        errno = EINVAL;
        return nullptr;
    }
    if (working_directory.size() + 1 > size) {
        errno = ERANGE;
        return nullptr;
    }
    std::copy(working_directory.cbegin(), working_directory.cend(), buf);
    return buf;
}
}