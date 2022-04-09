#pragma once

namespace perlbind {

struct type_base
{
  type_base() : my_perl(PERL_GET_THX) {}
  type_base(PerlInterpreter* interp) : my_perl(interp) {}
  PerlInterpreter* my_perl = nullptr;
};

// helper type to allow null object reference arguments in bindings
template <typename T>
struct nullable
{
  static_assert(std::is_pointer<T>::value, "nullable<T> 'T' must be pointer");

  nullable() = default;
  nullable(T ptr) : m_ptr(ptr) {}
  T get() { return m_ptr; }
private:
  T m_ptr = nullptr;
};

} // namespace perlbind
