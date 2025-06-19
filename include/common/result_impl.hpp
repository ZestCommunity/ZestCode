#pragma once

#include "common/result.hpp"

#include <functional>

namespace zest {

/**
 * @brief Result class for expected value or error handling (similar to std::expected).
 *
 * @note the "normal" value is always valid, but an "error" value may or may not be contained.
 *
 * @tparam T Type of the expected value. Must have
 * @tparam Errs List of possible error types (must inherit from ResultError).
 */
template<typename T, traits::IsResultError... Errs>
    requires(traits::HasSentinel<T> || std::same_as<void, T>) && (sizeof...(Errs) > 0)
class Result {
  public:
    // instead of wrapping the variant in std::optional, we can use std::monostate
    std::variant<std::monostate, Errs...> error;
    T value;

    using value_type = T;
    using error_types = traits::type_pack<Errs...>;

    /**
     * @brief Construct a Result with a normal value (no error).
     * @tparam U Type convertible to T, and U not derived from ResultError
     * @param value Value to initialize the result with.
     */
    template<typename U>
        requires std::convertible_to<T, U>
    constexpr Result(U&& value)
        : error(std::monostate()),
          value(std::forward<U>(value)) {}

    /**
     * @brief Construct a Result with an error, initializing the value to its sentinel.
     * @tparam E Error type, must be in Errs and derived from ResultError.
     * @param error Error to store.
     * @note Requires T to have a defined sentinel value (via SentinelValue<T>).
     */
    template<traits::IsResultError E>
        requires traits::InPack<E, Errs...>
    constexpr Result(E&& error)
        : error(std::forward<E>(error)),
          value(traits::sentinel_v<T>) {}

    /**
     * @brief Get the error of the given type, wrapped in std::optional
     *
     * @tparam Self the deduced self type
     * @tparam E the error type to get
     * @param self the self object
     * @return the error type, wrapped in std::optional
     */
    template<traits::IsResultError E, typename Self>
        requires traits::InPack<E, Errs...>
    constexpr auto get_error(this Self&& self) {
        if (std::holds_alternative<E>(self.error)) {
            return std::optional<E>(std::get<E>(std::forward<Self>(self).error));
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
        return std::forward<Self>(self).value;
    }

    /**
     * @brief whether there is an error value
     *
     * @return true
     * @return false
     */
    constexpr bool has_error() {
        return !std::holds_alternative<std::monostate>(error);
    }

    /**
     * @brief and_then monadic function
     *
     * The callable must be able to take the perfectly-forwarded value of this Result instance as an
     * argument. The callable must be able to return any error passed to it. The callable must
     * return a Result.
     *
     * @tparam Self deduced self type
     * @tparam F the type of the callable
     * @param self the current Result instance
     * @param f the callable
     * @return return type of callable
     */
    template<typename Self, typename F>
        requires std::invocable<F, decltype((std::declval<Self>().value))>
                 && traits::IsResult<traits::and_then_return_t<Self, F>>
                 && traits::ContainsAll<
                     typename traits::and_then_return_t<Self, F>::error_types,
                     error_types>
    constexpr auto and_then(this Self&& self, F&& f) {
        // if there is an error, return said error immediately
        if (self.has_error()) {
            return std::visit([](auto&& arg) -> traits::and_then_return_t<Self, F> {
                // even though this conditional is impossible, it's necessary to stop the compiler
                // from thinking that ReturnType could be constructed with std::monostate
                if constexpr (std::same_as<std::monostate, std::remove_cvref_t<decltype(arg)>>) {
                    throw std::logic_error("This exception is unreachable");
                } else {
                    return std::forward<decltype(arg)>(arg);
                }
            }, std::forward<Self>(self).error);
        }
        // otherwise, invoke the callable and return the result
        return std::invoke(f, std::forward<Self>(self).value);
    }

    /**
     * @brief or_else monadic function.
     *
     * @note wrappers are used for the requires clause in order to avoid the use of fold
     * expressions, as fold expressions significantly complicates error messages
     *
     * constraints:
     * - Callable must be invocable with at least one error type as an argument
     * - If invocable, the callable must always return the same type regardless of argument type
     * - If invocable, the callable must return a Result
     * - The Result returned by the callable must have the same value type
     *
     * @tparam Self deduced self type
     * @tparam F the type of the callable
     * @param self the current Result instance
     * @param f the callable
     */
    template<typename Self, typename F>
        requires traits::AnyInvocable<Self, F, Errs...>
                 // the callable must always return the same type (if it's invocable)
                 && traits::AllSame<traits::or_else_return_t<Self, F, Errs>...>
                 // the callable must return a Result (if it's invocable)
                 && traits::AllResult<Self, F, Errs...>
                 // the Result returned by the callable must have the same value type
                 && traits::AllValueTypeMatch<Self, traits::or_else_return_t<Self, F, Errs>...>
    constexpr auto or_else(this Self&& self, F&& f) {
        using namespace traits;

        // the return type is the same as the return type of the callable, unless the callable
        // returns void. In that case, the return type is Self with cv-refs removed
        using CallableReturnType =
            first_type_that_satisfies_t<invocable_indirect_v<F, Self>, Errs...>;
        using ReturnType = std::conditional_t<
            IsResult<CallableReturnType>,
            CallableReturnType,
            std::remove_cvref_t<Self>>;

        // if there isn't an error, return the value
        if (!self.has_error()) {
            return ReturnType(std::forward<Self>(self).value);
        }

        // if the callable returns void
        if constexpr (std::same_as<void, CallableReturnType>) {
            return std::visit([&f](auto&& arg) -> ReturnType {
                // even though this condition is impossible, it's necessary. Otherwise the
                // compiler will compile a branch where f is invoked with an unsupported argument
                // type
                if constexpr (!Invocable<Self, F, decltype((arg))>
                              || std::same_as<std::monostate, std::remove_cvref_t<decltype(arg)>>) {
                    throw std::logic_error("This exception is unreachable");
                } else {
                    std::invoke(f, std::forward<decltype(arg)>(arg));
                    return std::forward<decltype(arg)>(arg);
                }
            }, std::forward<Self>(self).error);
        }

        // if the callable returns Result
        return std::visit([&f](auto&& arg) -> ReturnType {
            // even though this condition is impossible, it's necessary. Otherwise the
            // compiler will compile a branch where f is invoked with an unsupported argument
            // type
            if constexpr (!Invocable<Self, F, decltype((arg))>
                          || std::same_as<std::monostate, std::remove_cvref_t<decltype(arg)>>) {
                throw std::logic_error("This exception is unreachable");
            } else {
                std::invoke(f, std::forward<decltype(arg)>(arg));
                return std::forward<decltype(arg)>(arg);
            }
        }, std::forward<Self>(self).error);
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
}; // namespace zest

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
    using value_type = void;
    using error_types = traits::type_pack<Errs...>;

    // instead of wrapping the variant in std::optional, we can use std::monostate
    std::variant<std::monostate, Errs...> error; ///< Variant holding an error or monostate.

    /**
     * @brief Construct a Result with an error.
     * @tparam E Error type (must be in Errs).
     * @param error Error to store.
     */
    template<traits::IsResultError E>
        requires traits::InPack<E, Errs...>
    constexpr Result(E&& error)
        : error(std::forward<E>(error)) {}

    /**
     * @brief Construct a Result with no error (success state).
     */
    constexpr Result()
        : error(std::monostate()) {}

    /**
     * @brief Get the error of the given type, wrapped in std::optional
     *
     * @tparam Self the deduced self type
     * @tparam E the error type to get
     * @param self the self object
     * @return the error type, wrapped in std::optional
     */
    template<traits::IsResultError E, typename Self>
        requires traits::InPack<E, Errs...>
    constexpr auto get_error(this Self&& self) {
        if (std::holds_alternative<E>(self.error)) {
            return std::optional<E>(std::get<E>(std::forward<Self>(self).error));
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
        return !std::holds_alternative<std::monostate>(error);
    }

    /**
     * @brief and_then monadic function
     *
     * The callable must be able to take the perfectly-forwarded value of this Result instance as an
     * argument. The callable must be able to return any error passed to it. The callable must
     * return a Result.
     *
     * @tparam Self deduced self type
     * @tparam F the type of the callable
     * @param self the current Result instance
     * @param f the callable
     * @return return type of callable
     */
    template<std::invocable F, typename Self>
        requires traits::IsResult<std::invoke_result_t<F>>
                 && traits::ContainsAll<typename std::invoke_result_t<F>::error_types, error_types>
    constexpr auto and_then(this Self&& self, F&& f) {
        // if there is an error, return said error immediately
        if (self.has_error()) {
            return std::visit([](auto&& arg) -> std::invoke_result_t<F> {
                // even though this conditional is impossible, it's necessary to stop the compiler
                // from thinking that ReturnType could be constructed with std::monostate
                if constexpr (std::same_as<std::monostate, std::remove_cvref_t<decltype(arg)>>) {
                    throw std::logic_error("This exception is unreachable");
                } else {
                    return std::forward<decltype(arg)>(arg);
                }
            }, std::forward<Self>(self).error);
        }
        // otherwise, invoke the callable and return the result
        return std::invoke(f);
    }

    /**
     * @brief or_else monadic function.
     *
     * @note wrappers are used for the requires clause in order to avoid the use of fold
     * expressions, as fold expressions significantly complicates error messages
     *
     * constraints:
     * - Callable must be invocable with at least one error type as an argument
     * - If invocable, the callable must always return the same type regardless of argument type
     * - If invocable, the callable must return a Result
     * - The Result returned by the callable must have the same value type
     *
     * @tparam Self deduced self type
     * @tparam F the type of the callable
     * @param self the current Result instance
     * @param f the callable
     */
    template<typename Self, typename F>
        requires traits::AnyInvocable<Self, F, Errs...>
                 // the callable must always return the same type (if it's invocable)
                 && traits::AllSame<traits::or_else_return_t<Self, F, Errs>...>
                 // the callable must return a Result (if it's invocable)
                 && traits::AllResult<Self, F, Errs...>
                 // the Result returned by the callable must have the same value type
                 && traits::AllValueTypeMatch<Self, traits::or_else_return_t<Self, F, Errs>...>
    constexpr auto or_else(this Self&& self, F&& f) {
        using namespace traits;

        // the return type is the same as the return type of the callable, unless the callable
        // returns void. In that case, the return type is Self with cv-refs removed
        using CallableReturnType =
            first_type_that_satisfies_t<invocable_indirect_v<F, Self>, Errs...>;
        using ReturnType = std::conditional_t<
            IsResult<CallableReturnType>,
            CallableReturnType,
            std::remove_cvref_t<Self>>;

        // if there isn't an error, return
        if (!self.has_error()) {
            return ReturnType();
        }

        // if the callable returns void
        if constexpr (std::same_as<void, CallableReturnType>) {
            return std::visit([&f](auto&& arg) -> ReturnType {
                // even though this condition is impossible, it's necessary. Otherwise the
                // compiler will compile a branch where f is invoked with an unsupported argument
                // type
                if constexpr (!Invocable<Self, F, decltype((arg))>
                              || std::same_as<std::monostate, std::remove_cvref_t<decltype(arg)>>) {
                    throw std::logic_error("This exception is unreachable");
                } else {
                    std::invoke(f, std::forward<decltype(arg)>(arg));
                    return std::forward<decltype(arg)>(arg);
                }
            }, std::forward<Self>(self).error);
        }

        // if the callable returns Result
        return std::visit([&f](auto&& arg) -> ReturnType {
            // even though this condition is impossible, it's necessary. Otherwise the
            // compiler will compile a branch where f is invoked with an unsupported argument
            // type
            if constexpr (!Invocable<Self, F, decltype((arg))>
                          || std::same_as<std::monostate, std::remove_cvref_t<decltype(arg)>>) {
                throw std::logic_error("This exception is unreachable");
            } else {
                std::invoke(f, std::forward<decltype(arg)>(arg));
                return std::forward<decltype(arg)>(arg);
            }
        }, std::forward<Self>(self).error);
    }
};
} // namespace zest