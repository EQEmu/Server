#pragma once

namespace perlbind { namespace detail {

template<typename T, typename... Rest>
struct is_any : std::false_type {};
template<typename T, typename Last>
struct is_any<T, Last> : std::is_same<T, Last> {};
template<typename T, typename First, typename... Rest>
struct is_any<T, First, Rest...> : std::integral_constant<bool, std::is_same<T, First>::value || is_any<T, Rest...>::value> {};

template <typename T>
struct is_signed_integral : std::integral_constant<bool, std::is_integral<T>::value && std::is_signed<T>::value> {};

template <typename T>
struct is_signed_integral_or_enum : std::integral_constant<bool, is_signed_integral<T>::value || std::is_enum<T>::value> {};

template <typename T, typename... Rest>
struct count_of : std::integral_constant<size_t, 0> {};
template <typename T, typename Last>
struct count_of<T, Last> : std::integral_constant<size_t, std::is_same<T, Last>::value ? 1 : 0> {};
template <typename T, typename Next, typename... Rest>
struct count_of<T, Next, Rest...> : std::integral_constant<size_t, count_of<T, Next>::value + count_of<T, Rest...>::value> {};

template <typename T, typename... Args>
struct is_last : std::false_type {};
template <typename T, typename Last>
struct is_last<T, Last> : std::is_same<T, Last> {};
template <typename T, typename Next, typename... Args>
struct is_last<T, Next, Args...> : std::integral_constant<bool, is_last<T, Args...>::value> {};

} // namespace detail
} // namespace perlbind
