/*<-
(C) Copyright Tobias Schwinger
(C) Copyright 2016 (Modified Work) Barrett Adair

Use modification and distribution are subject to the boost Software License,
Version 1.0. (See http://www.boost.org/LICENSE_1_0.txt).

-----------------------------------------------------------------------------
See interface.hpp in this directory for details.
->*/

#include <eraserface/eraserface.hpp>
#include <cassert>
#undef NDEBUG

DEFINE_ERASERFACE(test_interface,
    ((f, int))
);

struct foo {
    int f;
    static bool destroyed;
    foo(int i) : f(i) {}
    ~foo() {destroyed = true;}
};

struct bar {
    int f;
    static bool destroyed;
    bar(int i) : f(i) {}
    ~bar() {destroyed = true;}
};

bool foo::destroyed = false;
bool bar::destroyed = false;

int main() {

    auto ti = test_interface<>::make_shared<foo>(5);
    assert(ti.f() == 5);

    assert(!foo::destroyed);
    ti = test_interface<>::make_shared<bar>(7);
    assert(foo::destroyed);

    assert(ti.f() == 7);
    assert(!bar::destroyed);
}
