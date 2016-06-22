// (C) Copyright Tobias Schwinger
// (C) Copyright 2016 (Modified Work) Barrett Adair
//
// Use modification and distribution are subject to the boost Software License,
// Version 1.0. (See http://www.boost.org/LICENSE_1_0.txt).

#ifndef ERASERFACE_HPP
#define ERASERFACE_HPP

#include <callable_traits/arg_at.hpp>
#include <callable_traits/return_type.hpp>
#include <callable_traits/expand_args.hpp>
#include <callable_traits/replace_args.hpp>
#include <callable_traits/apply_return.hpp>
#include <callable_traits/function_type.hpp>
#include <callable_traits/pop_front_args.hpp>
#include <callable_traits/push_front_args.hpp>
#include <callable_traits/is_const_member.hpp>
#include <callable_traits/remove_member_cv.hpp>
#include <callable_traits/expand_args_right.hpp>
#include <callable_traits/remove_member_reference.hpp>
#include <callable_traits/qualified_parent_class_of.hpp>

#include <boost/preprocessor/seq/seq.hpp>
#include <boost/preprocessor/seq/elem.hpp>
#include <boost/preprocessor/seq/size.hpp>
#include <boost/preprocessor/tuple/elem.hpp>
#include <boost/preprocessor/facilities/empty.hpp>
#include <boost/preprocessor/repetition/repeat.hpp>
#include <boost/preprocessor/punctuation/comma_if.hpp>


#include <tuple>
#include <memory>
#include <utility>
#include <type_traits>

namespace eraserface {

    namespace ct = callable_traits;

    //this is our placeholder parent class for member pointers
    struct secret {};

    struct ownership_semantics {};
    struct ref : ownership_semantics {};
    struct shared : ownership_semantics {};

    // used to forward a parameter without copying it
    template<typename T>
    using forward_t = std::conditional_t<
        std::is_reference<T>::value, T, T const &>;
    
    template<typename T>
    struct forward_all;

    template<typename... Args>
    struct forward_all_helper {
        using type = std::tuple<forward_t<Args>...>;
    };
    
    template<typename F>
    struct forward_all {
        using return_type = ct::return_type_t<F>;
        using forwarded_args = typename ct::expand_args_t<F, forward_all_helper>::type;
        using type = ct::apply_return_t<forwarded_args, return_type>;
    };
    
    template<typename T>
    struct remove_member_pointer { using type = T; };

    template<typename T, typename C>
    struct remove_member_pointer<T C::*> { using type = T; };

    // The member struct erases the object reference type that is supplied by
    // function_type, which aliases an INVOKE-aware function type when
    // a pmf is passed to it. We replace the first argument (which is
    // a reference to an object of the member ptr's parent class, as required
    // by INVOKE) with void*.

    template<typename Ptr, Ptr Value, bool IsPmf =
        std::is_member_function_pointer<Ptr>::value>
    struct member;

    inline constexpr void* get_ptr(void* p) {
        return p;
    }

    inline void* get_ptr(const std::shared_ptr<void>& p) {
        return p.get();
    }

    // this is our type erasure "glue". member<...>::wrapper::wrap is a static member function which
    // casts a void* back to its original object type, and invokes the appropriate member. This first
    // definition handles member functions.
    template<typename Pmf, Pmf PmfValue>
    struct member<Pmf, PmfValue, true> {

        // qualified_parent_class_of yields a reference type which is qualified
        // according to the member function type.
        using context =
            std::remove_reference_t<ct::qualified_parent_class_of_t<Pmf>>;

        template<typename... Args>
        struct member_wrapper {

            static inline decltype(auto) wrap(void* c, ::eraserface::forward_t<Args>... args) {
                //hand-rolled perfect forwarding
                return (reinterpret_cast<context*>(c)->*PmfValue)
                    (static_cast< ::eraserface::forward_t<Args>&&>(args)...);
            };
        };

        // removing the member pointer so that expand_args below doesn't include
        // the INVOKE-required object argument
        using abominable_function_type =
            typename ::eraserface::remove_member_pointer<Pmf>::type;

        // expand_args is used to expand the argument types into member_wrapper
        using wrapper = ::eraserface::ct::expand_args_t<
            abominable_function_type, member_wrapper>;
    };

    // This specialization handles member data.
    template<typename T, typename C, T C::* PmdValue>
    struct member<T C::*, PmdValue, false> {

        using no_ref = std::remove_reference_t<T>;

        static constexpr const bool is_const = std::is_const<no_ref>::value;

        using context = std::conditional_t<is_const, const C, C>;

        struct wrapper {

            static inline T& wrap(void* c) {
                return reinterpret_cast<context*>(c)->*PmdValue;
            };
        };
    };
}

#define ERASERFACE_IMPL(name) BOOST_PP_CAT(name, _detail)

// the interface definition on the client's side
#define DEFINE_ERASERFACE(name,def)                                     \
struct ERASERFACE_IMPL(name) {                                          \
                                                                        \
    struct vtable {                                                     \
        ERASERFACE_MEMBERS(def, VTABLE)                                 \
    };                                                                  \
                                                                        \
    template<typename SemanticsTag>                                     \
    struct interface_root {                                             \
    protected:                                                          \
                                                                        \
        const vtable * ptr_vtable;                                      \
                                                                        \
        static constexpr bool is_ref =                                  \
            ::std::is_same<SemanticsTag, ::eraserface::ref>::value;     \
                                                                        \
        static constexpr bool is_shared =                               \
            ::std::is_same<SemanticsTag, ::eraserface::shared>::value;  \
                                                                        \
        using ptr_type = ::std::conditional_t<is_ref,                   \
            void*, ::std::shared_ptr<void>>;                            \
                                                                        \
        ptr_type obj_ptr;                                               \
                                                                        \
        template <bool B = is_ref, class T,                             \
            typename ::std::enable_if<B>::type* = nullptr>              \
        inline interface_root(T& that)                                  \
            : ptr_vtable(&vtable_holder<T>::val_vtable),                \
            obj_ptr( ::std::addressof(that)) {}                         \
                                                                        \
        template <bool B = is_shared, class T,                          \
            typename ::std::enable_if<B>::type* = nullptr>              \
        inline interface_root(const ::std::shared_ptr<T> & that)        \
            : ptr_vtable(&vtable_holder<T>::val_vtable),                \
            obj_ptr(that) {}                                            \
                                                                        \
        template <class T>                                              \
        struct vtable_holder {                                          \
            static const vtable val_vtable;                             \
        };                                                              \
    };                                                                  \
                                                                        \
    template <int I, typename SemanticsTag>                             \
    struct base {                                                       \
        using type = SemanticsTag;                                      \
    };                                                                  \
                                                                        \
    template<int I, typename SemanticsTag>                              \
    using get_next_base = std::conditional_t<I == 0,                    \
        interface_root<SemanticsTag>,                                   \
        typename base<I - 1, SemanticsTag>::type>;                      \
                                                                        \
    ERASERFACE_MEMBERS(def, BASES)                                      \
};                                                                      \
                                                                        \
template <typename SemanticsTag>                                        \
template <typename T> ERASERFACE_IMPL(name)::vtable const               \
    ERASERFACE_IMPL(name)::interface_root<SemanticsTag                  \
        >::vtable_holder<T>                                             \
            ::val_vtable = { ERASERFACE_MEMBERS(def, INIT_VTABLE) };    \
                                                                        \
template<typename SemanticsTag = ::eraserface::ref>                     \
struct name : ERASERFACE_IMPL(name)                                     \
    ::base<BOOST_PP_SEQ_SIZE(def) - 1, SemanticsTag>::type {            \
                                                                        \
    static_assert( ::std::is_base_of< ::eraserface::ownership_semantics,\
        SemanticsTag>::value, "Template argument to " #name " must be " \
        "eraserface::ref or eraserface::shared (default is ref)");      \
                                                                        \
private:                                                                \
                                                                        \
    using detail = ERASERFACE_IMPL(name);                               \
                                                                        \
    using base = typename detail::base<                                 \
        BOOST_PP_SEQ_SIZE(def) - 1, SemanticsTag>::type;                \
                                                                        \
public:                                                                 \
                                                                        \
    template<typename T, ::std::enable_if_t<! ::std::is_base_of<        \
        typename detail::template interface_root<SemanticsTag>,         \
        ::std::decay_t<T>>::value, int> = 0>                            \
    inline name(T& that) : base(that) {}                                \
                                                                        \
    template<typename T, ::std::enable_if_t<! ::std::is_base_of<        \
        typename detail::template interface_root<SemanticsTag>,         \
        ::std::decay_t<T>>::value, int> = 0>                            \
    inline name(const ::std::shared_ptr<T>& that) : base(that) {}       \
                                                                        \
    inline name(const name &) = default;                                \
                                                                        \
    template <bool B = ::std::is_same<                                  \
        SemanticsTag, ::eraserface::ref>::value,                        \
        typename ::std::enable_if<B>::type* = nullptr>                  \
    inline name& operator=(const name< ::eraserface::shared> & i) {     \
        base::ptr_vtable = i.ptr_vtable;                                \
        base::obj_ptr = ::eraserface::get_ptr(i.obj_ptr);               \
        return *this;                                                   \
    }                                                                   \
                                                                        \
    template <bool B = ::std::is_same<                                  \
        SemanticsTag, ::eraserface::ref>::value,                        \
        typename ::std::enable_if<B>::type* = nullptr>                  \
    inline name& operator=(name< ::eraserface::shared>&&) {             \
        static_assert(!B, "cannot assign an rvalue reference of " #name \
        "<eraserface::shared> to a " #name "<> object.");               \
        return *this;                                                   \
    }                                                                   \
                                                                        \
    name & operator=(const name&) = default;                            \
                                                                        \
    ERASERFACE_MEMBERS(def, USING_DECLARATIONS)                         \
                                                                        \
    template <class T, class... Args>                                   \
    static inline name< ::eraserface::shared>                           \
    make_shared(Args&&... args) {                                       \
        return {                                                        \
            ::std::make_shared<T>( ::std::forward<Args>(args)...)       \
        };                                                              \
    }                                                                   \
}                                                                       \
/**/

// preprocessing code details

// iterate all of the interface's members and invoke a macro, prefixed
// with ERASERFACE_
#define ERASERFACE_MEMBERS(seq,macro)                                  \
    BOOST_PP_REPEAT(BOOST_PP_SEQ_SIZE(seq),                            \
        ERASERFACE_ ## macro, seq)                                     \
/**/

// generate the vtable initilizer code
#define ERASERFACE_INIT_VTABLE(z,i,seq)                                \
    ERASERFACE_INIT_VTABLE_I(i,                                        \
        BOOST_PP_TUPLE_ELEM(2,0,BOOST_PP_SEQ_ELEM(i,seq)))             \
/**/

#define ERASERFACE_INIT_VTABLE_I(i,mem)                                \
    BOOST_PP_COMMA_IF(i)                                               \
        &::eraserface::member<                                         \
            typename vtable::template                                  \
                BOOST_PP_CAT(member_info, i)<T>::ptr_type,             \
            &T::mem                                                    \
        >::wrapper::wrap                                               \
/**/

//generate the vtable
#define ERASERFACE_VTABLE(z,i,seq)                                     \
    ERASERFACE_VTABLE_I(z,i,                                           \
        BOOST_PP_TUPLE_ELEM(2,1,BOOST_PP_SEQ_ELEM(i,seq)))             \
/**/

#define ERASERFACE_VTABLE_I(z,i,signature)                             \
template<typename T = ::eraserface::secret>                            \
struct BOOST_PP_CAT(member_info, i) {                                  \
                                                                       \
    using member_type = signature;                                     \
                                                                       \
    template<typename U, typename Member = member_type,                \
        bool = std::is_function<member_type>::value>                   \
    struct member_info {                                               \
                                                                       \
        using result_type = std::add_lvalue_reference_t<Member>;       \
        using ptr_type = Member U::*;                                  \
                                                                       \
        using is_const = typename std::is_const<Member>::type;         \
                                                                       \
        using type_erased_ptr = result_type(*)(void*);                 \
    };                                                                 \
                                                                       \
    template<typename U>                                               \
    struct member_info <U, member_type, true> {                        \
                                                                       \
        using ptr_type = member_type U::*;                             \
        using result_type = ::eraserface::ct::return_type_t<ptr_type>; \
                                                                       \
        using function_type =                                          \
            ::eraserface::ct::function_type_t<ptr_type>;               \
                                                                       \
        using is_const =                                               \
            typename ::eraserface::ct::is_const_member<ptr_type>::type;\
                                                                       \
        using type_erased_ptr = ::eraserface::ct::replace_args_t<0,    \
            typename ::eraserface::forward_all<function_type>::type,   \
            void*> *;                                                  \
    };                                                                 \
                                                                       \
    using info = member_info<T>;                                       \
    using ptr_type = typename info::ptr_type;                          \
    using result_type = typename info::result_type;                    \
    using type_erased_ptr = typename info::type_erased_ptr;            \
    using is_const = typename info::is_const;                          \
};                                                                     \
                                                                       \
using BOOST_PP_CAT(pmf, i) =                                           \
    typename BOOST_PP_CAT(member_info, i)<>::ptr_type;                 \
                                                                       \
typename BOOST_PP_CAT(member_info, i)<>                                \
    ::type_erased_ptr BOOST_PP_CAT(func, i);                           \
/**/

// generate the bases, each of which will contain a public-facing
// interface function
#define ERASERFACE_BASES(z,i,seq)                                      \
    ERASERFACE_BASES_I(i,                                              \
        BOOST_PP_TUPLE_ELEM(2,0,BOOST_PP_SEQ_ELEM(i,seq)))             \
/**/

#define ERASERFACE_BASES_I(i, mem)                                     \
template<typename SemanticsTag, typename... Args>                      \
struct BOOST_PP_CAT(choose_member_constness, i) {                      \
                                                                       \
    template<bool IsConst,                                             \
        typename Base = get_next_base<i, SemanticsTag>>                \
    struct apply;                                                      \
                                                                       \
    ERASERFACE_BASES_APPLY(i, mem, false, BOOST_PP_EMPTY())            \
    ERASERFACE_BASES_APPLY(i, mem, true,  const)                       \
                                                                       \
};                                                                     \
                                                                       \
template <typename SemanticsTag>                                       \
struct base<i, SemanticsTag> {                                         \
                                                                       \
    using function_type = ::eraserface::ct::function_type_t<           \
        typename vtable::BOOST_PP_CAT(pmf, i)>;                        \
                                                                       \
    using impl = ::eraserface::ct::expand_args_right_t<                \
        ::eraserface::ct::pop_front_args_t<function_type>,             \
        BOOST_PP_CAT(choose_member_constness, i), SemanticsTag>;       \
                                                                       \
    using is_const =                                                   \
        typename vtable::template BOOST_PP_CAT(member_info, i)<>       \
            ::is_const;                                                \
                                                                       \
    using type = typename impl::template apply<is_const::value>;       \
};                                                                     \
/**/

// generate specializations based on qualifiers on member functions/data.
// lvalue and rvalue member functions are ignored.
#define ERASERFACE_BASES_APPLY(i, mem, isconst, qualifiers)            \
    template<typename Base>                                            \
    struct apply<isconst, Base> : Base {                               \
                                                                       \
        using Base::Base;                                              \
        using Base::ptr_vtable;                                        \
        using Base::obj_ptr;                                           \
                                                                       \
        inline decltype(auto)                                          \
        mem( ::eraserface::forward_t<Args>... args) qualifiers {       \
            return ptr_vtable->BOOST_PP_CAT(func, i)(                  \
              ::eraserface::get_ptr(obj_ptr),                          \
              static_cast< ::eraserface::forward_t<Args>&&>(args)...); \
        }                                                              \
    };                                                                 \
/**/

#define ERASERFACE_USING_DECLARATIONS(z,i,seq)                         \
    using detail::base<i, SemanticsTag>::type::                        \
        BOOST_PP_TUPLE_ELEM(2,0,BOOST_PP_SEQ_ELEM(i,seq));             \
/**/

#endif //#ifndef ERASERFACE_HPP
