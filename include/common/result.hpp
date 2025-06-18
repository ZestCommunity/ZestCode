#pragma once

#include <cerrno>
#include <chrono>
#include <concepts>
#include <functional>
#include <limits>
#include <optional>
#include <stacktrace>
#include <type_traits>
#include <variant>

namespace zest {

/**
 * @brief lambda argument type helper
 *
 * Yoinked from c++ stories https://www.cppstories.com/2019/02/2lines3featuresoverload.html/
 *
 * @b Example
 * @code
 * // output: hello
 * std::variant<int, float, std::string> int_float_string("Hello");
 * std::visit(overload {
 *         [](const int& i) { std::println("int: {}", i); },
 *         [](const float& f) { std::println("float: {}", f); },
 *         [](const std::string& s) { std::println("string: {}", s); }
 *     },
 *     int_float_string
 * );
 * @endcode
 *
 * @tparam Ts
 */
template<class... Ts>
struct overload : Ts... {
    using Ts::operator()...;

    consteval void operator()(auto) const {
        static_assert(false, "Unsupported type");
    }
};

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

    // the runtime data is only contained if ResultError was constructed at runtime
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
concept InPack = (std::is_same_v<std::remove_cvref_t<T>, Ts> || ...);

/**
 * @brief a simple typename pack wrapper
 *
 * @tparam Ts the typename pack to wrap
 */
template<typename... Ts>
struct type_pack {};

/**
 * @brief Check if PackA contains all the types in PackB
 *
 * @tparam PackA
 * @tparam PackB
 */
template<typename PackA, typename PackB>
struct contains_all;

/**
 * @brief Check if PackA contains all the types in PackB
 *
 * @tparam PackA
 * @tparam PackB
 */
template<typename... As, typename... Bs>
struct contains_all<type_pack<As...>, type_pack<Bs...>> {
    static constexpr bool value = (InPack<Bs, As...> && ...);
};

/**
 * @brief Check if PackA contains all the types in PackB
 *
 * @tparam PackA
 * @tparam PackB
 */
template<typename PackA, typename PackB>
concept ContainsAll = contains_all<PackA, PackB>::value;

/**
 * @brief Check if a given type is derived from ResultError
 */
template<typename T>
concept IsResultError = std::is_base_of_v<ResultError, std::remove_cvref_t<T>>;

/**
 * @brief Trait to define a "sentinel" value for types indicating an error state.
 *
 * @note Specialize this template for custom types if needed.
 *
 * @tparam T Type to provide a sentinel value for.
 */
template<typename T>
class SentinelValue;

/**
 * @brief Partial specialization of SentinelValue for integral and floating-point types.
 *
 * @details Uses infinity for floating-point types if available; otherwise uses max value.
 *
 * @tparam T Integral or floating-point type.
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
class Result;

namespace traits {

/**
 * @brief Check if a type is a Result
 *
 * @tparam T Type to check.
 */
template<typename>
struct is_result : std::false_type {};

// is_result specialization. Used to infer Result.
template<typename T, typename... Errs>
struct is_result<Result<T, Errs...>> : std::true_type {};

/**
 * @brief Check if a type is a Result
 *
 * @tparam T Type to check
 */
template<typename T>
concept IsResult = is_result<T>::value;

/**
 * @brief Used to show that a type should be ignored
 *
 */
struct ignored_type {};

/**
 * @brief Check whether all types in a pack are the same
 *
 * @note any ignored_type will not be considered
 *
 * @tparam Ts types to check
 */
template<typename... Ts>
inline constexpr bool all_same_v = true;

// this specialization uses recursion to check all the types.
// This is the only known way to do this in C++23 (because of the extra logic needed for
// ignored_type)
// TODO: use C++26 pack indexing once it's supported
template<typename T, typename... Ts>
inline constexpr bool all_same_v<T, Ts...> =
    ((std::same_as<T, Ts> || std::same_as<ignored_type, Ts>) && ...);

/**
 * @brief Check whether all types in a pack are the same
 *
 * @note any ignored_type will not be considered
 *
 * @tparam Ts types to check
 */
template<typename... Ts>
concept AllSame = all_same_v<Ts...>;

/**
 * @brief Find the first type in a typename pack that satisfies the given condition
 *
 * @tparam Condition the condition to check
 * @tparam Ts the types to apply to the condition
 */
template<typename Condition, typename... Ts>
struct first_type_that_satisfies {};

// This specialization uses recursion to find the first type.
// TODO: use C++26 pack indexing once it's supported
template<typename Condition, typename T, typename... Ts>
struct first_type_that_satisfies<Condition, T, Ts...> :
    std::conditional<
        Condition::template value<T>,               // Check condition for T
        std::type_identity<T>,                      // If true, use T
        first_type_that_satisfies<Condition, Ts...> // Otherwise, check rest
        > {};

/**
 * @brief Find the first type in a typename pack that satisfies the given condition
 *
 * @tparam Condition the condition to check
 * @tparam Ts the types to apply to the condition
 */
template<typename Condition, typename... Ts>
using first_type_that_satisfies_t = typename first_type_that_satisfies<Condition, Ts...>::type;

/**
 * @brief get the return type of a callable invoked with the given arguments
 *
 * @note if the callable is not invocable, the type is ignored_type
 *
 * @tparam F the callable to invoke
 * @tparam Args callable argument
 */
template<typename F, typename... Args>
struct invoke_result : std::type_identity<ignored_type> {};

// specialization, used when the callable is invocable
template<typename F, typename... Args>
    requires std::invocable<F, Args...>
struct invoke_result<F, Args...> : std::type_identity<std::invoke_result_t<F, Args...>> {};

/**
 * @brief get the return type of a callable invoked with the given arguments
 *
 * @note if the callable is not invocable, the type is ignored_type
 *
 * @tparam F the callable to invoke
 * @tparam Args callable argument
 */
template<typename F, typename... Args>
using invoke_result_t = invoke_result<F, Args...>::type;

/**
 * @brief get the return type of Result::and_then given the Result type and callable type
 *
 * @tparam R the Result
 * @tparam F the callable
 */
template<typename R, std::invocable F>
using and_then_return_t = std::invoke_result_t<F, decltype((std::declval<R>().value))>;

/**
 * @brief get the return type of the callable passed to Result::or_else
 *
 * @note if the callable is not invocable, the type is ignored_type
 *
 * @tparam R the Result
 * @tparam F the callable
 * @tparam E the callable argument
 */
template<typename R, typename F, typename E>
using or_else_return_t = invoke_result_t<F, decltype((std::get<E>(std::declval<R>().error)))>;

/**
 * @brief check whether a callable is invocable given the Result and argument type
 *
 * @tparam R the Result
 * @tparam F the callable
 * @tparam E the callable argument
 */
template<typename R, typename F, typename E>
concept Invocable = std::invocable<F, decltype((std::get<E>(std::declval<R>().error)))>;

/**
 * @brief check whether a callable is invocable given the Result and argument type
 *
 * @note the value is wrapped in a struct in order to separate the template arguments. This is done
 * so this can be used as the condition when using first_type_that_satisfies_t
 *
 * @tparam F the callable
 * @tparam R the Result
 */
template<typename F, typename R>
struct invocable_indirect_v {
    /**
     * @brief whether the callable is invocable
     *
     * @tparam U the callable argument
     */
    template<typename U>
    static constexpr bool value = Invocable<R, F, U>;
};

/**
 * @brief check whether a type is a Result or void.
 *
 * @note helper for Result::or_else
 * @note concept is satisfied with ignored_type as well
 *
 * @tparam T the type to check
 */
template<typename T>
concept ResultOrVoid =
    traits::IsResult<std::remove_cvref_t<T>> || std::same_as<void, std::remove_cvref_t<T>>
    || std::same_as<ignored_type, std::remove_cvref_t<T>>;

/**
 * @brief check whether the value type of a Result is the other given type
 *
 * @note helper for Result::or_else
 * @note concept if satisfied if T is void or ignored_type
 *
 * @tparam R the Result
 * @tparam T the other type
 */
template<typename R, typename S>
concept ValueTypeMatch = std::same_as<void, S> || std::same_as<ignored_type, S>
                         || std::same_as<typename S::value_type, typename S::value_type>;

} // namespace traits

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
                if constexpr (std::is_same_v<std::monostate, std::remove_cvref_t<decltype(arg)>>) {
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
     * constraints:
     * - Callable must be invocable with at least one error type as an argument
     * - If the callable is invocable, it must always return the same type given different parameter
     * types
     * - The callable is only allowed to return a Result, or void
     * - If a Result is returned, it must have the same value type as this Result instance
     * - The return type of the callable must be the same for all possible arguments it may be
     * called with.
     *
     * @tparam Self deduced self type
     * @tparam F the type of the callable
     * @param self the current Result instance
     * @param f the callable
     */
    template<typename Self, typename F>
        requires(traits::Invocable<Self, F, Errs> || ...)
                // the callable must always return the same type (if it's invocable)
                && traits::AllSame<traits::or_else_return_t<Self, F, Errs>...>
                // the callable must return a Result or void (if it's invocable)
                && (traits::ResultOrVoid<traits::or_else_return_t<Self, F, Errs>> && ...)
                // if the callable returns a Result, it must have the same value type as this
                // Result instance
                && (traits::ValueTypeMatch<Self, traits::or_else_return_t<Self, F, Errs>> && ...)
    constexpr auto or_else(this Self&& self, F&& f) {
        // the return type is the same as the return type of the callable, unless the callable
        // returns void. In that case, the return type is Self with cv-refs removed
        using CallableReturnType =
            traits::first_type_that_satisfies_t<traits::invocable_indirect_v<F, Self>, Errs...>;
        using ReturnType = std::conditional_t<
            traits::IsResult<CallableReturnType>,
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
                if constexpr (!traits::Invocable<Self, F, decltype((arg))>
                              || std::
                                  is_same_v<std::monostate, std::remove_cvref_t<decltype(arg)>>) {
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
            if constexpr (!traits::Invocable<Self, F, decltype((arg))>
                          || std::is_same_v<std::monostate, std::remove_cvref_t<decltype(arg)>>) {
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
  private:
    // or_else return type helper
    template<typename Self, typename F, typename E>
    using or_else_return_t =
        std::invoke_result_t<F, decltype((std::get<E>(std::declval<Self>().error)))>;

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
     * The callable must be able to take a perfectly forwarded error that may be contained by this
     * Result instance. The callable must be able to return a Result containing any error type that
     * may be passed to it. The normal value type of the Result returned by the callable must be the
     * same as the normal value type of this Result instance.
     *
     * @tparam Self deduced self type
     * @tparam F the type of the callable
     * @param self the current Result instance
     * @param f the callable
     */
    template<typename Self, typename F>
        requires(std::invocable<F, decltype((std::get<Errs>(std::declval<Self>().error)))> && ...)
                && (traits::IsResult<or_else_return_t<Self, F, Errs>> && ...)
                && (traits::AllSame<or_else_return_t<Self, F, Errs>...>)
                && (std::same_as<void, typename or_else_return_t<Self, F, Errs>::value_type> && ...)
    constexpr auto or_else(this Self&& self, F&& f) {
        using ReturnType = or_else_return_t<Self, F, std::tuple_element_t<0, std::tuple<Errs...>>>;
        // if there isn't an error, return
        return ReturnType();
        // otherwise, invoke the given lambda
        return std::visit([&f](auto&& arg) -> ReturnType {
            // even though this conditional is impossible, it's necessary to stop the compiler from
            // thinking that ReturnType could be constructed with std::monostate
            if constexpr (std::is_same_v<std::monostate, std::remove_cvref_t<decltype(arg)>>) {
                throw std::logic_error("This exception is unreachable");
            } else {
                return std::invoke(f, arg);
            }
        }, std::forward<Self>(self).error);
    }
};
} // namespace zest