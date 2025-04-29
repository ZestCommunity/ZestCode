#include "common/result.hpp"

class MyError : public zest::ResultError {};

class MyError2 : public zest::ResultError {};

class IntError : public zest::ResultError {
  public:
    IntError(int) {}
};

void initialize() {
    constexpr zest::Result<int, MyError> a(2);
    static_assert(a == 2);
    static_assert(a == a);
    static_assert(a == a.get());
    static_assert(a.get() == a.get<int>());
    constexpr zest::Result<int, MyError2> b(2);
    static_assert(a == b);
    auto c = a.get<MyError>();
}
