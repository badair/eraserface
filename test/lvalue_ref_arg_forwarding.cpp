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

struct copy_counter {

    static int copy_count;

    copy_counter() = default;
    copy_counter(const copy_counter&) {
        copy_count++;
    }
};

int copy_counter::copy_count = 0;

DEFINE_ERASERFACE(test_interface,
    ((f, int(copy_counter&) const))
);

struct foo {
    int f(copy_counter&) const { return 1; }
};

struct bar {
    int f(copy_counter&) const { return 2; }
};

int main() {

    foo my_foo;
    bar my_bar;
    copy_counter counter;

    test_interface<> ti = my_foo;

    assert(ti.f(counter) == 1);
    assert(copy_counter::copy_count == 0);

    ti = my_bar;

    assert(ti.f(counter) == 2);
    assert(copy_counter::copy_count == 0);

    ti = ti;
    assert(ti.f(counter) == 2);
    assert(copy_counter::copy_count == 0);
}
