#include "common/result.hpp"

// there'll be a lot of unused variables, since we just want to see if it compiles
#pragma GCC diagnostic ignored "-Wunused-variable"

class MyError {};

class MyError2 {};

zest::Result<void, MyError> test_function_1() {
    // return nothing
    return {};
    // return MyError
    return MyError();
}

zest::Result<int, MyError> test_function_2() {
    // return an integer
    return 1;
    // return MyError
    return MyError();
}

constexpr void compile_time_tests() {
    // test sentinel values
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

    {
        // test error getting
    }
}

void runtime_tests() {}
