#pragma once

#include <concepts>
#include <optional>
#include <utility>

namespace zest {

/**
 * @brief Error handling utility
 *
 * @tparam T "normal" type
 * @tparam E "error" type
 */
template<typename T, typename E>
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
     * @brief Get the error
     *
     * @tparam Self
     * @param self
     * @return constexpr auto
     */
    template<typename Self>
    constexpr auto get_error(this Self&& self) {
        return std::forward<Self>(self).error;
    }

    // prevent ambiguous operator overload resolution
    bool operator==(const Result& other) = delete;
};

/**
 * @brief void specialization of Result
 *
 * @tparam E "error" type
 */
template<typename E>
class Result<void, E> {
    std::optional<E> error;

    /**
     * @brief default constructor
     */
    constexpr Result() = default;

    /**
     * @brief Construct with an "error" value, and the default "normal" value
     *
     * @param error argument to initialize the "error" value with
     */
    constexpr Result(E error)
        : error(error) {}

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
     * @brief Get the error
     *
     * @tparam Self
     * @param self
     * @return constexpr auto
     */
    template<typename Self>
    constexpr auto get_error(this Self&& self) {
        return std::forward<Self>(self).error;
    }
};

} // namespace zest