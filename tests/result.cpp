#include "common/result.hpp"

#include <string>

// there'll be a lot of unused variables, since we just want to see if it compiles
#pragma GCC diagnostic ignored "-Wunused-variable"

class MyError {
  public:
    constexpr MyError(std::string) {}

    constexpr MyError() {}
};

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
    {
        // check constructors
        zest::Result<int, MyError> a(MyError{});
        zest::Result<int, MyError> b("test_MyError_conversion");
        zest::Result<int, MyError> c(2);
        zest::Result<int, MyError> d(2.0);
        zest::Result<int, MyError> e;
        zest::Result<void, MyError> f(MyError{});
        zest::Result<void, MyError> g("test_MyError_conversion");
    }

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
