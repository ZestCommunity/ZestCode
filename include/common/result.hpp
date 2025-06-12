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

    std::optional<RuntimeData> runtime_data;

  protected:
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
};

namespace traits {

/**
 * @brief Check whether a given type is in a typename pack
 *
 * @tparam T the type to check
 * @tparam Ts the typename pack to check
 */
template<typename T, typename... Ts>
inline constexpr bool is_in_pack_v = (std::is_same_v<std::remove_cvref_t<T>, Ts> || ...);

/**
 * @brief type trait for checking whether a type is derived from the ResultError class
 */
template<typename>
struct is_result_error : std::false_type {};

/**
 * @brief type trait for checking whether a type is derived from the ResultError class
 */
template<typename T>
    requires std::derived_from<ResultError, T>
struct is_result_error<T> : std::true_type {};

/**
 * @brief whether the given type is derived from the ResultError class
 */
template<typename T>
inline constexpr bool is_result_error_v = is_result_error<T>::value;

/**
 * @brief IsResultError concept. Enforces that the given type is derived from the ResultError class
 */
template<typename T>
concept IsResultError = is_result_error_v<std::remove_cvref_t<T>>;

/**
 * @brief Trait to define a "sentinel" value for types indicating an error state.
 * @tparam T Type to provide a sentinel value for.
 * @note Specialize this template for custom types if needed.
 */
template<typename T>
class SentinelValue;

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
 * @brief Concept to check if a type has a defined sentinel value.
 * @tparam T Type to check.
 */
template<typename T>
concept HasSentinel = requires(const T& val) { SentinelValue<T>::value; };

/**
 * @brief Helper variable to simplify access to a type's sentinel value.
 * @tparam T Type with a defined sentinel (must satisfy Sentinel concept).
 */
template<HasSentinel T>
inline constexpr T sentinel_v = SentinelValue<T>::value;

} // namespace traits

// forward declarations, necessary to declare traits
template<typename T, traits::IsResultError... Errs>
    requires(sizeof...(Errs) > 0)
class Result;

namespace traits {

/**
 * @brief Type trait to check if a type is a Result.
 * @tparam T Type to check.
 */
template<typename>
struct is_result : std::false_type {};

/**
 * @brief Type trait to check if a type is a Result.
 * @tparam T Type to check.
 */
template<typename T, typename... Errs>
struct is_result<Result<T, Errs...>> : std::true_type {};

/**
 * @brief Check whether a type is a Result
 * @tparam T Type to check
 */
template<typename T>
inline constexpr bool is_result_v = is_result<T>::value;

/**
 * @brief Concept to check if a type is a Result
 *
 * @tparam T
 */
template<typename T>
concept IsResult = is_result_v<T>;

} // namespace traits

/**
 * @brief Result class for expected value or error handling (similar to std::expected).
 * @tparam T Type of the expected value.
 * @tparam Errs List of possible error types (must inherit from ResultError).
 * @note Errors are stored in a variant, and the value is always initialized.
 */
template<typename T, traits::IsResultError... Errs>
    requires(sizeof...(Errs) > 0)
class Result {
  public:
    /**
     * @brief Construct a Result with a normal value (no error).
     * @tparam U Type convertible to T, and U not derived from ResultError
     * @param value Value to initialize the result with.
     */
    template<typename U>
        requires std::constructible_from<T, U> && (!traits::IsResultError<U>)
    constexpr Result(U&& value)
        : m_error(std::monostate()),
          m_value(std::forward<U>(value)) {}

    /**
     * @brief Construct a Result with a value and an error.
     * @tparam U Type convertible to T, and U not derived from ResultError
     * @tparam E Error type, must be in Errs and must be derived from ResultError
     * @param value Value to store.
     * @param error Error to store.
     */
    template<typename U, traits::IsResultError E>
        requires std::constructible_from<T, U> && (!traits::IsResultError<U>)
                     && traits::is_in_pack_v<E, Errs...>
    constexpr Result(U&& value, E&& error)
        : m_value(std::forward<U>(value)),
          m_error(std::forward<E>(error)) {}

    /**
     * @brief Construct a Result with an error, initializing the value to its sentinel.
     * @tparam E Error type, must be in Errs and derived from ResultError.
     * @param error Error to store.
     * @note Requires T to have a defined sentinel value (via SentinelValue<T>).
     */
    template<traits::IsResultError E>
        requires traits::HasSentinel<T> && traits::is_in_pack_v<E, Errs...>
    constexpr Result(E&& error)
        : m_error(std::forward<E>(error)),
          m_value(traits::sentinel_v<T>) {}

    /**
     * @brief Get the error of the given type, wrapped in std::optional
     *
     * @tparam Self the deduced self type
     * @tparam E the error type to get
     * @param self the self object
     * @return the error type, wrapped in std::optional
     */
    template<typename Self, traits::IsResultError E>
        requires traits::is_in_pack_v<E, Errs...>
    constexpr auto&& get_error(this Self&& self) {
        if (std::holds_alternative<E>(self.m_error)) {
            return std::optional(std::holds_alternative<E>(std::forward<Self>(self).m_error));
        } else {
            return std::optional<E>();
        }
    }

    /**
     * @brief Get the normal value
     *
     * @tparam Self the deduced self type
     * @param self the self object
     * @return the normal value
     */
    template<typename Self>
    constexpr auto&& get_value(this Self&& self) {
        return std::forward<Self>(self).m_value;
    }

    /**
     * @brief whether there is an error value
     *
     * @return true
     * @return false
     */
    constexpr bool has_error() {
        return !std::holds_alternative<std::monostate>(m_error);
    }

    constexpr operator T&() & {
        return m_value;
    }

    constexpr operator const T&() const& {
        return m_value;
    };

    constexpr operator T&&() && {
        return std::move(m_value);
    }

    constexpr operator const T&&() const&& {
        return std::move(m_value);
    }

  private:
    // instead of wrapping the variant in std::optional, we can use std::monostate
    std::variant<std::monostate, Errs...> m_error;
    T m_value;
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
    return lhs.get_value() == rhs.get_value();
}

/**
 * @brief Result specialization for void value type (no stored value).
 * @tparam Errs List of possible error types (must inherit from ResultError).
 */
template<traits::IsResultError... Errs>
    requires(sizeof...(Errs) > 0)
class Result<void, Errs...> {
  public:
    /**
     * @brief Construct a Result with an error.
     * @tparam E Error type (must be in Errs).
     * @param error Error to store.
     */
    template<traits::IsResultError E>
        requires traits::is_in_pack_v<E, Errs...>
    constexpr Result(E&& error)
        : m_error(std::forward<E>(error)) {}

    /**
     * @brief Construct a Result with no error (success state).
     */
    constexpr Result()
        : m_error(std::monostate()) {}

    /**
     * @brief Get the error of the given type, wrapped in std::optional
     *
     * @tparam Self the deduced self type
     * @tparam E the error type to get
     * @param self the self object
     * @return the error type, wrapped in std::optional
     */
    template<typename Self, traits::IsResultError E>
        requires traits::is_in_pack_v<E, Errs...>
    constexpr auto&& get_error(this Self&& self) {
        if (std::holds_alternative<E>(self.m_error)) {
            return std::optional(std::holds_alternative<E>(std::forward<Self>(self).m_error));
        } else {
            return std::optional<E>();
        }
    }

    /**
     * @brief whether there is an error value
     *
     * @return true
     * @return false
     */
    constexpr bool has_error() {
        return !std::holds_alternative<std::monostate>(m_error);
    }

  private:
    // instead of wrapping the variant in std::optional, we can use std::monostate
    std::variant<std::monostate, Errs...> m_error; ///< Variant holding an error or monostate.
};

} // namespace zest