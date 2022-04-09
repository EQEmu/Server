#pragma once

#include "stack_push.h"
#include "stack_read.h"
#include <algorithm>
#include <string>
#include <tuple>

namespace perlbind { namespace detail {

// handles xsub call stack from perl, inherits stack::pusher to push return values
class xsub_stack : public stack::pusher
{
public:
  xsub_stack() = delete;
  xsub_stack(PerlInterpreter* my_perl, CV* cv)
    : stack::pusher(my_perl)
  {
    GV* gv = CvGV(cv);
    m_sub_name = GvNAME(gv);
    m_pkg_name = HvNAME(GvSTASH(gv));

    dXSARGS;
    this->sp = sp;
    this->ax = ax;
    this->mark = mark;
    this->items = items;
  }
  ~xsub_stack() { XSRETURN(m_pushed); }

  int size() const { return items; }
  std::string name() const { return std::string(pkg_name()) + "::" + sub_name(); }
  const char* pkg_name() const { return m_pkg_name; }
  const char* sub_name() const { return m_sub_name; }

  template <typename T>
  void push_return(T&& value)
  {
    XSprePUSH;
    push(std::forward<T>(value));
  }

  // returns true if all perl stack arguments are compatible with expected native arg types
  template <typename Tuple>
  bool check_types(Tuple&& types)
  {
    static constexpr int count = std::tuple_size<Tuple>::value;
    if (items != count)
      return false;
    else if (count == 0)
      return true;

    using make_sequence = std::make_index_sequence<count>;
    return check_stack(std::forward<Tuple>(types), make_sequence());
  }

  // returns tuple of converted perl stack arguments, throws on an incompatible type
  template <typename Tuple>
  auto convert_stack(Tuple&& types)
  {
    using make_sequence = std::make_index_sequence<std::tuple_size<Tuple>::value>;
    return get_stack(std::forward<Tuple>(types), make_sequence());
  }

  std::string types()
  {
    std::string args;
    for (int i = 0; i < items; ++i)
    {
      args += get_type_name(ST(i));
      if (i < (items - 1))
        args += ", ";
    }
    return args.empty() ? "void" : args;
  }

protected:
  int ax = 0;
  int items = 0;
  SV** mark = nullptr;
  const char* m_pkg_name = nullptr;
  const char* m_sub_name = nullptr;

  std::string get_type_name(SV* item)
  {
    switch (SvTYPE(item))
    {
    case SVt_NULL: return "<undefined>";
    case SVt_NV:   return "double";
    case SVt_PV:   return "string";
    case SVt_PVAV: return "array";
    case SVt_PVHV: return "hash";
    case SVt_IV:
      if (sv_isobject(item))
        return std::string(sv_reftype(SvRV(item), true)) + "*";
      else if (SvROK(item))
        return "ref";
      else
        return "int";
    default:
      return sv_reftype(item, true);
    }
  }

private:
  template <typename T>
  bool check_index(T t, size_t index)
  {
    return stack::read_as<T>::check(my_perl, static_cast<int>(index), ax, items);
  }

  // return true if perl stack matches all expected argument types in tuple
  template <typename Tuple, size_t... I>
  bool check_stack(Tuple&& t, std::index_sequence<I...>)
  {
    // lists compatibility of each expected arg type (no short-circuit)
    std::initializer_list<bool> res = {
      check_index(std::get<I>(std::forward<Tuple>(t)), I)... };

    return std::all_of(res.begin(), res.end(), [](bool same) { return same; });
  }

  template <typename T>
  T get_stack_index(T t, size_t index)
  {
    return stack::read_as<T>::get(my_perl, static_cast<int>(index), ax, items);
  }

  template <typename Tuple, size_t... I>
  auto get_stack(Tuple&& t, std::index_sequence<I...>)
  {
    return Tuple{ get_stack_index(std::get<I>(std::forward<Tuple>(t)), I)... };
  }
};

} // namespace detail
} // namespace perlbind
