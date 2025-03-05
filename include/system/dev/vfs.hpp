#pragma once

#include <compare>
#include <cstddef>
#include <cstdint>
#include <sys/stat.h>
#include <unistd.h>

namespace zest {

class FileDescriptor {
  public:
    FileDescriptor(int fd)
        : m_fd(fd) {}

    std::strong_ordering operator<=>(const FileDescriptor&) const = default;

  private:
    int m_fd;
};

class FileDriver {
  public:
    virtual FileDescriptor open(const char* path, int flags, int mode) = 0;
    virtual ssize_t read(void* const, uint8_t*, const size_t) = 0;
    virtual int write(void* const, uint8_t*, const size_t) = 0;
    virtual int close(void* const) = 0;
    virtual int fstat(void* const, struct stat*) = 0;
    virtual int isatty(void* const) = 0;
    virtual off_t lseek(void* const, off_t, int) = 0;
    virtual int ctl(void* const, const uint32_t, void* const) = 0;
    virtual ~FileDriver() = default;
};
} // namespace zest