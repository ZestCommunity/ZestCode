#pragma once

#include <any>
#include <compare>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <memory>
#include <span>
#include <sys/stat.h>
#include <unistd.h>

namespace zest::fs {

class FileDescriptor {
  public:
    constexpr FileDescriptor(int fd)
        : m_fd(fd) {}

    explicit operator int() const {
        return m_fd;
    }

    std::strong_ordering operator<=>(const FileDescriptor&) const = default;

  private:
    int m_fd;
};

inline constexpr FileDescriptor FD_STDIN{0};
inline constexpr FileDescriptor FD_STDOUT{1};
inline constexpr FileDescriptor FD_STDERR{2};

class FileDriver : public std::enable_shared_from_this<FileDriver> {
  public:
    virtual void init() = 0;
    virtual FileDescriptor open(const char* path, int flags, int mode) = 0;
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

struct FileEntry {
  std::shared_ptr<FileDriver> driver;
  std::any data;
};

} // namespace zest::fs