# Eraserface

_Type-**erase**d polymorphic inte**rface**s_

*Note: This project is based on the [Boost.FunctionTypes interface example](http://www.boost.org/doc/libs/1_61_0/libs/function_types/example/interface.hpp) by Tobias Schwinger.*

An interface is a collection of member function prototypes that may be implemented by classes. Objects of classes that implement the interface can then be assigned to an interface variable through which the interface's functions can be called.

Interfaces are a prominent feature in many object-oriented programming languages, [such as Java](https://en.wikipedia.org/wiki/Interface_(Java)). Historically, an "interface" in C++ is modeled by a pure abstract base class. However, the inheritance-based approach to interfaces has the following drawbacks:

* Inheritance-based interfaces require that all interface functions be virtual.
  * A function that calls another function of the interface must do so via virtual dispatch, and may not be inlined.
* A derived class cannot implement an interface function via function template.
* Inheritance-based interfaces are *intrusive*
  * The memory layout of an interface-derived object is altered and obscured:
    * A [vtable](https://en.wikipedia.org/wiki/Virtual_method_table) pointer must be stored, even for objects used in code where virtual lookup is not necessary
    * interface-derived objects cannot be [standard layout types](http://en.cppreference.com/w/cpp/concept/StandardLayoutType), by definition
    * Interfaces can only be applied to classes which can be modified by the programmer
    * Adding an interface requires all clients to be rebuilt
* Inheritance-based interfaces are a source of [tighter coupling](https://en.wikipedia.org/wiki/Coupling_%28computer_programming%29) - an interface needed for one small section of code must be inherited by a class that might be used in many unrelated sections of code

Fortunately, it is possible to mitigate some or all of these drawbacks by using an alternative approach based on [type erasure](http://stackoverflow.com/questions/5450159/type-erasure-techniques). Eraserface is an implementation of this approach using template metaprogramming and preprocessor metaprogramming techniques.

# Usage

Eraserface is contained to a single header file:

```cpp
    #include <eraserface/eraserface.hpp>
```

The `DEFINE_ERASERFACE` macro generates a type-erased interface type which can be used to apply an interface to an object, without altering class definitions. This allows objects to be interfaced polymorphically whose class definitions are not accessible to the programmer. In addition, the presence of implementations are checked and enforced at compile time, so there is no safety disadvantage to using this technique. Eraserface even accounts for member data, such as `some_data` below:

```cpp
    DEFINE_ERASERFACE( my_interface,
      (( a_func, void(int) const ))
      (( another_func, int() ))
      (( some_data, int ))
    );
```

This macro will generate an interface that *roughly* corresponds to the following abstract base class:

```cpp
    struct my_interface {
        virtual void a_func(int) const = 0;
        virtual int another_func() = 0;

        //(imagine a virtual data member, accessible with a function call)
        virtual int some_data;
    };
```

The difference is that an Eraserface interface is applied "inline", instead of at a class definition :

```cpp
    my_interface i = some_object;
```

# Caveats

First and foremost, Eraserface is a fun metaprogramming exercise. A handful of test cases exist in this repository, which one should review (and perhaps expand) before deciding to use Eraserface in their own project.

Eraserface requires access to member function pointers through the target object type. This brings 4 important considerations:

* Publicly inherited member functions cannot be used to implement an Eraserface interface, unless they are imported with `using` declarations in the derived class.
* Neither covariance nor contravariance are supported.
* objects of classes in the `std` namespace may not be used, since taking the address of a member function of a class in this namespace is undefined behavior.
* If interface members are implemented with a member function template, Eraserface will instantiate the template and [ODR-use](http://en.cppreference.com/w/cpp/language/definition%23One_Definition_Rule#ODR-use) the member function according to the respective signature(s) passed to the `DEFINE_ERASERFACE` macro.

The Eraserface macro adds two names to the current scope. The first macro parameter (e.g. `my_interface` in the previous example) is expanded to a struct. The first macro parameter is also appended with `_detail_`, which is a class in the current scope (e.g. `my_interface_detail_` in the previous example).

# Dependencies

Dependencies must be available in the include path.

* [Boost.CallableTraits](https://github.com/boostorg/callable_traits)
* [Boost.PreProcessor](http://www.boost.org/doc/libs/1_65_1/libs/preprocessor/doc/index.html)

# Compatibility

Eraserface worked at some point on GCC 4.9.3+ and Clang 3.8+, and maybe it still does. I need to set up continous integration tests. MSVC is not supported, largely (if not entirely) due to name-lookup compiler bugs.

# [License](LICENSE.md)
Distributed under the [Boost Software License, Version 1.0](http://boost.org/LICENSE_1_0.txt).

* (C) Copyright Tobias Schwinger
* (C) Copyright 2016, 2017 Barrett Adair
