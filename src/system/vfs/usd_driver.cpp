#include "system/vfs/usd_driver.hpp"

#include "system/vfs/file_flags.hpp"
#include "v5_api.h"
#include "v5_apitypes.h"

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
    FRESULT result = vexFileMountSD();
    FIL* file = nullptr;
    if (result != FR_OK) {
        return std::unexpected{map_fresult(result)};
    }

    const zest::fs::FileFlags flags{_flags};

    if (flags.create_new && vexFileStatus(path) != 0) {
        // User specified create_new but the file already exists
        return std::unexpected{std::errc::file_exists};
    }

    if (flags.read && !flags.write) {
        // Open in read-only mode
        file = vexFileOpen(path, ""); // mode is ignored
    } else if (flags.write && flags.append) {
        // Open in write & append mode
        file = vexFileOpenWrite(path);
    } else if (flags.write && flags.truncate) {
        // Open in write mode & truncate file
        file = vexFileOpenCreate(path);
    } else if (flags.write) {
        // Open in write & overwrite mode
        file = vexFileOpenWrite(path);
        vexFileSeek(file, 0, 0);
    } else {
        return std::unexpected{std::errc::invalid_argument};
    }
    if (file == nullptr) {
        return std::unexpected{std::errc::too_many_files_open_in_system};
    }
    return this->add_vfs_entry(std::any{file});
}

ssize_t zest::fs::UsdDriver::read(std::any _file, std::span<std::byte> output) {
    FIL* file = std::any_cast<FIL*>(_file);
    return vexFileRead(reinterpret_cast<char*>(output.data()), 1, output.size(), file);
}