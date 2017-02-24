/*
(C) Copyright Tobias Schwinger
(C) Copyright 2016 (Modified Work) Barrett Adair

Use modification and distribution are subject to the boost Software License,
Version 1.0. (See http://www.boost.org/LICENSE_1_0.txt).
*/

#include <iostream>
#include <memory>
#include <eraserface/eraserface.hpp>

DEFINE_ERASERFACE(my_interface,
    ((a_func,       void(int) const))
    ((a_func,       void(long) const))
    ((another_func, int()))
    ((some_data,    const char*))
);

// two classes that implement my_interface
struct a_class {
    void a_func(int v) const {
        std::cout << "a_class::void a_func(int v = " << v << ")" << std::endl;
    }

    void a_func(long v) const {
        std::cout << "a_class::void a_func(long v = " << v << ")" << std::endl;
    }

    int another_func() {
        std::cout << "a_class::another_func() = 3" << std::endl;
        return 3;
    }

    const char* some_data = "a_class's data";
};

struct another_class {

    int n;

    another_class() = default;
    
    another_class(int i) : n(i) {}

    // Notice a_func is implemented as a function template? No problem for eraserface.
    template<typename T>
    void a_func(T v) const {
        std::cout <<
            "another_class::void a_func(T v = " << v << ")"
            "  [ T = " << typeid(T).name() << " ]" << std::endl;
    }


    int another_func() {
        std::cout << "another_class::another_func() = " << n << std::endl;
        return n;
    }

    const char* some_data = "another_class's data";
};

// Both classes above can be assigned to the interface variable and their
// member functions can be called through it.

void print_data(my_interface obj) {
    std::cout << obj.some_data() << std::endl;
}

int main() {
    a_class x;
    another_class y{ 43 };

    my_interface i = x;

    i.a_func(12);
    i.a_func(77L);
    i.another_func();

    print_data(i);
    i.some_data() = "x's data has changed.";
    print_data(x);

    // reusing the same interface object
    i = y;
    i.a_func(13);
    i.a_func(21L);
    i.another_func();
    print_data(y);
}
