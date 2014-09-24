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

void basic_currying()
{
  auto avg = curry(avg3)(1)(2)(3);
  assert(avg == 2);
}

void reference_semantics()
{
  auto swap2 =
    [](auto&& x, auto&& y)
    {
      auto tmp = forward<decltype(x)>(x);
      x = forward<decltype(y)>(y);
      y = move(tmp);
    };

  int x = 0, y = 1;

  auto a = curry(swap2);
  a(0)(1); // nothing's changed

  a(x)(y); // like above, nothing's changed
  assert(x == 0 && y == 1);
  
  a(ref(x))(ref(y)); // use std::ref for reference semantics
  assert(x == 1 && y == 0);
}

void uncurrying()
{
  auto a = curry(avg3)(1);
  auto b = a(uncurry);
  assert(b(1)(2)(3) == 2);

  auto c = b(uncurry);
  assert(c(1, 2, 3) == 2);
  
  // this only holds if avg3 is a function (pointer),
  // but not a closure nor functor (object)
  assert(c == avg3);
}

void callable_with()
{
  // won't compile
  // curry(avg3)(1)(2)(3)(4);

  // but this does, though malformed...
  // one shortcoming of fp14::curry() is that it couldn't
  // tell the difference between a malformed call and a
  // call that's expecting more arguments
  curry(avg3)(1)(2)("ha");

  // callable_with(arg) helps making sure calls are legit
  assert( curry(avg3)(1)(2)(callable_with(3)) == true );
  assert( curry(avg3)(1)(2)(callable_with("ha")) == false );
}

int main()
{
  basic_currying();
  reference_semantics();
  uncurrying();
  callable_with();
  
  return 0;
}
```

More to come (hopefully)
----
