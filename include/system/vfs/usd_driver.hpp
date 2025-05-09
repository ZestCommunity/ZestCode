#pragma once

#include "pros/rtos.hpp"
#include "system/vfs/file_driver.hpp"
#include "v5_apitypes_patched.h"

#include <mutex>

namespace zest::fs {
class UsdDriver : public FileDriver {
  public:
    void init() override {}

    std::expected<FileDescriptor, std::error_condition>
    open(const char* path, int flags, int mode) override;
    std::expected<ssize_t, std::error_condition> read(std::any, std::span<std::byte>) override;
    std::expected<int, std::error_condition> write(std::any, std::span<std::byte>) override;
    std::expected<void, std::error_condition> close(std::any) override;
    std::expected<void, std::error_condition> fstat(std::any, struct stat*) override;
    bool isatty(std::any) override;
    std::expected<off_t, std::error_condition> lseek(std::any, off_t, int) override;
    int ctl(std::any, const uint32_t, void* const) override;
    ~UsdDriver() override = default;

  private:
    struct File {
        FIL* fd;
        bool is_writable;
    };

    template<typename F, typename... Args>
    auto inline with_lock(F&& fun, Args... args) {
        std::lock_guard lock{this->mut};
        return fun(args...);
    }

    pros::RecursiveMutex mut{};
};

} // namespace zest::fs