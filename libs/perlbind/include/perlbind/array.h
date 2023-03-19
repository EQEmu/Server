#pragma once

#include "types.h"
#include "iterator.h"
#include <stdexcept>

namespace perlbind {

struct array : public type_base
{
  using iterator = detail::array_iterator;

  ~array() noexcept
  {
    SvREFCNT_dec(m_av);
  }

  array() noexcept
    : type_base(), m_av(newAV()) {}
  array(PerlInterpreter* interp) noexcept
    : type_base(interp), m_av(newAV()) {}
  array(const array& other) noexcept
    : type_base(other.my_perl), m_av(copy_array(other.m_av)) {}
  array(array&& other) noexcept
    : type_base(other.my_perl), m_av(other.m_av)
  {
    other.m_av = newAV();
  }
  array(AV*& value) noexcept
    : type_base(), m_av(copy_array(value)) {}
  array(AV*&& value) noexcept
    : type_base(), m_av(value) {} // take ownership
  array(scalar ref)
    : type_base(ref.my_perl)
  {
    if (!ref.is_array_ref())
      throw std::runtime_error("cannot construct array from non-array reference");

    reset(reinterpret_cast<AV*>(SvREFCNT_inc(*ref)));
  }
  array(scalar_proxy proxy)
    : array(scalar(SvREFCNT_inc(proxy.sv()))) {}

  array& operator=(const array& other) noexcept
  {
    if (this != &other)
      m_av = copy_array(other.m_av);

    return *this;
  }

  array& operator=(array&& other) noexcept
  {
    if (this != &other)
      std::swap(m_av, other.m_av);

    return *this;
  }

  array& operator=(AV*& value) noexcept
  {
    if (m_av != value)
      m_av = copy_array(value);

    return *this;
  }

  array& operator=(AV*&& value) noexcept
  {
    reset(value);
    return *this;
  }

  operator AV*() const { return m_av; }
  operator SV*() const { return reinterpret_cast<SV*>(m_av); }

  AV* release() noexcept
  {
    AV* tmp = m_av;
    m_av = newAV();
    return tmp;
  }

  void reset(AV* value) noexcept
  {
    SvREFCNT_dec(m_av);
    m_av = value;
  }

  void clear() noexcept               { av_clear(m_av); } // decreases refcnt of all SV elements
  scalar pop_back() noexcept          { return av_pop(m_av); }
  scalar pop_front() noexcept         { return av_shift(m_av); }
  void push_back(const scalar& value) { av_push(m_av, newSVsv(value)); }
  void push_back(scalar&& value)      { av_push(m_av, value.release()); }
  void reserve(size_t count)          { av_extend(m_av, count > 0 ? count - 1 : 0); }
  size_t size() const                 { return av_len(m_av) + 1; }
  SV* sv() const                      { return reinterpret_cast<SV*>(m_av); }

  // returns a proxy that takes ownership of one reference to the SV element
  // extends the array and creates an undef SV if index out of range
  scalar_proxy operator[](size_t index)
  {
    SV** sv = av_fetch(m_av, index, 1);
    return scalar_proxy(my_perl, SvREFCNT_inc(*sv));
  }

  iterator begin() const noexcept { return { my_perl, m_av, 0 }; }
  iterator end() const noexcept { return { my_perl, m_av, size() }; }

private:
  AV* copy_array(AV* other)
  {
    return av_make(av_len(other)+1, AvARRAY(other));
  }

  AV* m_av = nullptr;
};

} // namespace perlbind
