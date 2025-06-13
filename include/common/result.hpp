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
    requires std::derived_from<T, ResultError>
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
    requires(traits::HasSentinel<T> || std::same_as<void, T>) && (sizeof...(Errs) > 0)
class Result {
  private:
    // helper type
    template<typename Self, typename F>
        requires std::invocable<F, decltype((std::declval<Self>().m_value))>
    using and_then_return_t = std::invoke_result_t<F, decltype((std::declval<Self>().m_value))>;

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
        requires std::convertible_to<T, U> && (!traits::IsResultError<U>)
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
    template<typename Self, traits::IsResultError E>
        requires traits::is_in_pack_v<E, Errs...>
    constexpr auto&& get_error(this Self&& self) {
        if (std::holds_alternative<E>(self.error)) {
            return std::optional(std::holds_alternative<E>(std::forward<Self>(self).error));
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
    constexpr auto and_then(this Self&& self, F&& f)
        requires std::invocable<F, decltype(std::forward<Self>(self).m_value)>
                 && traits::is_result_v<and_then_return_t<Self, F>>
                 && traits::contains_all_v<and_then_return_t<Self, F>, typename Self::error_types>
    {
        // if there is an error, return said error immediately
        if (self.has_error()) {
            return std::visit([](auto&& var) -> and_then_return_t<Self, F> {
                return std::forward<decltype(var)>(var);
            }, std::forward<Self>(self));
        }
        // otherwise, invoke the callable and return the result
        return std::invoke(f, std::forward<Self>(self).m_value);
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
    template<typename Self, traits::IsResultError E>
        requires traits::is_in_pack_v<E, Errs...>
    constexpr auto&& get_error(this Self&& self) {
        if (std::holds_alternative<E>(self.error)) {
            return std::optional(std::holds_alternative<E>(std::forward<Self>(self).error));
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
    template<typename Self, typename F>
    constexpr auto and_then(this Self&& self, F&& f)
        requires std::invocable<F, void> && traits::is_result_v<std::invoke_result_t<F, void>>
                 && traits::
                     contains_all_v<std::invoke_result_t<F, void>, typename Self::error_types>
    {
        // if there is an error, return said error immediately
        if (self.has_error()) {
            return std::visit([](auto&& var) -> std::invoke_result_t<F, void> {
                return std::forward<decltype(var)>(var);
            }, std::forward<Self>(self));
        }
        // otherwise, invoke the callable and return the result
        return std::invoke(f, std::forward<Self>(self).m_value);
    }
};

} // namespace zest

namespace zest::test {

// Custom error types
struct ErrorA : protected ResultError {};

struct ErrorB : protected ResultError {};

// Test callables for non-void Results
constexpr auto return_rvalue = [](int&& x) -> Result<int, ErrorA, ErrorB> {
    return x * 2;
};
constexpr auto return_lvalue = [](int& x) -> Result<int, ErrorA, ErrorB> {
    return x * 2;
};
constexpr auto return_const = [](const int& x) -> Result<int, ErrorA, ErrorB> {
    return x * 2;
};

// Test callables for void Results
constexpr auto void_success = []() -> Result<void, ErrorA, ErrorB> {
    return {};
};
constexpr auto void_failure = []() -> Result<void, ErrorA, ErrorB> {
    return ErrorA{};
};

// Compile-time test runner
constexpr bool run_tests() {
    // Non-void Result tests
    {
        // Test lvalue forwarding
        Result<int, ErrorA, ErrorB> res{42};
        auto new_res = res.and_then(return_lvalue);
        if (new_res.has_error() || new_res.get_value() != 84)
            return false;

        // Test const lvalue forwarding
        const Result<int, ErrorA, ErrorB> cres{42};
        auto cnew_res = cres.and_then(return_const);
        if (cnew_res.has_error() || cnew_res.get_value() != 84)
            return false;

        // Test rvalue forwarding
        auto rnew_res = Result<int, ErrorA, ErrorB>{42}.and_then(return_rvalue);
        if (rnew_res.has_error() || rnew_res.get_value() != 84)
            return false;

        // Test error propagation
        Result<int, ErrorA, ErrorB> err_res{ErrorA{}};
        auto err_new_res = err_res.and_then(return_lvalue);
        if (!err_new_res.has_error() || !err_new_res.get_error<ErrorA>())
            return false;
    }

    // Void Result tests
    {
        // Test successful chain
        Result<void, ErrorA, ErrorB> vres;
        auto vnew_res = vres.and_then(void_success);
        if (vnew_res.has_error())
            return false;

        // Test error propagation
        Result<void, ErrorA, ErrorB> verr_res{ErrorA{}};
        auto verr_new_res = verr_res.and_then(void_success);
        if (!verr_new_res.has_error() || !verr_new_res.get_error<ErrorA>())
            return false;

        // Test new error in callback
        auto vfail_res = Result<void, ErrorA, ErrorB>{}.and_then(void_failure);
        if (!vfail_res.has_error() || !vfail_res.get_error<ErrorA>())
            return false;
    }

    return true;
}

// Compile-time test execution
static_assert(run_tests(), "All tests passed");

} // namespace zest::test