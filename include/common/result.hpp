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
 *
 */
class ResultError {
  public:
    /**
     * @brief struct containing data that can only be known at runtime.
     *
     */
    struct RuntimeData {
        std::stacktrace stacktrace;
        std::chrono::time_point<std::chrono::system_clock> time;
    };

    /**
     * @brief Construct a new ResultError object.
     *
     * @details Captures the current stacktrace and system time if called at runtime.
     */
    constexpr ResultError() {
        if !consteval {
            runtime_data = {
                .stacktrace = std::stacktrace::current(),
                .time = std::chrono::system_clock::now()
            };
        }
    }

    std::optional<RuntimeData> runtime_data;
};

/**
 * @brief Unknown Error
 *
 */
class UnknownError : public ResultError {
  public:
    template<typename T>
        requires std::convertible_to<T, std::string>
    UnknownError(T&& message)
        : message(std::forward<T>(message)) {}

    std::string message;
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
        requires std::constructible_from<T, U>
                     && (std::same_as<std::remove_cvref_t<E>, Errs> || ...)
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
        requires Sentinel<T> && (std::same_as<std::remove_cvref_t<E>, Errs> || ...)
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
     * @brief Get an error of type E if present (const rvalue overload).
     * @tparam E Error type to retrieve.
     * @return std::optional<E> Contains the error if present; otherwise nullopt.
     */
    template<typename E>
        requires(std::same_as<E, Errs> || ...)
    constexpr const std::optional<E> get() const&& {
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

    constexpr operator T&() & {
        return value;
    }

    constexpr operator const T&() const& {
        return value;
    };

    constexpr operator T&&() && {
        return std::move(value);
    }

    constexpr operator const T&&() const&& {
        return std::move(value);
    }

    /**
     * @brief error value
     * @details instead of wrapping the variant in std::optional, it's more efficient to use
     * std::monostate. since we have to use std::variant in any case.
     */
    std::variant<std::monostate, Errs...> error;
    T value;
};

/**
 * @brief compare Result instances with comparable normal values
 *
 * @tparam LhsT the normal value type of the left-hand side argument
 * @tparam RhsT the normal value type of the right-hand side argument
 * @tparam LhsErrs the error value types of the left-hand side argument
 * @tparam RhsErrs the error value types of the right-hand side argument
 * @param lhs the left-hand side of the expression
 * @param rhs the right-hand side of the expression
 * @return true if the values are equal
 * @return false if the values are not equal
 */
template<typename LhsT, typename RhsT, typename... LhsErrs, typename... RhsErrs>
    requires std::equality_comparable_with<LhsT, RhsT>
constexpr bool
operator==(const Result<LhsT, LhsErrs...>& lhs, const Result<RhsT, RhsErrs...>& rhs) {
    return lhs.value == rhs.value;
}

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
        requires(std::same_as<std::remove_cvref_t<E>, Errs> || ...)
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
     * @brief Get an error of type E if present (const rvalue overload).
     * @tparam E Error type to retrieve.
     * @return std::optional<E> Contains the error if present; otherwise nullopt.
     */
    template<typename E>
        requires(std::same_as<E, Errs> || ...)
    constexpr const std::optional<E> get() const&& {
        if (std::holds_alternative<E>(error)) {
            return std::move(std::get<E>(error));
        } else {
            return std::nullopt;
        }
    }

    std::variant<std::monostate, Errs...> error; ///< Variant holding an error or monostate.
};

} // namespace zest