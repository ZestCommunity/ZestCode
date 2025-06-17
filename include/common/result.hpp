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

// A simple type pack wrapper
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
    static constexpr bool value = (is_in_pack_v<Bs, As...> && ...);
};

/**
 * @brief Check if PackA contains all the types in PackB
 *
 * @tparam PackA
 * @tparam PackB
 */
template<typename PackA, typename PackB>
inline constexpr bool contains_all_v = contains_all<PackA, PackB>::value;

/**
 * @brief Check if PackA contains all the types in PackB
 *
 * @tparam PackA
 * @tparam PackB
 */
template<typename PackA, typename PackB>
concept ContainsAll = contains_all_v<PackA, PackB>;

/**
 * @brief type trait for checking whether a type is derived from the ResultError class
 */
template<typename>
struct is_result_error : std::false_type {};

/**
 * @brief type trait for checking whether a type is derived from the ResultError class
 */
template<typename T>
    requires std::is_base_of_v<ResultError, T>
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
concept IsResultError = is_result_error_v<T>;

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
    requires(traits::HasSentinel<T> || std::same_as<void, T>) && (sizeof...(Errs) > 0)
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
 * @tparam T Type to check
 */
template<typename T>
concept IsResult = is_result_v<T>;

/**
 * @brief Check whether all types in a pack are the same
 *
 * @tparam Ts types to check
 */
template<typename... Ts>
struct all_same : std::true_type {};

// special type that is ignored by the all_same trait
struct all_same_ignored_type {};

/**
 * @brief Check whether all types in a pack are the same
 *
 * @tparam T first type
 * @tparam Ts the rest of the types
 */
template<typename T, typename... Ts>
struct all_same<T, Ts...> :
    std::bool_constant<
        ((std::is_same_v<T, Ts> || std::is_same_v<all_same_ignored_type, Ts>) && ...)> {};

/**
 * @brief Check whether all types in a pack are the same
 *
 * @tparam Ts types to check
 */
template<typename... Ts>
inline constexpr bool all_same_v = all_same<Ts...>::value;

// Primary template: no types, no result (causes substitution failure if used)
template<typename Condition, typename... Ts>
struct first_type_that_satisfies {};

// Specialization: at least one type in the pack
template<typename Condition, typename T, typename... Ts>
struct first_type_that_satisfies<Condition, T, Ts...> :
    std::conditional_t<
        Condition::template value<T>,               // Check condition for T
        std::type_identity<T>,                      // If true, use T
        first_type_that_satisfies<Condition, Ts...> // Otherwise, check rest
        > {};

// Helper alias to access the result type
template<typename Condition, typename... Ts>
using first_type_that_satisfies_t = typename first_type_that_satisfies<Condition, Ts...>::type;

} // namespace traits

/**
 * @brief Result class for expected value or error handling (similar to std::expected).
 * @tparam T Type of the expected value.
 * @tparam Errs List of possible error types (must inherit from ResultError).
 * @note Errors are stored in a variant, and the value is always initialized.
 */
template<typename T, traits::IsResultError... Errs>
    requires(traits::HasSentinel<T> || std::same_as<void, T>) && (sizeof...(Errs) > 0)
class Result {
  private:
    // and_then return type helper
    template<typename Self, typename F>
    using and_then_return_t = std::invoke_result_t<F, decltype((std::declval<Self>().value))>;

    // or_else return type helper
    template<typename Self, typename F, typename E>
    using or_else_return_t =
        std::invoke_result_t<F, decltype((std::get<E>(std::declval<Self>().error)))>;

    // or_else invocable callable helper
    template<typename Self, typename F, typename E>
    constexpr static bool or_else_invocable_v =
        std::is_invocable<F, decltype((std::get<E>(std::declval<Self>().error)))>::value;

    template<typename F, typename Self>
    struct or_else_invocable_indirect_v {
        template<typename U>
        static constexpr bool value = or_else_invocable_v<Self, F, U>;
    };

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
        requires traits::is_in_pack_v<E, Errs...>
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
        requires traits::is_in_pack_v<E, Errs...>
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
                 && traits::is_result_v<and_then_return_t<Self, F>>
                 && traits::
                     contains_all_v<typename and_then_return_t<Self, F>::error_types, error_types>
    constexpr auto and_then(this Self&& self, F&& f) {
        // if there is an error, return said error immediately
        if (self.has_error()) {
            return std::visit([](auto&& arg) -> and_then_return_t<Self, F> {
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
        requires( // callable must be invocable with at least 1 error type
                    or_else_invocable_v<Self, F, Errs> || ...
                )
                // the callable must always return the same type if it is invocable
                && traits::all_same_v<std::conditional_t<
                    or_else_invocable_v<Self, F, Errs>,
                    // if the callable is invocable
                    or_else_return_t<Self, F, Errs>,
                    // if the callable is not invocable
                    traits::all_same_ignored_type>...>
                // the callable must return a Result or void if it is invocable with a given error
                // type
                && (std::conditional_t<
                        or_else_invocable_v<Self, F, Errs>,
                        // if the callable is invocable
                        std::disjunction<
                            traits::is_result<or_else_return_t<Self, F, Errs>>,
                            std::is_same<or_else_return_t<Self, F, Errs>, void>>,
                        // otherwise, if the callable is not invocable
                        std::true_type>::value
                    && ...)
                // if F is invocable and returns a result, it must have the same value type as Self
                && (std::conditional_t<
                        or_else_invocable_v<Self, F, Errs>,
                        // if the callable is invocable
                        std::conditional_t<
                            traits::is_result_v<or_else_return_t<Self, F, Errs>>,
                            // if the return is a Result
                            std::is_same<T, typename or_else_return_t<Self, F, Errs>::value_type>,
                            // otherwise, if the return is not a Result
                            std::true_type>,
                        // otherwise, if the callable is not invocable
                        std::true_type>::value
                    && ...)
    constexpr auto or_else(this Self&& self, F&& f) {
        // the return type is the same as the return type of the callable, unless the callable
        // returns void. In that case, the return type is Self with cv-refs removed
        using CallableReturnType =
            traits::first_type_that_satisfies_t<or_else_invocable_indirect_v<F, Self>, Errs...>;
        using ReturnType = std::conditional_t<
            traits::is_result_v<CallableReturnType>,
            CallableReturnType,
            std::remove_cvref_t<Self>>;
        // if there isn't an error, return the value
        if (!self.has_error()) {
            return std::forward<Self>(self).value;
        }
        // if the callable returns void, then return this Result instance after invoking the
        // callable
        return std::visit([&f](auto&& arg) -> ReturnType {
            // even though this condition is impossible, it's necessary. Otherwise the
            // compiler will compile a branch where f is invoked with an unsupported argument
            // type
            if constexpr (!or_else_invocable_v<Self, F, decltype((arg))>) {
                throw std::logic_error("This exception is unreachable");
            } else if constexpr (std::same_as<ReturnType, void>) {
                std::invoke(f, std::forward<decltype(arg)>(arg));
                return std::forward<decltype(arg)>(arg);
            } else {
                return std::invoke(f, std::forward<decltype(arg)>(arg));
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
        requires traits::is_in_pack_v<E, Errs...>
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
        requires traits::is_in_pack_v<E, Errs...>
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
        requires traits::is_result_v<std::invoke_result_t<F>>
                 && traits::
                     contains_all_v<typename std::invoke_result_t<F>::error_types, error_types>
    constexpr auto and_then(this Self&& self, F&& f) {
        // if there is an error, return said error immediately
        if (self.has_error()) {
            return std::visit([](auto&& arg) -> std::invoke_result_t<F> {
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
                && (traits::is_result_v<or_else_return_t<Self, F, Errs>> && ...)
                && (traits::all_same_v<or_else_return_t<Self, F, Errs>...>)
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