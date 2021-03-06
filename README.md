Functional-style Programming in C++14
====

Compiles using `clang++-3.5 -std=c++1y` and `clang++-3.6 -std=c++14`. No luck with g++-4.9.

Currying
----

```c++
#include <fp14/curry.hpp>

#include <cassert>
#include <utility>
#include <functional>

using namespace fp14;
using namespace std;

int avg3(int a, int b, int c)
{
  return (a + b + c) / 3;
}
auto swap2 =
  [](auto& x, auto& y)
  {
    auto tmp = move(x);
    x = move(y);
    y = move(tmp);
  };

auto identity =
  [](auto& x) -> decltype(auto)
  {
    return x;
  };

void currying()
{
  assert( curry(avg3)(1)(2)(3) == 2 );
  assert( curry(avg3)(1, 2)(3) == 2 );
  assert( curry(avg3)(1)(2, 3) == 2 );
  assert( curry(avg3)(1, 2, 3) == 2 );

  auto a = curry(avg3)(1);
  assert( a(2, 3) == 2 );

  auto b = a(2);
  assert( b(3) == 2 );
}

void value_semantics()
{
  // fp14 assumes and supports value semantics by default

  auto a = curry(swap2);
  // Valid, but nothing changes
  a(0)(1);

  int x = 0, y = 1;

  // Like above, no visible side-effect
  auto b = a(x);
  b(y);
  assert( x == 0 && y == 1 );

  // `z` and `x` are two instances of the same value
  decltype(auto) z = curry(identity)(x);
  assert( x == z && &x != &z );
  
}

void referece_semantics()
{
  // For reference semantics, use `std::ref()` and `std::cref()`

  int x = 0, y = 1;

  // Now `x` and `y` can be swapped
  curry(swap2)(ref(x))(ref(y));
  assert( x == 1 && y == 0 );

  // `z` is a reference to `x`
  decltype(auto) z = curry(identity)(cref(x));
  assert( x == z && &x == &z );
}

void uncurrying()
{
  auto a = curry(avg3)(1);
  auto b = a(uncurry);
  assert( b(1)(2)(3) == 2 );

  auto c = b(uncurry);
  // This only holds true, if avg3 is a function (pointer), but not a
  // closure nor a functor (object)
  assert( c == avg3 );

  assert(c(1, 2, 3) == 2);
}

void callable_with()
{
#if(0)
  // Won't compile
  curry(avg3)(1)(2)(3)(4);
#endif

  // But these do, though malformed.
  // One shortcoming of fp14::curry() is that it couldn't tell the
  // difference between a malformed call and a curry function that's
  // expecting more arguments
  curry(avg3)(1)('2')("3")(4);
  auto malformed = curry(avg3)(1)(2)("3");
  malformed(4);

  // callable_with(arg) helps making sure calls are legit
  assert( curry(avg3)( 1 )(2)(callable_with("3")) == false );
  assert( curry(avg3)('1')(2)(callable_with( 3 )) == false );
  assert( curry(avg3)( 1 )(2)(callable_with( 3 )) == true  );
}

int main()
{
  currying();
  value_semantics();
  referece_semantics();
  uncurrying();
  callable_with();

  return 0;
}
```

More to come (hopefully)
----
