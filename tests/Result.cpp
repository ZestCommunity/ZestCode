#include "common/result.hpp"

class MyError : public zest::ResultError {};

void initialize() {
    zest::Result<int, MyError> res(2);
}
