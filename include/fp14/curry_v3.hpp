#ifndef FP14_CURRY_V3_HPP
#define FP14_CURRY_V3_HPP

#include <tuple>
#include <utility>
#include <type_traits>

namespace fp14 {
inline namespace v3 {

namespace detail {

// argpack
template <class Argpack, class AddedArg>
constexpr auto argpack_push(Argpack&& argpack, AddedArg&& added_arg)
{
  return
    std::tuple_cat(
      std::forward<Argpack>(argpack),
      std::make_tuple(std::forward<AddedArg>(added_arg)));
}

template <class Arg0, class... OtherArgs, size_t... N>
constexpr auto argpack_pop_impl(std::tuple<Arg0, OtherArgs...>&& argpack, std::index_sequence<N...>)
{
  return std::tuple_cat(
    std::make_tuple(
       std::move(std::get<0>(argpack))),
    std::make_tuple(std::move(std::get<N>(argpack))...));
}
template <class Arg0, class... OtherArgs, size_t... N>
constexpr auto argpack_pop_impl(const std::tuple<Arg0, OtherArgs...>& argpack, std::index_sequence<N...>)
{
  return std::tuple_cat(
    std::make_tuple(
      std::get<0>(argpack)),
    std::make_tuple(std::get<N>(argpack)...));
}

template <class Arg0, class Arg1, class... OtherArgs>
constexpr auto argpack_pop(std::tuple<Arg0, Arg1, OtherArgs...>&& argpack)
{
  return argpack_pop_impl(std::move(argpack), std::make_index_sequence<sizeof...(OtherArgs)>{});
}
template <class Arg0, class Arg1, class... OtherArgs>
constexpr auto argpack_pop(const std::tuple<Arg0, Arg1, OtherArgs...>& argpack)
{
  return argpack_pop_impl(argpack, std::make_index_sequence<sizeof...(OtherArgs)>{});
}

template <class Arg0>
constexpr auto argpack_pop(const std::tuple<Arg0>& argpack)
{
  return std::tuple<>{};
}

template <class Func, class Argpack, size_t... N>
constexpr auto is_callable_with_argpack_impl(
  Func* func,
  Argpack* argpack,
  std::index_sequence<N...>)
  -> decltype((*func)(std::get<N>(*argpack)...), std::true_type{})
{
  return std::true_type{};
}

inline constexpr std::false_type is_callable_with_argpack_impl(...)
{
  return std::false_type{};
}

template <class Func, class Argpack>
constexpr auto is_callable_with_argpack(
  const Func&,
  const Argpack&)
{
  return is_callable_with_argpack_impl(
    reinterpret_cast<Func*>(0),
    reinterpret_cast<Argpack*>(0),
    std::make_index_sequence<std::tuple_size<Argpack>::value>());
}

template <class Func, class Argpack, size_t... N>
decltype(auto) call_with_argpack_impl(
  Func func,
  Argpack argpack,
  std::index_sequence<N...>)
{
  // have to use a copy of elements in `argpack` to call `func()`
  return func(std::get<N>(argpack)...);
}

template <class Func, class Argpack>
decltype(auto) call_with_argpack(
  Func&& func,
  Argpack&& argpack)
{
  return
    call_with_argpack_impl(
      std::forward<Func>(func),
      std::forward<Argpack>(argpack),
      std::make_index_sequence<std::tuple_size<std::remove_reference_t<Argpack>>::value>());
}



// callable with
template <class Arg>
struct callable_with_t
{
  using arg_type = std::add_rvalue_reference_t<Arg>;
};

inline constexpr std::false_type is_callable_with(...)
{
  return std::false_type{};
}
template <class Arg>
constexpr std::true_type is_callable_with(const callable_with_t<Arg>&)
{
  return std::true_type{};
}



// uncurry
struct uncurry_t {};

template <class Arg>
constexpr auto is_uncurry(const Arg&)
{
  // It appears there is no `std::is_same_t<>` alias
  return typename std::is_same<Arg, uncurry_t>::type{};
}



template <
  class Func,
  class CurriedArgpack,
  class AddedArg>
constexpr auto curry_dispatch(
  Func&& func,
  CurriedArgpack&& curried_argpack,
  AddedArg&& added_arg,
  std::enable_if_t<
    decltype(is_callable_with(added_arg))::value
  >* = nullptr)
{
  return decltype(is_callable_with_argpack(
    std::forward<Func>(func),
      argpack_push(
        std::forward<CurriedArgpack>(curried_argpack),
        std::declval<typename AddedArg::arg_type>()))){};
}

template <
  class Func,
  class CurriedArgpack,
  class AddedArg>
constexpr auto curry_dispatch(
  Func&& func,
  CurriedArgpack&& curried_argpack,
  AddedArg&& added_arg,
  std::enable_if_t<
    !decltype(is_callable_with(added_arg))::value &&
    !decltype(is_uncurry(added_arg))::value &&
    !decltype(
      is_callable_with_argpack(
        func,
        argpack_push(
          std::forward<CurriedArgpack>(curried_argpack),
          std::forward<AddedArg>(added_arg))))::value
  >* = nullptr);

template <
  class Func,
  class CurriedArgpack,
  class AddedArg>
constexpr decltype(auto) curry_dispatch(
  Func&& func,
  CurriedArgpack&& curried_argpack,
  AddedArg&& added_arg,
  std::enable_if_t<
    decltype(
      is_callable_with_argpack(
        std::forward<Func>(func),
        argpack_push(
          std::forward<CurriedArgpack>(curried_argpack),
          std::forward<AddedArg>(added_arg))))::value
  >* = nullptr)
{
  return call_with_argpack(
    std::forward<Func>(func),
    argpack_push(
      std::forward<CurriedArgpack>(curried_argpack),
      std::forward<AddedArg>(added_arg)));
}


template <
  class Func,
  class CurriedArgpack,
  class AddedArg>
constexpr auto curry_arg_expected(
  Func&& func,
  CurriedArgpack&& curried_argpack,
  AddedArg&& added_arg)
{
  return
    [func = std::forward<Func>(func),
      curried_argpack =
        argpack_push(
          std::forward<CurriedArgpack>(curried_argpack),
          std::forward<AddedArg>(added_arg))](auto&& added_arg) mutable -> decltype(auto)
    {
      return curry_dispatch(
        func,
        curried_argpack,
        std::forward<decltype(added_arg)>(added_arg));
    };
}

template <class Func, class CurriedArgpack>
constexpr Func uncurry(
  Func&& func,
  CurriedArgpack&&,
  std::enable_if_t<
    std::tuple_size<std::remove_reference_t<CurriedArgpack>>::value == 0
  >* = nullptr)
{
  return std::forward<Func>(func);
}

template <class Func, class CurriedArgpack>
constexpr auto uncurry(
  Func&& func,
  CurriedArgpack&& curried_argpack,
  std::enable_if_t<
    (std::tuple_size<std::remove_reference_t<CurriedArgpack>>::value > 0)
  >* = nullptr)
{
  return
    [func = std::forward<Func>(func),
     curried_argpack = argpack_pop(std::forward<CurriedArgpack>(curried_argpack))]
     (auto&& added_arg) mutable  -> decltype(auto)
    {
      return curry_dispatch(
        func,
        curried_argpack,
        std::forward<decltype(added_arg)>(added_arg));
    };
}

template <
  class Func,
  class CurriedArgpack,
  class AddedArg>
constexpr auto curry_dispatch(
  Func&& func,
  CurriedArgpack&& curried_argpack,
  AddedArg&& added_arg,
  std::enable_if_t<
    decltype(is_uncurry(added_arg))::value
  >* = nullptr)
{
  return uncurry(
    std::forward<Func>(func),
    std::forward<CurriedArgpack>(curried_argpack));
}


template <
  class Func,
  class CurriedArgpack,
  class AddedArg>
constexpr auto curry_dispatch(
  Func&& func,
  CurriedArgpack&& curried_argpack,
  AddedArg&& added_arg,
  std::enable_if_t<
    !decltype(is_callable_with(added_arg))::value &&
    !decltype(is_uncurry(added_arg))::value &&
    !decltype(
      is_callable_with_argpack(
        func,
        argpack_push(
          std::forward<CurriedArgpack>(curried_argpack),
          std::forward<AddedArg>(added_arg))))::value
  >*)
{
  return
    curry_arg_expected(
      std::forward<Func>(func),
      std::forward<CurriedArgpack>(curried_argpack),
      std::forward<AddedArg>(added_arg));
}

template <class Func>
auto curry(Func&& func)
{
  return
    [func = std::forward<Func>(func)](auto&& added_arg) mutable ->decltype(auto) {
      return curry_dispatch(
        func,
        std::tuple<>{},
        std::forward<decltype(added_arg)>(added_arg));
    };
}

} // namespace detail

template <class Func>
auto curry(Func&& func)
{
  return detail::curry(std::forward<Func>(func));
}

template <class Arg>
detail::callable_with_t<Arg> callable_with(const Arg&)
{
  return detail::callable_with_t<Arg>{};
}

namespace {
  const detail::uncurry_t uncurry{};
}


} //namespace v3
} // namespace fp14

#endif // FP14_CURRY_V3_HPP
