#pragma once

#include <compare>

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
} // namespace zest::fs