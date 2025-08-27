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
        noexcept(std::is_nothrow_default_constructible_v<T>)
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
        // TODO: figure out correct noexcept specifier
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
        noexcept(std::is_nothrow_default_constructible_v<T>)
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
        // TODO: figure out correct noexcept specifier
        requires std::constructible_from<T, U&&>
    constexpr Result(E error, U&& value)
        : error(error),
          value(std::forward<U>(value)) {}

    /**
     * @brief conversion operator for an l-value reference to the "normal" value type
     *
     * @return T&
     */
    constexpr operator T&() & noexcept {
        return value;
    }

    /**
     * @brief conversion operator for a const l-value reference to the "normal" value type
     *
     * @return const T&
     */
    constexpr operator const T&() const& noexcept {
        return value;
    }

    /**
     * @brief conversion operator for an r-value reference to the "normal" value type
     *
     * @return T&&
     */
    constexpr operator T&&() && noexcept {
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
    // TODO: figure out correct noexcept specifier
    constexpr auto get_value(this Self&& self) {
        return std::forward<Self>(self).value;
    }

    /**
     * @brief Whether an error is contained
     *
     * @return true an error is contained
     * @return false an error is not contained
     */
    constexpr bool has_error() const noexcept {
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
    constexpr bool contains(E error) const noexcept(noexcept(std::declval<E>() == std::declval<E>())) {
        return this->error == error;
    }

    // prevent ambiguous operator overload resolution
    bool operator==(const Result& other) = delete;
};

} // namespace zest
