#pragma once

#include <concepts>
#include <functional>
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
    using ErrorT = E;
    using ValueT = T;

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
    template<typename F>
    constexpr Result(F&& error)
        requires std::default_initializable<T>
        : error(std::forward<F>(error)) {}

    /**
     * @brief Construct with an "error" value and "normal" value
     *
     * @tparam U value argument type
     *
     * @param error argument to initialize the "error" value with
     * @param value argument to initialize the "normal" value with
     *
     * Constraints:
     * - T must be constructible given perfectly forwarded value argument
     * - U must be constructible given perfectly forwarded error argument
     */
    template<typename F, typename U>
        requires std::constructible_from<T, U&&> && std::constructible_from<E, F&&>
    constexpr Result(F&& error, U&& value)
        : error(std::forward<F>(error)),
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
     * @tparam Self deduced self type
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
    constexpr bool has_error() const {
        return error.has_value();
    }

    /**
     * @brief Get the error
     *
     * @tparam Self deduced self type
     *
     * @param self
     *
     * @return std::optional<E>
     */
    template<typename Self>
    constexpr auto get_error(this Self&& self) {
        return std::forward<Self>(self).error;
    }

    /**
     * @brief and_then monadic function. Invokes the callable with the normal value if no error is
     * contained and returns the output. Otherwise, returns the error.
     *
     * @tparam F callable type (deduced)
     * @tparam Self self type (deduced)
     *
     * @param self this Result instance
     * @param f the callable
     *
     * @return return type of callable
     *
     * Constraints:
     * - callable must be invokable with the perfectly forwarded normal value
     * - callable return type must be default-constructible
     * - callable must return a Result instance with the same error value type
     *
     * TODO: add support for non-default-constructible types
     */
    template<typename F, typename Self>
    constexpr auto and_then(this Self&& self, F&& f) {
        using R = std::invoke_result_t<F, decltype((std::forward<Self>(self).get_value()))>;
        static_assert(
            std::default_initializable<R>,
            "callable return type must be default-constructible"
        );

        if (self.has_error()) {
            return R(std::forward<Self>(self).error.get_value());
        }
        return std::invoke(std::forward<F>(f), std::forward<Self>(self).value);
    }

    /**
     * @brief or_else monadic function. Invokes the callable with the error value if present and
     * returns the output. Otherwise, returns the normal value.
     *
     * @tparam F callable type (deduced)
     * @tparam Self self type (deduced)
     *
     * @param self this Result instance
     * @param f the callable
     *
     * @return return type of the callable
     *
     * Constraints:
     * - callable must be invokable with the perfectly forwarded error value
     * - callable must return Result instance with the same normal value type
     */
    template<typename F, typename Self>
    constexpr auto or_else(this Self&& self, F&& f) {
        if (self.has_error()) {
            return std::invoke(std::forward<F>(f), std::forward<Self>(self).error.get_value());
        }
        return std::forward<Self>(self);
    }

    /**
     * @brief transform monadic function. Invokes the callable with the normal value if no error
     * value is present, and returns the output wrapped in a Result. Otherwise, returns the
     * default value of the new value type and the error type.
     *
     * @tparam F callable type (deduced)
     * @tparam Self self type (deduced)
     *
     * @param self this Result instance
     * @param f the callable
     *
     * @return return type of the callable, wrapped in a Result
     *
     * Constraints:
     * - callable must be invokable with the perfectly forwarded normal value
     * - callable return type must be default-constructible
     *
     * TODO: add support for non-default-constructible types.
     */
    template<typename F, typename Self>
    constexpr auto transform(this Self&& self, F&& f) {
        using R = std::invoke_result_t<F, decltype((std::forward<Self>(self).get_value()))>;

        if (self.has_error()) {
            return Result<R, E>(std::forward<Self>(self).error.get_value());
        }
        return Result<R, E>(std::invoke(std::forward<F>(f), std::forward<Self>(self).value));
    }

    /**
     * @brief transform_error monadic function. Invokes the callable with the error value if
     * present, and returns the output wrapped in a Result. Otherwise, returns the normal value and
     * no error value.
     *
     * @tparam F callable type (deduced)
     * @tparam Self self type (deduced)
     *
     * @param self this Result instance
     * @param f the callable
     *
     * @return return type of the callable, wrapped in a Result
     *
     * Constraints:
     * - callable must be invokable with the perfectly forwarded error value
     */
    template<typename F, typename Self>
    constexpr auto transform_error(this Self&& self, F&& f) {
        using R = std::invoke_result_t<F, decltype((std::forward<Self>(self).get_error()))>;

        if (self.has_error()) {
            return Result<T, R>(
                std::invoke(std::forward<F>(f), std::forward<Self>(self).get_error().get_value())
            );
        }
        return Result<T, R>(std::forward<Self>(self).get_value());
    }

    /**
     * @brief return the current value if no error is contained, otherwise return alternate value
     *
     * @tparam U other value type (deduced)
     * @tparam Self self type (deduced)
     *
     * @param self this Result instance
     * @param other_value the alternate value
     *
     * @return T
     */
    template<typename U, typename Self>
        requires std::convertible_to<U&&, T>
    constexpr T value_or(this Self&& self, U&& other_value) {
        if (self.has_error()) {
            return std::forward<U>(other_value);
        }
        return std::forward<Self>(self).get_value();
    }

    /**
     * @brief return the current error if contained, otherwise return alternate error
     *
     * @tparam F other error type (deduced)
     * @tparam Self self type (deduced)
     *
     * @param self this Result instance
     * @param other_error the alternate error
     *
     * @return E
     */
    template<typename F, typename Self>
        requires std::convertible_to<F&&, E>
    constexpr E error_or(this Self&& self, F&& other_error) {
        if (self.has_error()) {
            return std::forward<Self>(self).error.get_value();
        }
        return std::forward<F>(other_error);
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
     *
     * Constraints:
     * E must be constructible given perfectly forwarded error argument
     */
    template<typename F>
        requires std::constructible_from<E, F&&>
    constexpr Result(F&& error)
        : error(error) {}

    /**
     * @brief Whether an error is contained
     *
     * @return true an error is contained
     * @return false an error is not contained
     */
    constexpr bool has_error() const {
        return error.has_value();
    }

    /**
     * @brief Get the error, wrapped in std::optional
     *
     * @tparam Self deduced self type
     *
     * @param self
     *
     * @return std::optional<E>
     */
    template<typename Self>
    constexpr auto get_error(this Self&& self) {
        return std::forward<Self>(self).error;
    }

    /**
     * @brief and_then monadic function. Invokes the callable if no error is
     * contained and returns the output. Otherwise, returns the error.
     *
     * @tparam F callable type (deduced)
     * @tparam Self self type (deduced)
     *
     * @param self this Result instance
     * @param f the callable
     *
     * @return return type of callable
     *
     * Constraints:
     * - callable must be invokable
     * - callable return type must be default-constructible
     * - callable must return a Result instance with the same error value type
     *
     * TODO: add support for non-default-constructible types
     * TODO: check void specialization has valid documentation
     */
    template<std::invocable F, typename Self>
    constexpr auto and_then(this Self&& self, F&& f) {
        using R = std::invoke_result_t<F>;
        if (self.has_error()) {
            return R(std::forward<Self>(self).error.value());
        }
        return std::invoke(std::forward<F>(f));
    }

    /**
     * @brief or_else monadic function. Invokes the callable with the error value if present and
     * returns the output. Otherwise, returns the normal value.
     *
     * @tparam F callable type (deduced)
     * @tparam Self self type (deduced)
     *
     * @param self this Result instance
     * @param f the callable
     *
     * @return return type of the callable
     *
     * Constraints:
     * - callable must be invokable with the perfectly forwarded error value
     * - callable must return Result instance with the same normal value type
     *
     * TODO: check void specialization has valid documentation
     */
    template<typename F, typename Self>
    constexpr auto or_else(this Self&& self, F&& f) {
        if (self.has_error()) {
            return std::invoke(std::forward<F>(f), std::forward<Self>(self).error.value());
        }
        return std::forward<Self>(self).get_value();
    }

    /**
     * @brief transform monadic function. Invokes the callable with the normal value if no error
     * value is present, and returns the output wrapped in a Result. Otherwise, returns the
     * default value of the new value type and the error type.
     *
     * @tparam F callable type (deduced)
     * @tparam Self self type (deduced)
     *
     * @param self this Result instance
     * @param f the callable
     *
     * @return return type of the callable, wrapped in a Result
     *
     * Constraints:
     * - callable must be invokable with the perfectly forwarded normal value
     * - callable return type must be default-constructible
     *
     * TODO: add support for non-default-constructible types.
     */
    template<typename F, typename Self>
    constexpr auto transform(this Self&& self, F&& f) {
        using R = std::invoke_result_t<F>;
        if (self.has_error()) {
            return R(std::forward<Self>(self).error.value());
        }
        return R{std::invoke(std::forward<F>(f))};
    }

    /**
     * @brief transform_error monadic function. Invokes the callable with the error value if
     * present, and returns the output wrapped in a Result. Otherwise, returns the normal value and
     * no error value.
     *
     * @tparam F callable type (deduced)
     * @tparam Self self type (deduced)
     *
     * @param self this Result instance
     * @param f the callable
     *
     * @return return type of the callable, wrapped in a Result
     *
     * Constraints:
     * - callable must be invokable with the perfectly forwarded error value
     */
    template<typename F, typename Self>
    constexpr auto transform_error(this Self&& self, F&& f) {
        using R = std::invoke_result_t<F, decltype((std::forward<Self>(self).get_error()))>;

        if (self.has_error()) {
            return Result<T, R>(
                std::invoke(std::forward<F>(f), std::forward<Self>(self).get_error().get_value())
            );
        }
        return Result<T, R>(std::forward<Self>(self).get_value());
    }

    /**
     * @brief return the current error if contained, otherwise return alternate error
     *
     * @tparam F other error type (deduced)
     * @tparam Self self type (deduced)
     *
     * @param self this Result instance
     * @param other_error the alternate error
     *
     * @return E
     */
    template<typename F, typename Self>
        requires std::convertible_to<F&&, E>
    constexpr E error_or(this Self&& self, F&& other_error) {
        if (self.has_error()) {
            return std::forward<Self>(self).error.get_value();
        }
        return std::forward<F>(other_error);
    }
};

} // namespace zest