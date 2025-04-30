#pragma once

#include <chrono>
#include <concepts>
#include <limits>
#include <optional>
#include <stacktrace>
#include <variant>

namespace zest {

// custom error types inherit from the ResultError class, which enforces that custom error types
// have some shared functionality.
class ResultError {
  public:
    // since this constructor has no arguments, it'll be called implicitly by the constructor of any
    // child class.
    ResultError()
        : stacktrace(std::stacktrace::current()),
          time(std::chrono::system_clock::now()) {}

    std::stacktrace stacktrace;
    std::chrono::time_point<std::chrono::system_clock> time;
};

// Some primitive types such as float, double, etc may have a standardized "sentinel" value.
// For example, a function that returns a float may return INFINITY if an error occurs.
// Usually the maximum value of the type is returned if there is an error.
// This trait helps simplify DX.
// This trait can also be specialized for custom types (e.g unitized types)
template<typename T>
class SentinelValue;

// concept which can be used to determine if a type has a sentinel value
template<typename T>
concept Sentinel = requires(const T& val) { SentinelValue<T>::value; };

// templated variable which can be used to simplify usage of HasSentinel
template<Sentinel T>
constexpr T sentinel_v = SentinelValue<T>::value;

// partial specialization for HasSentinel.
// any integral type (e.g double, int, uint8, etc) has a sentinel value equal to its maximum value
template<std::integral T>
class SentinelValue<T> {
  public:
    static constexpr T value = std::numeric_limits<T>::max();
};

// Result class.
// An alternative to std::expected, where an expected value can be contained alongside an unexpected
// error value.
// This means that Result will always contain an expected value.
// Therefore, if the user does not check if a function returned an error, an exception will NOT be
// thrown, and therefore the thread won't crash.
// Constraint: requires at least 1 possible Error type
// Constraint: Error types must inherit from ResultError
template<typename T, typename... Errs>
    requires(sizeof...(Errs) > 0) && (std::derived_from<Errs, ResultError> && ...)
class Result {
  public:
    // Construct a Result with a value and no error value.
    // Constraint: type T can be constructed from type U
    template<typename U>
        requires std::constructible_from<T, U>
    constexpr Result(U&& value)
        : error(std::monostate()),
          value(std::forward<U>(value)) {}

    // Construct a Result with a value and an error value.
    // Constraint: type T can be constructed from type U
    // Constraint: E in Errs
    template<typename U, typename E>
        requires std::constructible_from<T, U> && (std::same_as<std::remove_cvref<E>, Errs> || ...)
    constexpr Result(U&& value, E&& error)
        : value(std::forward<U>(value)),
          error(std::forward<E>(error)) {}

    // Construct a Result with an error, initializing the normal value to its sentinel value.
    // Constraint: type T has a sentinel value
    // Constraint: E in Errs
    template<typename E>
        requires Sentinel<T> && (std::same_as<std::remove_cvref<E>, Errs> || ...)
    constexpr Result(E&& error)
        : error(std::forward<E>(error)),
          value(sentinel_v<T>) {}

    // Get the given error type, if it exists
    // Constraint: E in Errs
    template<typename E>
        requires(std::same_as<E, Errs> || ...)
    constexpr std::optional<E> get() const& {
        if (std::holds_alternative<std::monostate>(error)) {
            return std::nullopt;
        } else {
            return std::get<E>(error);
        }
    }

    // Get the given error type, if it exists
    // Constraint: E in Errs
    template<typename E>
        requires(std::same_as<E, Errs> || ...)
    constexpr std::optional<E> get() && {
        if (std::holds_alternative<std::monostate>(error)) {
            return std::nullopt;
        } else {
            return std::move(std::get<E>(error));
        }
    }

    // get the normal value
    template<typename U = T>
        requires std::same_as<U, T>
    constexpr T get() const& {
        return value;
    }

    // get the normal value
    template<typename U = T>
        requires std::same_as<U, T>
    constexpr T get() && {
        return std::move(value);
    }

    // implicit conversion operator
    constexpr operator T() const& {
        return value;
    };

    // implicit conversion operator
    constexpr operator T() && {
        return std::move(value);
    }

    // comparison operator overload
    template<typename U, typename... Es>
        requires std::equality_comparable_with<T, U>
    constexpr bool operator==(const Result<U, Es...>& other) {
        return value == other.value;
    }

    // a variant that could contain any of the specified error types
    std::variant<std::monostate, Errs...> error;
    // the normal value
    T value;
};

// Result specialization for a value of type void, which would otherwise be impossible.
// This specialization simply does not hold a "normal" value
// Constraint: requires at least 1 possible Error type
// Constraint: Error types must inherit from ResultError
template<typename... Errs>
    requires(sizeof...(Errs) > 0) && (std::derived_from<Errs, ResultError> && ...)
class Result<void, Errs...> {
  public:
    // construct with an error value
    template<typename E>
        requires(std::same_as<std::remove_cvref<E>, Errs> || ...)
    constexpr Result(E&& error)
        : error(std::forward<E>(error)) {}

    // construct with no error value
    constexpr Result()
        : error(std::monostate()) {}

    // Get the given error type, if it exists
    // Constraint: E in Errs
    template<typename E>
        requires(std::same_as<E, Errs> || ...)
    constexpr std::optional<E> get() const& {
        if (std::holds_alternative<std::monostate>(error)) {
            return std::nullopt;
        } else {
            return std::get<E>(error);
        }
    }

    // Get the given error type, if it exists
    // Constraint: E in Errs
    template<typename E>
        requires(std::same_as<E, Errs> || ...)
    constexpr std::optional<E> get() && {
        if (std::holds_alternative<std::monostate>(error)) {
            return std::nullopt;
        } else {
            return std::move(std::get<E>(error));
        }
    }

    std::variant<std::monostate, Errs...> error;
};
} // namespace zest