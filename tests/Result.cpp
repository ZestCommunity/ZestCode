#include "common/result.hpp"

#include <cstdint>
#include <limits>

class MyError : public zest::ResultError {};

class MyError2 : public zest::ResultError {};

constexpr void compile_time_tests() {
    // test sentinel values
    static_assert(zest::sentinel_v<int32_t> == INT32_MAX);
    static_assert(zest::sentinel_v<float> == std::numeric_limits<float>::infinity());

    {
        // test comparison operator
        static_assert(zest::Result<int, MyError>(2) == zest::Result<int, MyError>(2));
        static_assert(zest::Result<int, MyError>(3) == zest::Result<float, MyError2>(3));
        static_assert(zest::Result<int, MyError>(0) == 0);
        static_assert(0 == zest::Result<int, MyError>(0));
    }
}

void runtime_tests() {}