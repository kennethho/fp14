Functional-style Programming in C++14
====

Compiles using `clang++-3.5 -std=c++1y` and `clang++-3.6 -std=c++14`. No luck with g++-4.9.

Currying
----

```c++
#include <fp14/curry.hpp>

#include <assert>

int avg3(int a, int b, int c)
{
  return (a + b + c) / 3;
}

auto max2 = [](int a, int b)
{
  return a < b ? b : a;
};

int main()
{
  auto avg = curry(avg3)(1)(2)(3);
  assert(avg == 2);

  auto a = curry(max2);
  auto b = a(1);
  auto c = b(2)
  assert(c == 2);
  
  return 0;
}
```

