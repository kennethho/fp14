Functional-style Programming in C++14
====

Compiles using `clang++-3.5 -std=c++1y` and `clang++-3.6 -std=c++14`. No luck with g++-4.9.

Currying
----

```c++
#include <fp14/curry.hpp>

#include <cassert>

int avg3(int a, int b, int c)
{
  return (a + b + c) / 3;
}

int main()
{
  using namespace fp14;

  auto avg = curry(avg3)(1)(2)(3);
  assert(avg == 2);

  auto a = curry(
    // fp14::v2::curry() works with polymorphic lambdas!
    [](auto a, auto b)
    {
      return a < b ? b : a;
    });

  auto b = a(1);
  auto c = b(2);
  assert(c == 2);

  return 0;
}
```

