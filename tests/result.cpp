#include "common/result.hpp"

#include <cstdint>
#include <limits>

// there'll be a lot of unused variables, since we just want to see if it compiles
#pragma GCC diagnostic ignored "-Wunused-variable"

class MyError : public zest::ResultError {};

class MyError2 : public zest::ResultError {};

zest::Result<void, MyError> test_function_1() {
    // return nothing
    return {};
    // return MyError
    return MyError();
}

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
        static_assert(0.0 == zest::Result<int, MyError>(0));
        static_assert(zest::Result<int, MyError>(0) == 0.0);
    }

    {
        // test conversion operators
        zest::Result<int, MyError> a = 2;
        int& b = a;
        const int& c = a;
        int&& d = zest::Result<int, MyError>(2);
        const int&& e = zest::Result<int, MyError>(2);
        int f = a;
    }
}

void runtime_tests() {}
