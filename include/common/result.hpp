#pragma once

#include <chrono>
#include <concepts>
#include <limits>
#include <optional>
#include <stacktrace>
#include <variant>

namespace zest {

/**
 * @brief Base class for custom error types used in the Result class.
 * @details Enforces stacktrace and timestamp functionality for derived error types.
 */
class ResultError {
  public:
    /**
     * @brief Construct a new ResultError object.
     * @details Captures the current stacktrace and system time automatically.
     */
    ResultError()
        : stacktrace(std::stacktrace::current()),
          time(std::chrono::system_clock::now()) {}

    std::stacktrace stacktrace; ///< Captured stacktrace at error creation.
    std::chrono::time_point<std::chrono::system_clock> time; ///< Timestamp of error creation.
};

/**
 * @brief Trait to define a "sentinel" value for types indicating an error state.
 * @tparam T Type to provide a sentinel value for.
 * @note Specialize this template for custom types if needed.
 */
template<typename T>
class SentinelValue;

/**
 * @brief Concept to check if a type has a defined sentinel value.
 * @tparam T Type to check.
 */
template<typename T>
concept Sentinel = requires(const T& val) { SentinelValue<T>::value; };

/**
 * @brief Helper variable to simplify access to a type's sentinel value.
 * @tparam T Type with a defined sentinel (must satisfy Sentinel concept).
 */
template<Sentinel T>
constexpr T sentinel_v = SentinelValue<T>::value;

/**
 * @brief Partial specialization of SentinelValue for integral and floating-point types.
 * @tparam T Integral or floating-point type.
 * @details Uses infinity for floating-point types if available; otherwise uses max value.
 */
template<typename T>
    requires(std::integral<T> || std::floating_point<T>)
class SentinelValue<T> {
  public:
    static constexpr T get() {
        if constexpr (std::numeric_limits<T>::has_infinity) {
            return std::numeric_limits<T>::infinity();
        } else {
            return std::numeric_limits<T>::max();
        }
    }

    static constexpr T value = get(); ///< Precomputed sentinel value for type T.
};

/**
 * @brief Result class for expected value or error handling (similar to std::expected).
 * @tparam T Type of the expected value.
 * @tparam Errs List of possible error types (must inherit from ResultError).
 * @note Errors are stored in a variant, and the value is always initialized.
 */
template<typename T, typename... Errs>
    requires(sizeof...(Errs) > 0) && (std::derived_from<Errs, ResultError> && ...)
class Result {
  public:
    /**
     * @brief Construct a Result with a normal value (no error).
     * @tparam U Type convertible to T.
     * @param value Value to initialize the result with.
     */
    template<typename U>
        requires std::constructible_from<T, U>
    constexpr Result(U&& value)
        : error(std::monostate()),
          value(std::forward<U>(value)) {}

    /**
     * @brief Construct a Result with a value and an error.
     * @tparam U Type convertible to T.
     * @tparam E Error type (must be in Errs).
     * @param value Value to store.
     * @param error Error to store.
     */
    template<typename U, typename E>
        requires std::constructible_from<T, U> && (std::same_as<std::remove_cvref<E>, Errs> || ...)
    constexpr Result(U&& value, E&& error)
        : value(std::forward<U>(value)),
          error(std::forward<E>(error)) {}

    /**
     * @brief Construct a Result with an error, initializing the value to its sentinel.
     * @tparam E Error type (must be in Errs).
     * @param error Error to store.
     * @note Requires T to have a defined sentinel value (via SentinelValue<T>).
     */
    template<typename E>
        requires Sentinel<T> && (std::same_as<std::remove_cvref<E>, Errs> || ...)
    constexpr Result(E&& error)
        : error(std::forward<E>(error)),
          value(sentinel_v<T>) {}

    /**
     * @brief Get an error of type E if present (const-qualified overload).
     * @tparam E Error type to retrieve.
     * @return std::optional<E> Contains the error if present; otherwise nullopt.
     */
    template<typename E>
        requires(std::same_as<E, Errs> || ...)
    constexpr std::optional<E> get() const& {
        if (std::holds_alternative<E>(error)) {
            return std::get<E>(error);
        } else {
            return std::nullopt;
        }
    }

    /**
     * @brief Get an error of type E if present (rvalue overload).
     * @tparam E Error type to retrieve.
     * @return std::optional<E> Contains the error if present; otherwise nullopt.
     */
    template<typename E>
        requires(std::same_as<E, Errs> || ...)
    constexpr std::optional<E> get() && {
        if (std::holds_alternative<E>(error)) {
            return std::move(std::get<E>(error));
        } else {
            return std::nullopt;
        }
    }

    /**
     * @brief Get the stored value (const-qualified overload).
     * @return T Copy of the stored value.
     */
    template<typename U = T>
        requires std::same_as<U, T>
    constexpr T get() const& {
        return value;
    }

    /**
     * @brief Get the stored value (rvalue overload).
     * @return T Moved value.
     */
    template<typename U = T>
        requires std::same_as<U, T>
    constexpr T get() && {
        return std::move(value);
    }

    /**
     * @brief Implicit conversion to the stored value (const-qualified).
     */
    constexpr operator T() const& {
        return value;
    };

    /**
     * @brief Implicit conversion to the stored value (rvalue).
     */
    constexpr operator T() && {
        return std::move(value);
    }

    /**
     * @brief Equality comparison operator.
     * @tparam U Type of the other result's value.
     * @tparam Es Other result's error types.
     * @param other Result to compare with.
     * @return true If values are equal.
     */
    template<typename U, typename... Es>
        requires std::equality_comparable_with<T, U>
    constexpr bool operator==(const Result<U, Es...>& other) {
        return value == other.value;
    }

    std::variant<std::monostate, Errs...> error; ///< Variant holding an error or monostate.
    T value;                                     ///< The stored value (always initialized).
};

/**
 * @brief Result specialization for void value type (no stored value).
 * @tparam Errs List of possible error types (must inherit from ResultError).
 */
template<typename... Errs>
    requires(sizeof...(Errs) > 0) && (std::derived_from<Errs, ResultError> && ...)
class Result<void, Errs...> {
  public:
    /**
     * @brief Construct a Result with an error.
     * @tparam E Error type (must be in Errs).
     * @param error Error to store.
     */
    template<typename E>
        requires(std::same_as<std::remove_cvref<E>, Errs> || ...)
    constexpr Result(E&& error)
        : error(std::forward<E>(error)) {}

    /**
     * @brief Construct a Result with no error (success state).
     */
    constexpr Result()
        : error(std::monostate()) {}

    /**
     * @brief Get an error of type E if present (const-qualified overload).
     * @tparam E Error type to retrieve.
     * @return std::optional<E> Contains the error if present; otherwise nullopt.
     */
    template<typename E>
        requires(std::same_as<E, Errs> || ...)
    constexpr std::optional<E> get() const& {
        if (std::holds_alternative<std::monostate>(error)) {
            return std::nullopt;
        } else {
            return std::get<E>(error);
        }
    }

    /**
     * @brief Get an error of type E if present (rvalue overload).
     * @tparam E Error type to retrieve.
     * @return std::optional<E> Contains the error if present; otherwise nullopt.
     */
    template<typename E>
        requires(std::same_as<E, Errs> || ...)
    constexpr std::optional<E> get() && {
        if (std::holds_alternative<std::monostate>(error)) {
            return std::nullopt;
        } else {
            return std::move(std::get<E>(error));
        }
    }

    std::variant<std::monostate, Errs...> error; ///< Variant holding an error or monostate.
};

} // namespace zest