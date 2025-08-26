#pragma once

#include <concepts>
#include <optional>
#include <type_traits>
#include <utility>

namespace zest {

/**
 * @brief Helper class for error handling.
 *
 * @tparam T "normal" type
 * @tparam E "error" type
 *
 * Constraints:
 * - E must be a scoped enum
 * - E must not be implicitly convertible to T
 * - E must not be equality comparable with T
 */
template<typename T, typename E>
    requires std::is_scoped_enum_v<E> && (!std::convertible_to<E, T>)
             && (!std::equality_comparable_with<T, E>)
class Result {
  public:
    std::optional<E> error;
    T value;

    /**
     * @brief default constructor
     *
     * Constraints:
     * - T must have a default constructor
     */
    constexpr Result()
        requires std::default_initializable<T>
    = default;

    /**
     * @brief Construct with a "normal" value, and no error value
     *
     * @tparam U argument type
     *
     * @param value argument to initialize the "normal" value with
     *
     * Constraints:
     * - T must be constructible with perfectly forwarded value argument
     */
    template<typename U>
        requires std::constructible_from<T, U&&>
    constexpr Result(U&& value)
        : value(std::forward<U>(value)) {}

    /**
     * @brief Construct with an "error" value, and the default "normal" value
     *
     * @param error argument to initialize the "error" value with
     *
     * Constraints:
     * - T must be default initializable
     */
    constexpr Result(E error)
        requires std::default_initializable<T>
        : error(error) {}

    /**
     * @brief Construct with an "error" value and "normal" value
     *
     * @tparam U value argument type
     *
     * @param error argument to initialize the "error" value with
     * @param value argument to initialize the "normal" value with
     *
     * Constraints:
     * - T must be constructible with perfectly forwarded value argument
     */
    template<typename U>
        requires std::constructible_from<T, U&&>
    constexpr Result(E error, U&& value)
        : error(error),
          value(std::forward<U>(value)) {}

    /**
     * @brief conversion operator for an l-value reference to the "normal" value type
     *
     * @return T&
     */
    constexpr operator T&() & {
        return value;
    }

    /**
     * @brief conversion operator for a const l-value reference to the "normal" value type
     *
     * @return const T&
     */
    constexpr operator const T&() const& {
        return value;
    }

    /**
     * @brief conversion operator for an r-value reference to the "normal" value type
     *
     * @return T&&
     */
    constexpr operator T&&() && {
        return std::move(value);
    }

    /**
     * @brief Get the value object
     *
     * @tparam Self
     *
     * @param self
     *
     * @return "normal" value
     */
    template<typename Self>
    constexpr auto get_value(this Self&& self) {
        return std::forward<Self>(self).value;
    }

    /**
     * @brief Whether an error is contained
     *
     * @return true an error is contained
     * @return false an error is not contained
     */
    constexpr bool has_error() {
        return error.has_value();
    }

    /**
     * @brief Whether a specific error is contained
     *
     * @param error the error to check
     *
     * @return true error is contained
     * @return false error is not contained
     */
    constexpr bool has_error(E error) {
        return this->error == error;
    }
};

} // namespace zest

/**
 * @brief Compare the equality of 2 Result instances
 *
 * @note this function is defined in order to prevent ambiguous overload resolution.
 *
 * @tparam LhsT "normal" type of left-hand side object
 * @tparam RhsT "normal" type of right-hand side object
 * @tparam LhsE "error" type of left-hand side object
 * @tparam RhsE "error" type of right-hand side object
 *
 * @param lhs left-hand side object
 * @param rhs right-hand side object
 *
 * @return true "normal" values are equal.
 * @return false "normal" values are not equal.
 */
template<typename LhsT, typename RhsT, typename LhsE, typename RhsE>
    requires std::equality_comparable_with<LhsT, RhsT>
constexpr bool
operator==(const zest::Result<LhsT, LhsE>& lhs, const zest::Result<RhsT, RhsE>& rhs) {
    return lhs.value == rhs.value;
}

/**
 * @brief Compare a Result instance with an instance of its error type. If the Result instance
 * contains the same error value, evaluates to true. If the Result instance does not contain the
 * same error value, evaluates to false.
 *
 * @tparam T "normal" type of the Result instance
 * @tparam E "error" type of the Result instance
 *
 * @param result Result instance to compare
 * @param error error instance to compare
 *
 * @return true instance contains the same error value.
 * @return false instance does not contain the same error value.
 */
template<typename T, typename E>
constexpr bool operator==(const zest::Result<T, E>& result, E error) {
    if (!result.error.has_value())
        return false;
    return result.error.value() == error;
}