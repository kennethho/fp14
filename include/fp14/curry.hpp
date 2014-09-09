#ifndef FP14_CURRY_HPP
#define FP14_CURRY_HPP

#include <tuple>
#include <utility>

namespace fp14 {

namespace detail {

template <size_t N, class... Args>
using nth_of = typename std::tuple_element<N, std::tuple<Args...>>::type;

template <size_t... N>
struct seq {};

template <size_t... N, size_t... M>
auto operator+ (seq<N...>, seq<M...>)
{
  return seq<N..., M...>();
}

template <size_t N>
struct make_seq;

template <>
struct make_seq<0>
{
  using type = seq<0>;
};
template <size_t N>
struct make_seq
{
  using type = decltype(typename make_seq<N-1>::type{} + seq<N>{});
};

template <class F, class... Args, size_t... N>
auto call_with_arg_pack_impl(F& f, std::tuple<Args...>&& arg_pack, seq<N...>)
{
  return f(std::get<N>(arg_pack)...);
}

template <class F, class... Args>
auto call_with_arg_pack(F& f, std::tuple<Args...>&& arg_pack)
{
  using call_seq = typename make_seq<sizeof...(Args) - 1>::type;
  return call_with_arg_pack_impl(f, std::move(arg_pack), call_seq{});
}



template <class Signature>
struct curry_builder;

template <class Signature, size_t RemainingArgs>
struct curry_binder;

template <class Signature>
struct curry_evaluator;



template <class R, class... Args>
struct curry_builder<R (Args...)>
{
  static_assert(sizeof...(Args) > 2, "You shall not enter!");

  template <class Func>
  static auto make(
    Func&& func)
  {
    using arg_type = nth_of<0, Args...>;
    return [func = std::forward<Func>(func)](arg_type arg) {
      auto curried_arg_pack = std::make_tuple(std::move(arg));

      return curry_binder<
        R (Args...),
        sizeof...(Args) - 1>::make(
          std::move(func),
          std::move(curried_arg_pack));
    };
  }
};

template <class R, class Arg1, class Arg2>
struct curry_builder<R (Arg1, Arg2)>
{
  template <class Func>
  static auto make(
    Func&& func)
  {
    using arg_type = Arg1;
    return [func = std::forward<Func>(func)](arg_type arg) {
      auto curried_arg_pack = std::make_tuple(std::move(arg));
      return curry_evaluator<R (Arg1, Arg2)>::make(
        std::move(func),
        std::move(curried_arg_pack));
    };
  }
};

template <class R, class Arg>
struct curry_builder<R (Arg)>
{
  template <class Func>
  static auto make(
    Func&& func)
  {
    return std::forward<Func>(func);
  }
};

template <class R>
struct curry_builder<R ()>
{
};



template <class R, class... Args, size_t RemainingArgs>
struct curry_binder</* Func =*/ R (Args...), RemainingArgs>
{
  static_assert(RemainingArgs > 2, "You shall not enter!");

  template <class Func, class CurriedArgsTuple>
  static auto make(
    Func&& func,
    CurriedArgsTuple&& curried_arg_pack)
  {
    static constexpr const size_t remaining_args = RemainingArgs;
    using arg_type = nth_of<sizeof...(Args) - remaining_args, Args...>;

    return [func = std::forward<Func>(func),
            curried_arg_pack = std::move(curried_arg_pack)]
            (arg_type arg)
      {
        return curry_binder<R (Args...), remaining_args - 1>::make(
          std::move(func),
          std::tuple_cat(
            std::move(curried_arg_pack),
            std::make_tuple(std::move(arg))));
      };
  }
};

template <class R, class... Args>
struct curry_binder</* Func =*/ R (Args...), /* RemainingArgs =*/ 2>
{
  template <class Func, class CurriedArgsTuple>
  static auto make(
    Func&& func,
    CurriedArgsTuple curried_arg_pack)
  {
    static constexpr const size_t remaining_args = 2;
    using arg_type = nth_of<sizeof...(Args) - remaining_args, Args...>;

    return [func = std::forward<Func>(func),
            curried_arg_pack = std::move(curried_arg_pack)]
            (arg_type arg)
      {
        return curry_evaluator<R (Args...)>::make(
          std::move(func),
          std::tuple_cat(
            std::move(curried_arg_pack),
            std::make_tuple(std::move(arg))));
      };
  }
};



template <class R, class... Args>
struct curry_evaluator</* Func =*/ R (Args...)>
{
  template <class Func, size_t... N, class CurriedArgsTuple>
  static auto make_impl(
    Func&& func,
    seq<N...>,
    CurriedArgsTuple curried_arg_pack)
  {
    static constexpr const size_t remaining_args = 1;
    using arg_type = nth_of<sizeof...(Args) - remaining_args, Args...>;

    return [func = std::forward<Func>(func),
            curried_arg_pack = std::move(curried_arg_pack)]
            (arg_type&& arg)
      {
        return func(std::get<N>(curried_arg_pack)..., std::move(arg));
      };
  }
  template <class Func, class CurriedArgsTuple>
  static auto make(
    Func&& func,
    CurriedArgsTuple curried_arg_pack)
  {
    static constexpr const size_t remaining_args = 1;
    using arg_type = nth_of<sizeof...(Args) - remaining_args, Args...>;

    return [func = std::forward<Func>(func),
            curried_arg_pack = std::move(curried_arg_pack)]
            (arg_type&& arg)
      {
        return call_with_arg_pack(
          func, 
          std::tuple_cat(
            std::move(curried_arg_pack),
            std::make_tuple(std::move(arg))));
      };
  }
};



template <typename F>
struct functor_curry_builder;

template <typename R, typename C, typename... A>
struct functor_curry_builder<R (C::*)(A...)> 
  : curry_builder<R (A...)>
{
};

template <typename R, typename C, typename... A>
struct functor_curry_builder<R (C::*)(A...) const> 
  : curry_builder<R (A...)>
{
};

} // namespace detail

template <class Functor>
auto curry(Functor func)
{
  return detail::functor_curry_builder<
    decltype(&Functor::operator())
    >::make(std::move(func));
}

template <class R, class... Args>
auto curry(R (*func)(Args...))
{
  return detail::curry_builder<R (Args...)>::make(func);
}

} // namespace fp14

#endif // FP14_CURRY_HPP
