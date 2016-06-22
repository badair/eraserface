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
    int f = 1;
};

struct bar {
    int f = 2;
};

int main() {

    foo my_foo;
    bar my_bar;

    test_interface<> ti = my_foo;
    assert(ti.f() == 1);

    ti = my_bar;
    assert(ti.f() == 2);

    ti = ti;
    assert(ti.f() == 2);
}
