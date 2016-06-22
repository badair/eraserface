/*
(C) Copyright Tobias Schwinger
(C) Copyright 2016 (Modified Work) Barrett Adair

Use modification and distribution are subject to the boost Software License,
Version 1.0. (See http://www.boost.org/LICENSE_1_0.txt).
*/

//***************************************************************************
static_assert(false, "Don't include this file; it is not library code."
" This file is solely for documentating the expansion behavior of the"
" DEFINE_ERASERFACE macro, which is defined in eraserface/eraserface.hpp.");

// The code in this file is the documented and formatted equivalent
// of the expansion of the following macro invocation:
//
//  DEFINE_ERASERFACE(interface_x,
//      ((print_member_data, void() const))
//      ((member_data,       int))
//  );
//***************************************************************************

// Most of our implementation logic is dumped into this
// class to reduce the chance of naming conflicts. We could
// use a namespace instead, except that namespaces can't be
// opened inside of class definitions. Anywhere you see "interface_x",
// "print_member_data", and "member_data", remember that these
// names originate from the macro parameters.
struct interface_x_detail {

  struct vtable {

    template <typename T = ::eraserface::secret>
    struct member_info0 {

      // this comes from the from the macro parameters
      using member_type = void() const;

      // this is used for member data, which will be accessed
      // through our interface via a member function of the same
      // name, taking no arguments and returning an lvalue reference.
      template <typename U, typename Member = member_type,
        bool = std::is_function<member_type>>::value>
      struct member_info {
        using result_type = std::add_lvalue_reference_t<Member>;
        using ptr_type = Member U::*;
        using is_const = typename std::is_const<Member>::type;
        using type_erased_ptr = result_type (*)(void *);
      };

      // this specialization is used for member functions.
      template <typename U>
      struct member_info<U, member_type, true> {
        using ptr_type = member_type U::*;
        using result_type = ::eraserface::ct::return_type_t<ptr_type>;
        using function_type = ::eraserface::ct::function_type_t<ptr_type>;
        using is_const = typename ::eraserface::ct::is_const_member<ptr_type>::type;

        // overwriting the U reference with void* and forwarding all parameters
        using type_erased_ptr = ::eraserface::ct::replace_args_t<0,
            typename ::eraserface::forward_all<function_type>::type,
            void*> *;
      };

      using info = member_info<T>;
      using ptr_type = typename info::ptr_type;
      using result_type = typename info::result_type;
      using type_erased_ptr = typename info::type_erased_ptr;
      using is_const = typename info::is_const;
    };

    //this saves us some typing later
    using pmf0 = typename member_info0<>::ptr_type;

    // This data member is our vtable function pointer entry,
    // which will be initialized at compile-time.
    typename member_info0<>::type_erased_ptr func0;

    // repeat above code for each interface member:

    template <typename T = ::eraserface::secret>
    struct member_info1 {

      using member_type = int;

      template <typename U, typename Member = member_type,
            bool = std::is_function<member_type>::value>
      struct member_info {
        using result_type = std::add_lvalue_reference_t<Member>;
        using ptr_type = Member U::*;
        using is_const = typename std::is_const<Member>::type;
        using type_erased_ptr = result_type (*)(void *);
      };

      template <typename U>
      struct member_info<U, member_type, true> {
        using ptr_type = member_type U::*;
        using result_type = ::eraserface::ct::return_type_t<ptr_type>;
        using function_type = ::eraserface::ct::function_type_t<ptr_type>;
        using is_const = typename ::eraserface::ct::is_const_member<ptr_type>::type;

        using type_erased_ptr = ::eraserface::ct::replace_args_t<0,
            typename ::eraserface::forward_all<function_type>::type,
            void*> *;
      };

      using info = member_info<T>;
      using ptr_type = typename info::ptr_type;
      using result_type = typename info::result_type;
      using type_erased_ptr = typename info::type_erased_ptr;
      using is_const = typename info::is_const;
    };

    using pmf1 = typename member_info1<>::ptr_type;
    typename member_info1<>::type_erased_ptr func1;
  };

  // This will be our root base class, containing our vtable pointer
  // and the void* necessary for type erasure.
  template <typename SemanticsTag>
  struct interface_root {
  protected:

    const vtable *ptr_vtable;

    static constexpr bool is_ref = ::std::is_same<SemanticsTag, ::eraserface::ref>::value;
    static constexpr bool is_shared = ::std::is_same<SemanticsTag, ::eraserface::shared>::value;

    // we use a shared_ptr when SematicsTag is eraserface::shared
    using ptr_type = ::std::conditional_t<is_ref, void *, ::std::shared_ptr<void>>;
    ptr_type obj_ptr;

    template <bool B = is_ref, class T, ::std::enable_if<B>::type * = nullptr>
    inline interface_root(T &that)
        : ptr_vtable(&vtable_holder<T>::val_vtable),
          obj_ptr(::std::addressof(that))
    {}

    template <bool B = is_shared, class T,
        typename ::std::enable_if<B>::type * = nullptr>
    inline interface_root(const ::std::shared_ptr<T> &that)
        : ptr_vtable(&vtable_holder<T>::val_vtable), obj_ptr(that)
    {}

    // the existence of vtable_holder and its usage in the constructors above
    // ensures that our vtable is initialized correctly at compile time
    template <class T> struct vtable_holder { static const vtable val_vtable; };
  };

  // for N interface members, our interface type inherits
  // N base classes, each one containing a proxy member function
  // for the respective interface member (notably, each proxy member
  // function may have its address taken - these are not templates)
  template <int I, typename SemanticsTag>
  struct base {
    
    // this specific type alias will not be used, but is necessary for
    // the eager template instantiation in the std::conditional_t used
    // by get_next_base below
    using type = SemanticsTag;
  };

  // We use get_next_base to daisy-chain our base classes, starting with interface_root
  template <int I, typename SemanticsTag>
  using get_next_base = std::conditional_t<I == 0,
      interface_root<SemanticsTag>, typename base<I - 1, SemanticsTag>::type>;

  template <typename SemanticsTag, typename... Args>
  struct choose_member_constness0 {

    // apply is necessary because we must meta-programmatically
    // choose the definition with the correct constness
    template <bool IsConst, typename Base = get_next_base<0, SemanticsTag>>
    struct apply;

    // non-const member definition
    template <typename Base>
    struct apply<false, Base> : Base {

      using Base::Base;
      using Base::ptr_vtable;
      using Base::obj_ptr;

      // member name comes from the macro parameters
      inline decltype(auto) print_member_data( ::eraserface::forward_t<Args>... args) {
        // perfect forwarding without templates
        return ptr_vtable->func0(
          ::eraserface::get_ptr(obj_ptr),
          ::eraserface::forward_t<Args>&&>(args)...
        );
      }
    };

    // const member definition
    template <typename Base>
    struct apply<true, Base> : Base {

      using Base::Base;
      using Base::ptr_vtable;
      using Base::obj_ptr;

      // member name comes from the macro parameters
      inline decltype(auto) print_member_data( ::eraserface::forward_t<Args>... args) const {
        // perfect forwarding without templates
        return ptr_vtable->func0(
          ::eraserface::get_ptr(obj_ptr),
          ::eraserface::forward_t<Args>&&>(args)...
        );
      }
    };
  };

  template <typename SemanticsTag>
  struct base<0, SemanticsTag> {

    using function_type = ::eraserface::ct::function_type_t<typename vtable::pmf0>;

    using impl = ::eraserface::ct::expand_args_right_t<
        ::eraserface::ct::pop_front_args_t<function_type>,
        choose_member_constness0,
        SemanticsTag>;

    using is_const = typename vtable::template member_info0<>::is_const;

    // the generated interface class will inherit from this
    using type = typename impl::template apply<is_const::value>;
  };

  // repeat above code for each interface member:

  template <typename SemanticsTag, typename... Args>
  struct choose_member_constness1 {

    template <bool IsConst, typename Base = get_next_base<1, SemanticsTag>>
    struct apply;

    template <typename Base>
    struct apply<false, Base> : Base {

      using Base::Base;
      using Base::ptr_vtable;
      using Base::obj_ptr;

      inline decltype(auto) member_data(::eraserface::forward_t<Args>... args) {
        return ptr_vtable->func1(
          ::eraserface::get_ptr(obj_ptr),
          ::eraserface::forward_t<Args>&&>(args)...
        );
      }
    };

    template <typename Base>
    struct apply<true, Base> : Base {

      using Base::Base;
      using Base::ptr_vtable;
      using Base::obj_ptr;

      inline decltype(auto) member_data(::eraserface::forward_t<Args>... args) const {
        return ptr_vtable->func1(
          ::eraserface::get_ptr(obj_ptr),
          ::eraserface::forward_t<Args>&&>(args)...
        );
      }
    };
  };

  template <typename SemanticsTag> struct base<1, SemanticsTag> {
    using function_type = ::eraserface::ct::function_type_t<typename vtable::pmf1>;

    using impl = ::eraserface::ct::expand_args_right_t<
        ::eraserface::ct::pop_front_args_t<function_type>,
        choose_member_constness1,
        SemanticsTag>;

    using is_const = typename vtable::template member_info1<>::is_const;

    using type = typename impl::template apply<is_const::value>;
  };
};

// this initializes the vtable at compile time for every class used to construct an
// interface_x object. Member functions and member data are both handled.
template <typename SemanticsTag>
template <typename T>
interface_x_detail::vtable const interface_x_detail::interface_root<SemanticsTag>::vtable_holder<T>::val_vtable = {

    // member names here comes from the macro parameters
    &::eraserface::member<typename vtable::template member_info0<T>::ptr_type, &T::print_member_data>::wrapper::wrap,
    &::eraserface::member<typename vtable::template member_info1<T>::ptr_type, &T::member_data>::wrapper::wrap
};

// this is the user-facing generated interface type
template <typename SemanticsTag = ::eraserface::ref>
struct interface_x : interface_x_detail::base<2 - 1, SemanticsTag>::type {
  static_assert(
      std::is_base_of<::eraserface::ownership_semantics, SemanticsTag>::value,
      "Template argument to "
      "interface_x"
      " must be "
      "eraserface::ref or eraserface::shared (default is ref)");

private:

  using detail = interface_x_detail;
  using base = typename detail::base<2 - 1, SemanticsTag>::type;

public:

  // SFINAE if T is the generated interface type
  template <typename T, ::std::enable_if_t<
    ! ::std::is_base_of<
        typename detail::template interface_root<SemanticsTag>,
        ::std::decay_t<T>>::value, int> = 0>
  inline interface_x(T &that) : base(that)
  {}

  // SFINAE if T is the generated interface type
  template <typename T, ::std::enable_if_t<
    ! ::std::is_base_of<
        typename detail::template interface_root<SemanticsTag>,
        ::std::decay_t<T>>::value, int> = 0>
  inline interface_x(const ::std::shared_ptr<T> &that) : base(that)
  {}

  inline interface_x(const interface_x &) = default;

  // allow reference interfaces to be assigned through shared interfaces
  template <bool B = ::std::is_same<SemanticsTag, ::eraserface::ref>::value,
    typename ::std::enable_if<B>::type * = nullptr>
  inline interface_x &operator=(const interface_x< ::eraserface::shared> &i) {
    base::ptr_vtable = i.ptr_vtable;
    base::obj_ptr = ::eraserface::get_ptr(i.obj_ptr);
    return *this;
  }

  // preventing accidental early destruction
  template <bool B = ::std::is_same<SemanticsTag, ::eraserface::ref>::value,
      typename ::std::enable_if<B>::type* = nullptr>
      inline name& operator=(name< ::eraserface::shared>&&) {
          static_assert(!B, "cannot assign an rvalue reference of "
              "interface_x<eraserface::shared> to a interface_x<> object.");
          return *this;
  }

  interface_x &operator=(const interface_x &) = default;

  // using declarations generated for each member
  using detail::base<0, SemanticsTag>::type::print_member_data;
  using detail::base<1, SemanticsTag>::type::member_data;

  // this function is used to instantiated reference-counted (shared)
  // eraserface objects.
  template <class T, class... Args>
  static inline interface_x< ::eraserface::shared> make_shared(Args &&... args) {
    return {
        ::std::make_shared<T>( ::std::forward<Args>(args)...)
    };
  }
};
