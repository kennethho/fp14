#ifndef FP14_CURRY_HPP
#define FP14_CURRY_HPP

#include <tuple>
#include <utility>

namespace fp14 {

namespace detail {

template <size_t N, class... Args>
using nth_of = typename std::tuple_element<N, std::tuple<Args...>>::type;



template <class Signature>
struct curry_begin;

template <class Signature, size_t Remainings>
struct curry_continue;

template <class Signature>
struct curry_end;



template <class R, class... Args>
struct curry_begin<R (Args...)>
{
  static_assert(sizeof...(Args) > 2, "You shall not enter!");

  template <class Func>
  static auto build(
    Func &&func)
  {
    using arg_type = nth_of<0, Args...>;
    return [func = std::forward<Func>(func)](arg_type arg) {
      return curry_continue<
        R (Args...),
        sizeof...(Args) - 1>::build(
          std::move(func),
          arg);
    };
  }
};

template <class R, class Arg1, class Arg2>
struct curry_begin<R (Arg1, Arg2)>
{
  template <class Func>
  static auto build(
    Func &&func)
  {
    using arg_type = Arg1;
    return [func = std::forward<Func>(func)](arg_type arg) {
      return curry_end<R (Arg1, Arg2)>::build(
        std::move(func),
        arg);
    };
  }
};

template <class R, class Arg>
struct curry_begin<R (Arg)>
{
  template <class Func>
  static auto build(
    Func &&func)
  {
    return std::forward<Func>(func);
  }
};

template <class R>
struct curry_begin<R ()>
{
};



template <class R, class... Args, size_t Remainings>
struct curry_continue</* Func =*/ R (Args...), Remainings>
{
  static_assert(Remainings > 2, "You shall not enter!");

  template <class Func, class... CurriedArgs>
  static auto build(
    Func &&func,
    CurriedArgs&&... curried_args)
  {
    static constexpr const size_t remainings = Remainings;
    using arg_type = nth_of<sizeof...(Args) - remainings, Args...>;

    return [func = std::forward<Func>(func), curried_args...](arg_type arg) {
      return curry_continue<R (Args...), remainings - 1>::build(
        std::move(func), std::move(curried_args)..., arg);
    };
  }
};

template <class R, class... Args>
struct curry_continue</* Func =*/ R (Args...), /* Remainings =*/ 2>
{
  template <class Func, class... CurriedArgs>
  static auto build(
    Func &&func,
    CurriedArgs... curried_args)
  {
    static constexpr const size_t remainings = 2;
    using arg_type = nth_of<sizeof...(Args) - remainings, Args...>;

    return [func = std::forward<Func>(func), curried_args...](arg_type arg) {
      return curry_end<R (Args...)>::build(
        std::move(func), curried_args..., arg);
    };
  }
};



template <class R, class... Args>
struct curry_end</* Func =*/ R (Args...)>
{
  template <class Func, class... CurriedArgs>
  static auto build(
    Func &&func,
    CurriedArgs... curried_args)
  {
    static constexpr const size_t remainings = 1;
    using arg_type = nth_of<sizeof...(Args) - remainings, Args...>;

    return [func = std::forward<Func>(func), curried_args...](arg_type arg) {
      return func(curried_args..., arg);
    };
  }
};

template <typename F>
struct functor_curry_helper;

template <typename R, typename C, typename... A>
struct functor_curry_helper<R (C::*)(A...)> 
  : curry_begin<R (A...)>
{
};

template <typename R, typename C, typename... A>
struct functor_curry_helper<R (C::*)(A...) const> 
  : curry_begin<R (A...)>
{
};

} // namespace detail

template <class Functor>
auto curry(Functor func)
{
  return detail::functor_curry_helper<
    decltype(&Functor::operator())
    >::build(std::forward<Functor>(func));
}

template <class R, class... Args>
auto curry(R (*func)(Args...))
{
  return detail::curry_begin<R (Args...)>::build(func);
}

} // namespace fp14

#endif // FP14_CURRY_HPP
