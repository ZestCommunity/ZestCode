#include "pros/rtos.hpp"
#include "system/vfs/vfs.hpp"

namespace zest::fs {

class UsdDriver : public FileDriver {
  public:
    void init() override {}
    std::expected<FileDescriptor, std::error_condition> open(const char* path, int flags, int mode) override;
    ssize_t read(std::any, std::span<std::byte>) override;
    int write(std::any, std::span<std::byte>) override;
    int close(std::any) override;
    int fstat(std::any, struct stat*) override;
    int isatty(std::any) override;
    off_t lseek(std::any, off_t, int) override;
    int ctl(std::any, const uint32_t, void* const) override;
    ~UsdDriver() override = default;
private:
    pros::RecursiveMutex mut{};
};

} // namespace zest::fs