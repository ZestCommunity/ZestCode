#include "system/vfs/usd_driver.hpp"

#include "system/vfs/file_flags.hpp"
#include "v5_api_patched.h"
#include "v5_apitypes_patched.h"

#include <any>
#include <expected>

static std::error_condition map_fresult(FRESULT result) {
    // See http://elm-chan.org/fsw/ff/doc/rc.html for a description of FRESULT codes
    switch (result) {
        case FR_OK:
            return {};
        case FR_DISK_ERR:
            return std::errc::state_not_recoverable;
        case FR_INT_ERR:
            return std::errc::state_not_recoverable;
        case FR_NOT_READY:
            return std::errc::device_or_resource_busy;
        case FR_NO_FILE:
            return std::errc::no_such_file_or_directory;
        case FR_NO_PATH:
            return std::errc::no_such_file_or_directory;
        case FR_INVALID_NAME:
            return std::errc::invalid_argument;
        case FR_DENIED:
            return std::errc::permission_denied;
        case FR_EXIST:
            return std::errc::file_exists;
        case FR_INVALID_OBJECT:
            return std::errc::io_error;
        case FR_WRITE_PROTECTED:
            return std::errc::read_only_file_system;
        case FR_INVALID_DRIVE:
            return std::errc::no_such_device_or_address;
        case FR_NOT_ENABLED:
            return std::errc::io_error;
        case FR_NO_FILESYSTEM:
            return std::errc::no_such_device_or_address;
        case FR_MKFS_ABORTED:
            return std::errc::io_error;
        case FR_TIMEOUT:
            return std::errc::io_error;
        case FR_LOCKED:
            return std::errc::permission_denied;
        case FR_NOT_ENOUGH_CORE:
            return std::errc::not_enough_memory;
        case FR_TOO_MANY_OPEN_FILES:
            return std::errc::too_many_files_open_in_system;
        case FR_INVALID_PARAMETER:
            return std::errc::invalid_argument;
        default:
            __builtin_abort(); // TODO: log error
    }
}

std::expected<zest::fs::FileDescriptor, std::error_condition>
zest::fs::UsdDriver::open(const char* path, int _flags, int mode [[maybe_unused]]) {
    FRESULT result = this->with_lock(vexFileMountSD);
    if (result != FR_OK) {
        return std::unexpected{map_fresult(result)};
    }

    const zest::fs::FileFlags flags{_flags};
    File file = {nullptr, flags.write};

    if (flags.create_new && this->with_lock(vexFileStatus, path) != 0) {
        // User specified create_new but the file already exists
        return std::unexpected{std::errc::file_exists};
    }

    if (flags.read && !flags.write) {
        // Open in read-only mode
        file.fd = this->with_lock(vexFileOpen, path, ""); // mode is ignored
    } else if (flags.write && flags.append) {
        // Open in write & append mode
        file.fd = this->with_lock(vexFileOpenWrite, path);
    } else if (flags.write && flags.truncate) {
        // Open in write mode & truncate file
        file.fd = this->with_lock(vexFileOpenCreate, path);
    } else if (flags.write) {
        // Open in write & overwrite mode
        file.fd = this->with_lock(vexFileOpenWrite, path);
        this->with_lock(vexFileSeek, file.fd, 0, 0);
    } else {
        return std::unexpected{std::errc::invalid_argument};
    }
    if (file.fd == nullptr) {
        return std::unexpected{std::errc::too_many_files_open_in_system};
    }
    return this->add_vfs_entry(std::any{file});
}

std::expected<ssize_t, std::error_condition>
zest::fs::UsdDriver::read(std::any _file, std::span<std::byte> output) {
    auto [file, is_writable] = std::any_cast<File>(_file);
    if (is_writable) {
        return std::unexpected{std::errc::operation_not_permitted};
    }
    char* out_ptr = reinterpret_cast<char*>(output.data());
    return this->with_lock(vexFileRead, out_ptr, 1, output.size(), file);
}

std::expected<int, std::error_condition>
zest::fs::UsdDriver::write(std::any _file, std::span<std::byte> input) {
    auto [file, is_writable] = std::any_cast<File>(_file);
    if (!is_writable) {
        return std::unexpected{std::errc::operation_not_permitted};
    }
    char* in_ptr = reinterpret_cast<char*>(input.data());
    return this->with_lock(vexFileWrite, in_ptr, 1, input.size(), file);
}

std::expected<void, std::error_condition> zest::fs::UsdDriver::close(std::any _file) {
    FIL* file = std::any_cast<File>(_file).fd;
    this->with_lock(vexFileClose, file);
    return {};
}

std::expected<void, std::error_condition>
zest::fs::UsdDriver::fstat(std::any _file, struct stat* stats) {
    FIL* file = std::any_cast<File>(_file).fd;
    stats->st_size = this->with_lock(vexFileSize, file);
    // TODO: set st_mode based to indicate file vs directory
    return {};
}

bool zest::fs::UsdDriver::isatty(std::any _file [[maybe_unused]]) {
    return false;
}

std::expected<off_t, std::error_condition>
zest::fs::UsdDriver::lseek(std::any _file, off_t offset, int dir) {
    FIL* file = std::any_cast<File>(_file).fd;
    FRESULT result;
    switch (dir) {
        case 0: {
            result = this->with_lock(vexFileSeek, file, offset, 0);
            break;
        }
        case 1: {
            if (offset >= 0) {
                result = this->with_lock(vexFileSeek, file, offset, 1);
            } else {
                // VEXos does not support negative seek offsets, so we calculate the offset from the
                // start of the file ourselves.
                auto stream_pos = this->with_lock(vexFileTell, file);
                result = this->with_lock(vexFileSeek, file, stream_pos + offset, 0);
            }
            break;
        }
        case 2: {
            if (offset >= 0) {
                result = this->with_lock(vexFileSeek, file, offset, 2);
            } else {
                // VEXos does not support negative seek offsets, so we calculate the offset from the
                // start of the file ourselves.
                struct stat stats;
                if (auto result = this->fstat(_file, &stats); !result.has_value()) {
                    return std::unexpected{result.error()};
                }
                auto file_size = stats.st_size;
                result = this->with_lock(vexFileSeek, file, offset + file_size, 0);
            }
            break;
        }
        default: {
            return std::unexpected{std::errc::invalid_argument};
        }
    }
    if (auto err = map_fresult(result); err) {
        return std::unexpected{err};
    }
    return this->with_lock(vexFileTell, file);
}

int zest::fs::UsdDriver::ctl(std::any _file [[maybe_unused]], uint32_t, void* const) {
    return 0;
}