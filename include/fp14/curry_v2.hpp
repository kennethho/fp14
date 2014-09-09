#ifndef FP14_CURRY_V2_HPP
#define FP14_CURRY_V2_HPP

#include <tuple>
#include <utility>
#include <type_traits>

namespace fp14 {
inline namespace v2 {

namespace detail {

template <size_t... N>
struct seq {};

template <size_t... N, size_t... M>
auto operator+ (seq<N...>, seq<M...>)
{
  return seq<N..., M...>();
}

template <size_t N>
struct make_seq
{
  using type = decltype(typename make_seq<N-1>::type{} + seq<N>{});
};

template <>
struct make_seq<0>
{
  using type = seq<0>;
};


template <class Func, class... Args, size_t... N>
auto call_with_arg_pack_impl(Func func, std::tuple<Args...> arg_pack, seq<N...>)
{
  return func(std::forward<Args>(std::get<N>(arg_pack))...);
}

template <class Func, class... Args>
auto call_with_arg_pack(Func&& func, std::tuple<Args...> arg_pack)
{
  using call_seq = typename make_seq<sizeof...(Args) - 1>::type;
  return call_with_arg_pack_impl(
    std::forward<Func>(func), std::move(arg_pack), call_seq{});
}



// http://stackoverflow.com/questions/9530928/checking-a-member-exists-possibly-in-a-base-class-c11-version
template <class Func, class... Args>
constexpr auto is_callable_with_arg_pack(Func func, std::tuple<Args...> arg_pack)
  -> decltype(func(std::forward<Args>(std::declval<Args>())...), std::true_type{})
{
  return std::true_type{};
}
constexpr auto is_callable_with_arg_pack(...)
{
  return std::false_type{};
}

// http://stackoverflow.com/questions/20709896/how-do-i-use-stdenable-if-with-a-self-deducing-return-type
enum class enabler_t {};

template <class Func, class... Args>
using EnableIfCallableWithArgPack =
  typename std::enable_if<decltype(
    is_callable_with_arg_pack(
      std::declval<Func>(),
      std::declval<std::tuple<Args...>>()))::value, enabler_t>::type;

template <class Func, class... Args>
using DisableIfCallableWithArgPack =
  typename std::enable_if<!decltype(
    is_callable_with_arg_pack(
      std::declval<Func>(),
      std::declval<std::tuple<Args...>>()))::value, enabler_t>::type;



template <class Func, class... Args>
auto curry_impl(
  Func&& func,
  std::tuple<Args...> curried_arg_pack,
  EnableIfCallableWithArgPack<Func, Args...>* = nullptr)
{
  return call_with_arg_pack(std::forward<Func>(func), std::move(curried_arg_pack));
}

template <class Func, class... Args>
auto curry_impl(
  Func&& func,
  std::tuple<Args...> curried_arg_pack,
  DisableIfCallableWithArgPack<Func, Args...>* = nullptr)
{
  return [func = std::forward<Func>(func), curried_arg_pack](auto arg) {
    return curry_impl(
      std::forward<Func>(func),
      std::tuple_cat(std::move(curried_arg_pack), std::make_tuple(std::move(arg))));
  };
}

} // namespace detail

template <class Func>
auto curry(Func func)
{
  return [func = std::forward<Func>(func)](auto arg) {
    return detail::curry_impl(std::move(func), std::make_tuple(std::move(arg)));
  };
}

} //namespace v2
} // namespace fp14

#endif // FP14_CURRY_V2_HPP
