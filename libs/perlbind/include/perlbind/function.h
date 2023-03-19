#pragma once

namespace perlbind { namespace detail {

// traits for function and class method exports
template <typename Ret, typename Class, typename... Args>
struct base_traits
{
  using return_t = Ret;
  using sig_t = util::type_name<Args...>;
  using stack_tuple = std::conditional_t<std::is_void<Class>::value,
                                         std::tuple<Args...>,
                                         std::tuple<Class*, Args...>>;
  static constexpr int arity = sizeof...(Args);
  static constexpr int stack_arity = sizeof...(Args) + (std::is_void<Class>::value ? 0 : 1);
  static constexpr int vararg_count = count_of<array, Args...>::value +
                                      count_of<hash, Args...>::value;
  static constexpr bool is_vararg = vararg_count > 0;
  static constexpr bool is_vararg_last = is_last<array, Args...>::value ||
                                         is_last<hash, Args...>::value;

  static_assert(!is_vararg || (vararg_count == 1 && is_vararg_last),
    "A function may only accept a single array or hash and it must be "
    "be the last parameter. Prefer using reference parameters instead.");
};

template <typename T, bool = std::is_class<T>::value>
struct function_traits : public function_traits<decltype(&T::operator()), true> {};

template <typename Ret, typename... Args>
struct function_traits<Ret(*)(Args...), false> : base_traits<Ret, void, Args...>
{
  using type = Ret(*)(Args...);
};

template <typename Ret, typename Class, typename... Args>
struct function_traits<Ret(Class::*)(Args...), false> : base_traits<Ret, Class, Args...>
{
  using type = Ret(Class::*)(Args...);
};

template <typename Ret, typename Class, typename... Args>
struct function_traits<Ret(Class::*)(Args...) const, false> : base_traits<Ret, Class, Args...>
{
  using type = Ret(Class::*)(Args...) const;
};

template <typename Ret, typename Class, typename... Args>
struct function_traits<Ret(Class::*)(Args...) const, true> : base_traits<Ret, void, Args...>
{
  using type = Ret(*)(Args...);
};

// represents a bound native function
struct function_base
{
  virtual ~function_base() = default;
  virtual std::string get_signature() const = 0;
  virtual bool is_compatible(xsub_stack&) const = 0;
  virtual void call(xsub_stack&) const = 0;

  static const MGVTBL mgvtbl;
};

template <typename T>
struct function : public function_base, function_traits<T>
{
  using target_t = typename function::type;
  using return_t = typename function::return_t;

  function() = delete;
  function(PerlInterpreter* interp, T func)
    : my_perl(interp), m_func(func) {}

  std::string get_signature() const override
  {
    return util::type_name<target_t>::str();
  };

  bool is_compatible(xsub_stack& stack) const override
  {
    return function::is_vararg || stack.check_types(typename function::stack_tuple{});
  }

  void call(xsub_stack& stack) const override
  {
    if (!function::is_vararg && stack.size() != function::stack_arity)
    {
      using sig = typename function::sig_t;
      int count = std::is_member_function_pointer<T>::value ? stack.size() - 1 : stack.size();
      SV* err = newSVpvf("'%s(%s)' called with %d argument(s), expected %d\n argument(s): (%s)\n",
                         stack.name().c_str(), sig::str().c_str(), count, function::arity, stack.types().c_str());
      err = sv_2mortal(err);
      throw std::runtime_error(SvPV_nolen(err));
    }

    call_impl(stack, std::is_void<function::return_t>());
  }

private:
  void call_impl(xsub_stack& stack, std::false_type) const
  {
    return_t result = apply(m_func, stack.convert_stack(typename function::stack_tuple{}));
    stack.push_return(std::move(result));
  }

  void call_impl(xsub_stack& stack, std::true_type) const
  {
    apply(m_func, stack.convert_stack(typename function::stack_tuple{}));
  }

  // c++14 call function template with tuple arg unpacking (c++17 can use std::apply())
  template <typename F, typename Tuple, size_t... I>
  auto call_func(F func, Tuple&& t, std::index_sequence<I...>) const
  {
    return func(std::get<I>(std::forward<Tuple>(t))...);
  }

  template <typename F, typename Tuple, size_t... I>
  auto call_member(F method, Tuple&& t, std::index_sequence<I...>) const
  {
    return (std::get<0>(t)->*method)(std::get<I + 1>(std::forward<Tuple>(t))...);
  }

  template <typename F, typename Tuple, std::enable_if_t<!std::is_member_function_pointer<F>::value, bool> = true>
  auto apply(F func, Tuple&& t) const
  {
    using make_sequence = std::make_index_sequence<std::tuple_size<Tuple>::value>;
    return call_func(func, std::forward<Tuple>(t), make_sequence{});
  }

  template <typename F, typename Tuple, std::enable_if_t<std::is_member_function_pointer<F>::value, bool> = true>
  auto apply(F func, Tuple&& t) const
  {
    using make_sequence = std::make_index_sequence<std::tuple_size<Tuple>::value - 1>;
    return call_member(func, std::forward<Tuple>(t), make_sequence{});
  }

  PerlInterpreter* my_perl = nullptr;
  T m_func;
};

} // namespace detail
} // namespace perlbind
