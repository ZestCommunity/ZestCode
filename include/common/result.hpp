#pragma once

#include <chrono>
#include <concepts>
#include <limits>
#include <stacktrace>
#include <variant> // IWYU pragma: keep (std::get is used, not sure why clangd says this header isn't needed)

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

/**
 * @brief Check whether a given type is in a typename pack
 *
 * @tparam T the type to check
 * @tparam Ts the typename pack to check
 */
template<typename T, typename... Ts>
concept InPack = (std::same_as<std::remove_cvref_t<T>, Ts> || ...);

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
 * @brief check whether a callable is invocable given the Result and argument type
 *
 * @tparam R the Result
 * @tparam F the callable
 * @tparam E the callable argument
 */
template<typename R, typename F, typename E>
concept Invocable = std::invocable<F, decltype((std::get<E>(std::declval<R>().error)))>;

/**
 * @brief check whether a callable is invocable with any of the given argument types
 *
 * @note Result::or_else helper
 *
 * @tparam R the Result
 * @tparam F the callable
 * @tparam Es the possible argument types passed to the callable
 */
template<typename R, typename F, typename... Es>
concept AnyInvocable = (Invocable<R, F, Es> || ...);

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
} // namespace traits

// forward-declare Result
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
 * @brief check whether the callable return types are all Result
 *
 * @note if an ignored_type is present, it won't be evaluated
 *
 * @note helper for Result::or_else
 * @note concept is satisfied with ignored_type as well
 *
 * @tparam Ts the type to check
 */
template<typename R, typename F, typename... Ts>
concept AllResult =
    ((IsResult<or_else_return_t<R, F, Ts>>
      || std::same_as<ignored_type, std::remove_cvref_t<or_else_return_t<R, F, Ts>>>)
     && ...);

/**
 * @brief check whether the value type of a Result is the other given type
 *
 * @note helper for Result::or_else
 * @note concept if satisfied if T is void or ignored_type
 *
 * @tparam R the Result
 * @tparam T the value type
 */
template<typename R, typename... Ts>
concept AllValueTypeMatch =
    ((std::same_as<void, Ts> || std::same_as<ignored_type, Ts>
      || std::same_as<typename R::value_type, Ts>)
     && ...);
} // namespace traits

} // namespace zest