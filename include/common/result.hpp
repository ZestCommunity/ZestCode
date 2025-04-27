#pragma once

#include <concepts>
#include <limits>
#include <optional>
#include <stacktrace>
#include <type_traits>
#include <variant>

namespace zest {

// custom error types inherit from the ResultError class, which enforces that custom error types
// have some shared functionality.
class ResultError {
  public:
    // since this constructor has no arguments, it'll be called implicitly by the constructor of any
    // child class.
    ResultError()
        : stacktrace(std::stacktrace::current()) {}

    std::stacktrace stacktrace;
};

// concept used to enforce that error types inherit from the ResultError class
template<typename T>
concept CustomError = std::derived_from<T, ResultError>;

// Some primitive types such as float, double, etc may have a standardized "sentinel" value.
// For example, a function that returns a float may return INFINITY if an error occurs.
// Usually the maximum value of the type is returned if there is an error.
// This trait helps simplify DX.
// This trait can also be specialized for custom types (e.g unitized types)
template<typename T>
class HasSentinel;

// concept which can be used to determine if a type has a sentinel value
template<typename T>
concept Sentinel = requires(const T& val) { HasSentinel<T>::value; };

// templated variable which can be used to simplify usage of HasSentinel
template<Sentinel T>
constexpr T sentinel_v = HasSentinel<T>::value;

// partial specialization for HasSentinel.
// any integral type (e.g double, int, uint8, etc) has a sentinel value equal to its maximum value
template<std::integral T>
class HasSentinel<T> {
    static constexpr T value = std::numeric_limits<T>::max();
};

// Result class.
// An alternative to std::expected, where an expected value can be contained alongside an unexpected
// error value.
// This means that Result will always contain an expected value.
// Therefore, if the user does not check if a function returned an error, an exception will NOT be
// thrown, and therefore the thread won't crash.
template<typename T, CustomError... Errs>
class Result {
  public:
    // Construct a Result with a value and no error value.
    // Constraint: the value member variable must be able to be constructed with the value
    // parameter.
    template<typename U>
        requires std::constructible_from<T, U>
    constexpr Result(U&& value)
        : value(std::forward<U>(value)) {}

    // Construct a Result with a value and an error value.
    // Constraint: the value member variable must be able to be constructed with the value
    // parameter.
    // Constraint: the error parameter must be of a type that may be contained in the error member
    // variable.
    template<typename U, CustomError E>
        requires std::constructible_from<T, U>
                     && (std::is_same_v<Errs, std::remove_cvref<E>> || ...)
    constexpr Result(U&& value, E&& error)
        : value(std::forward<U>(value)),
          error(std::forward<E>(error)) {}

    // Construct a Result with an error, initializing the normal value to its sentinel value.
    // Constraint: the type of the normal value must have a specialized sentinel value.
    // Constraint: the error parameter must be of a type that may be contained in the error member
    // variable.
    template<CustomError U>
        requires Sentinel<T> && (std::is_same_v<Errs, std::remove_cvref<U>> || ...)
    constexpr Result(U&& error)
        : value(sentinel_v<T>),
          error(std::forward<U>(error)) {}

    // Check whether the result contains an error of the given type
    // Constraint: the custom error type to check for must be able to be contained in the error
    // member variable.
    template<CustomError U>
        requires(std::is_same_v<Errs, std::remove_cvref<U>> || ...)
    constexpr U has() const& {
        if (!error.has_value()) {
            return false;
        } else {
            return std::holds_alternative<U>(error);
        }
    };

    // return an optional wrapping the given error type.
    // Constraint: the custom error type to get must be able to be contained in the error
    // member variable.
    template<CustomError U>
        requires(std::is_same_v<Errs, std::remove_cvref<U>> || ...)
    constexpr std::optional<U> get() const& {
        if (this->has<U>()) {
            return std::get<U>(error.value());
        } else {
            return std::nullopt;
        }
    }

    // return an optional wrapping the given error type.
    // Constraint: the custom error type to get must be able to be contained in the error
    // member variable.
    template<CustomError U>
        requires(std::is_same_v<Errs, std::remove_cvref<U>> || ...)
    constexpr std::optional<U> get() && {
        if (this->has<U>()) {
            return std::get<U>(std::move(error.value()));
        } else {
            return std::nullopt;
        }
    }

    // implicit conversion operator, on an l-value
    constexpr operator T() const& {
        return value;
    };

    // implicit conversion operator, on an r-value
    constexpr operator T() && {
        return std::move(value);
    }

    // the optional error value.
    // a variant that could contain any of the specified error types.
    std::optional<std::variant<Errs...>> error;
    // the normal value
    T value;
};

} // namespace zest