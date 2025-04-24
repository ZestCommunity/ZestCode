#include "pros/rtos.hpp"
#include "system/vfs/file_descriptor.hpp"
#include "system/vfs/file_driver.hpp"

#include <array>
#include <cerrno>
#include <concepts>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <string>
#include <sys/unistd.h>
#include <system_error>
#include <unordered_map>

namespace zest::fs {
namespace {
struct FileEntry {
    std::shared_ptr<FileDriver> driver;
    std::any data;
};
} // namespace
} // namespace zest::fs

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

static auto with_fd(int file, std::invocable<zest::fs::FileEntry> auto func) {
    using ReturnType = decltype(func(zest::fs::FileEntry{}))::value_type;
    struct _reent* r = _REENT;
    zest::fs::FileEntry file_entry;
    try {
        file_entry = fd_data.at(file).value();
    } catch (std::bad_optional_access&) {
        r->_errno = EBADF;
        return ReturnType{-1};
    } catch (std::out_of_range&) {
        r->_errno = EBADF;
        return ReturnType{-1};
    }
    auto result = func(file_entry);
    if (result) {
        return result.value();
    } else {
        r->_errno = result.error().value();
        return ReturnType{-1};
    }
}

// newlib fs stubs: these must be extern "C" to prevent name mangling,
// which would prevent newlib from linking to these functions properly

extern "C" {

int _open(const char* file, int flags, int mode) {
    std::lock_guard lock{vfs_mut};
    struct _reent* r = _REENT;

    std::string path = file;
    if (!path.starts_with("/"))
        path = working_directory + path;

    auto idx = path.find("/", 1);
    if (idx == std::string::npos) {
        idx = path.size() - 1;
    }

    std::string_view trimmed_path = path;
    trimmed_path.remove_prefix(idx + 1);
    std::string driver_name = path.substr(1, idx);

    auto driver = [&] {
        auto it = drivers.find(driver_name);
        if (it != drivers.end())
            return it->second;
        return std::shared_ptr<zest::fs::FileDriver>{nullptr};
    }();

    if (driver == nullptr) {
        r->_errno = ENOENT;
        return -1;
    }

    return static_cast<int>(driver->open(trimmed_path.cbegin(), flags, mode).value_or(-1));
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

ssize_t _write(int file, void* buf, size_t len) {
    return with_fd(file, [&](zest::fs::FileEntry file_entry) {
        return file_entry.driver->write(
            file_entry.data,
            std::span<std::byte>{static_cast<std::byte*>(buf), len}
        );
    });
}

ssize_t _read(int file, void* buf, size_t len) {
    return with_fd(file, [&](zest::fs::FileEntry file_entry) {
        return file_entry.driver->read(
            file_entry.data,
            std::span<std::byte>{static_cast<std::byte*>(buf), len}
        );
    });
}

int _close(int file) {
    // TODO: implement
    (void)file;
    return 0;
}

int _fstat(int file, struct stat* st) {
    return with_fd(file, [&](zest::fs::FileEntry file_entry) {
        return file_entry.driver->fstat(file_entry.data, st).transform([]() {
            return 0;
        });
    });
}

off_t _lseek(int file, off_t ptr, int dir) {
    return with_fd(file, [&](zest::fs::FileEntry file_entry) {
        return file_entry.driver->lseek(file_entry.data, ptr, dir);
    });
}

int _isatty(int file) {
    return with_fd(file, [&](zest::fs::FileEntry file_entry) {
        bool is_atty = file_entry.driver->isatty(file_entry.data);
        return std::expected<int, std::error_condition>{is_atty};
    });
}

int32_t fdctl(int file, uint32_t action, void* extra_arg) {
    return with_fd(file, [&](zest::fs::FileEntry file_entry) {
        int ret_val = file_entry.driver->ctl(file_entry.data, action, extra_arg);
        return std::expected<int, std::error_condition>{ret_val};
    });
}
}