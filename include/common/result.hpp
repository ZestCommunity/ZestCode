#pragma once

#include <format>
#include <iostream>
#include <optional>
#include <stacktrace>
#include <type_traits>

namespace zest {

template<typename E>
    requires std::is_scoped_enum_v<E>
struct ResultError {
    E type;
    std::string message;
    std::stacktrace stacktrace;
};

/**
 * @brief Result class, used for error handling.
 *
 * The Result class can contain a value and optionally an error. Unlike std::expected, the value is
 * ALWAYS valid. This means that a failure to handle an error by the user will not crash the thread.
 *
 * Instances of this class can be implicitly converted to the "normal" type, so UX is not
 * complicated at all.
 *
 * @tparam T the "normal" value.
 * @tparam E the error type, has to be an enum class.
 *
 * @b Example
 * @code
 * enum class ExampleError {
 *     ExampleA,
 *     ExampleB,
 * };
 *
 * zest::Result<double, ExampleError> get_double() {
 *     // for the purposes of this example, this function will always return an error
 *     return {INFINITY, ExampleError::ExampleA, "I can be formatted{}", "!!!"};
 * }
 *
 * void initialize() {
 * }
 *
 * @endcode
 */
template<typename T, typename E>
    requires std::is_scoped_enum_v<E>
class Result {
  public:
    template<typename U>
        requires std::constructible_from<T, U>
    Result(U&& val)
        : val(std::forward<T>(val)) {}

    Result(Result&& other)
        : val(std::move(other.val)),
          error(std::move(other.error)) {}

    template<typename U, typename F>
        requires std::constructible_from<T, U> && std::constructible_from<ResultError<E>, F>
    Result(U&& val, F&& error)
        : val(std::forward<T>(val)),
          error(std::forward<ResultError>(error)) {}

    template<typename U, typename... Args>
        requires std::constructible_from<T, U>
    Result(U&& val, E type, std::format_string<Args...> fmt, Args&&... args)
        : val(std::forward<T>(val)),
          error({type, std::format(fmt, std::forward<Args>(args)...), std::stacktrace::current()}) {
    }

    operator T() const& {
        return val;
    }

    operator T() && {
        return std::move(val);
    }

    T val;
    std::optional<ResultError<E>> error;
};

} // namespace zest

template<typename E>
struct std::formatter<zest::ResultError<E>> {
    constexpr auto parse(std::format_parse_context& ctx) {
        return ctx.begin();
    }

    auto format(const zest::ResultError<E>& error, std::format_context& ctx) const {
        return std::format_to(
            ctx.out(),
            "{}\n"
            "begin stacktrace\n"
            "{}\n"
            "end stacktrace",
            error.message,
            error.stacktrace
        );
    }
};

template<typename E>
std::ostream& operator<<(std::ostream& os, const zest::ResultError<E>& error) {
    os << error.message << std::endl
       << "begin stacktrace" << std::endl
       << error.stacktrace << std::endl
       << "end stacktrace";
    return os;
}