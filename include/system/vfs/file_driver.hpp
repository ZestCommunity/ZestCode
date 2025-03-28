#pragma once

#include "system/vfs/file_descriptor.hpp"

#include <any>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <expected>
#include <memory>
#include <span>
#include <sys/stat.h>
#include <system_error>
#include <unistd.h>

namespace zest::fs {
class FileDriver : public std::enable_shared_from_this<FileDriver> {
  public:
    virtual void init() = 0;
    virtual std::expected<FileDescriptor, std::error_condition>
    open(const char* path, int flags, int mode) = 0;
    virtual ssize_t read(std::any, std::span<std::byte>) = 0;
    virtual int write(std::any, std::span<std::byte>) = 0;
    virtual int close(std::any) = 0;
    virtual int fstat(std::any, struct stat*) = 0;
    virtual int isatty(std::any) = 0;
    virtual off_t lseek(std::any, off_t, int) = 0;
    virtual int ctl(std::any, const uint32_t, void* const) = 0;
    FileDescriptor add_vfs_entry(std::any data);
    int32_t update_vfs_entry(FileDescriptor fd, std::any data);
    virtual ~FileDriver() = default;
};
} // namespace zest::fs