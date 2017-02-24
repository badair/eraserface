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
    ((f, int(int)))
    ((f, int()))
);

struct foo {
    int f(){return 1;}

    template<typename T>
    T f(T) {return 2;}
};

struct bar {

    template<typename T>
    T f() {return 3;}

    int f(int){return 4;};
};

int main() {

    foo my_foo;
    bar my_bar;

    test_interface ti = my_foo;
    assert(ti.f() == 1);
    assert(ti.f(0) == 2);

    ti = my_bar;
    assert(ti.f() == 3);
    assert(ti.f(0) == 4);
}
