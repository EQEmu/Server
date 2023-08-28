#pragma once

#include "types.h"
#include <string>
#include <type_traits>

namespace perlbind {

struct scalar : type_base
{
  virtual ~scalar() noexcept
  {
    SvREFCNT_dec(m_sv);
  }

  scalar() noexcept
    : type_base(), m_sv(newSV(0)) {} // nothing allocated
  scalar(PerlInterpreter* interp) noexcept
    : type_base(interp), m_sv(newSV(0)) {}
  scalar(PerlInterpreter* interp, SV*&& sv) noexcept
    : type_base(interp), m_sv(sv) {}
  scalar(const scalar& other) noexcept
    : type_base(other.my_perl), m_sv(newSVsv(other.m_sv)) {}
  scalar(scalar&& other) noexcept
    : type_base(other.my_perl), m_sv(other.m_sv)
  {
    other.m_sv = newSV(0);
  }
  scalar(SV*& value) noexcept
    : type_base(), m_sv(newSVsv(value)) {}
  scalar(SV*&& value) noexcept
    : type_base(), m_sv(value) {}
  scalar(const char* value) noexcept
    : type_base(), m_sv(newSVpv(value, 0)) {}
  scalar(const std::string& value) noexcept
    : type_base(), m_sv(newSVpvn(value.c_str(), value.size())) {}

  template <typename T, std::enable_if_t<detail::is_signed_integral_or_enum<T>::value, bool> = true>
  scalar(T value) noexcept : type_base(), m_sv(newSViv(static_cast<IV>(value))) {}

  template <typename T, std::enable_if_t<std::is_unsigned<T>::value, bool> = true>
  scalar(T value) noexcept : type_base(), m_sv(newSVuv(value)) {}

  template <typename T, std::enable_if_t<std::is_floating_point<T>::value, bool> = true>
  scalar(T value) noexcept : type_base(), m_sv(newSVnv(value)) {}

  template <typename T, std::enable_if_t<std::is_pointer<T>::value, bool> = true>
  scalar(T value) noexcept : type_base(), m_sv(newSV(0))
  {
    *this = std::move(value);
  }

  scalar& operator=(const scalar& other) noexcept
  {
    if (this != &other)
      sv_setsv(m_sv, other.m_sv);

    return *this;
  }

  scalar& operator=(scalar&& other) noexcept
  {
    if (this != &other)
      std::swap(m_sv, other.m_sv);

    return *this;
  }

  scalar& operator=(SV*& value) noexcept
  {
    sv_setsv(m_sv, value);
    return *this;
  }

  scalar& operator=(SV*&& value) noexcept
  {
    reset(value);
    return *this;
  }

  scalar& operator=(const char* value) noexcept
  {
    sv_setpv(m_sv, value);
    return *this;
  }

  scalar& operator=(const std::string& value) noexcept
  {
    sv_setpvn(m_sv, value.c_str(), value.size());
    return *this;
  }

  template <typename T, std::enable_if_t<detail::is_signed_integral_or_enum<T>::value, bool> = true>
  scalar& operator=(T value) noexcept
  {
    sv_setiv(m_sv, static_cast<IV>(value));
    return *this;
  }

  template <typename T, std::enable_if_t<std::is_unsigned<T>::value, bool> = true>
  scalar& operator=(T value) noexcept
  {
    sv_setuv(m_sv, value);
    return *this;
  }

  template <typename T, std::enable_if_t<std::is_floating_point<T>::value, bool> = true>
  scalar& operator=(T value) noexcept
  {
    sv_setnv(m_sv, value);
    return *this;
  }

  template <typename T, std::enable_if_t<std::is_pointer<T>::value, bool> = true>
  scalar& operator=(T value) noexcept
  {
    // bless if it's in the typemap
    const char* type_name = detail::typemap::template get_name<T>(my_perl);
    sv_setref_pv(m_sv, type_name, static_cast<void*>(value));
    return *this;
  }

  operator SV*() const { return m_sv; }
  operator void*() const { return m_sv; }
  operator const char*() const { return SvPV_nolen(m_sv); }
  operator std::string() const { return SvPV_nolen(m_sv); }
  template <typename T, std::enable_if_t<detail::is_signed_integral_or_enum<T>::value, bool> = true>
  operator T() const { return static_cast<T>(SvIV(m_sv)); }
  template <typename T, std::enable_if_t<std::is_unsigned<T>::value, bool> = true>
  operator T() const { return static_cast<T>(SvUV(m_sv)); }
  template <typename T, std::enable_if_t<std::is_floating_point<T>::value, bool> = true>
  operator T() const { return static_cast<T>(SvNV(m_sv)); }
  template <typename T, std::enable_if_t<std::is_pointer<T>::value, bool> = true>
  operator T() const
  {
    const char* type_name = detail::typemap::template get_name<T>(my_perl);
    if (type_name && sv_isobject(m_sv) && sv_derived_from(m_sv, type_name))
    {
      IV tmp = SvIV(SvRV(m_sv));
      return INT2PTR(T, tmp);
    }
    return nullptr;
  }

  template <typename T>
  T as() const { return static_cast<T>(*this); }

  // release ownership of SV
  SV* release() noexcept
  {
    SV* tmp = m_sv;
    m_sv = newSV(0);
    return tmp;
  }
  // take ownership of an SV
  void reset(SV* value) noexcept
  {
    SvREFCNT_dec(m_sv);
    m_sv = value;
  }

  SV* sv()      const { return m_sv; }
  SV* deref()   const { return SvRV(m_sv); }
  size_t size() const { return SvPOK(m_sv) ? sv_len(m_sv) : 0; }
  svtype type() const { return SvTYPE(m_sv); }
  const char* c_str() const { return SvPV_nolen(m_sv); }

  SV* operator*() { return SvRV(m_sv); }

  bool is_null()       const { return type() == SVt_NULL; } //SvOK(m_sv)
  bool is_integer()    const { return SvIOK(m_sv); }
  bool is_float()      const { return SvNOK(m_sv); }
  bool is_string()     const { return SvPOK(m_sv); }
  bool is_reference()  const { return SvROK(m_sv); }
  bool is_scalar_ref() const { return SvROK(m_sv) && SvTYPE(SvRV(m_sv)) < SVt_PVAV; }
  bool is_array_ref()  const { return SvROK(m_sv) && SvTYPE(SvRV(m_sv)) == SVt_PVAV; }
  bool is_hash_ref()   const { return SvROK(m_sv) && SvTYPE(SvRV(m_sv)) == SVt_PVHV; }

protected:
  SV* m_sv = nullptr;
};

// references are scalars that take ownership of one new reference to a value
// use reset() to take ownership of an existing RV
struct reference  : public scalar
{
  reference() = default;

  template <typename T, std::enable_if_t<std::is_base_of<type_base, T>::value, bool> = true>
  reference(T& value) noexcept : scalar(value.my_perl, nullptr) { m_sv = newRV_inc(value); }

  // increments referent for rvalues of scalar objects (not raw SVs) since they dec on destruct
  template <typename T, std::enable_if_t<std::is_base_of<type_base, T>::value, bool> = true>
  reference(T&& value) noexcept : scalar(value.my_perl, nullptr) { m_sv = newRV_inc(value); }

  template <typename T, std::enable_if_t<detail::is_any<T, SV*, AV*, HV*>::value, bool> = true>
  reference(T& value) noexcept { reset(newRV_inc(reinterpret_cast<SV*>(value))); }

  template <typename T, std::enable_if_t<detail::is_any<T, SV*, AV*, HV*>::value, bool> = true>
  reference(T&& value) noexcept { reset(newRV_noinc(reinterpret_cast<SV*>(value))); }

  SV* operator*() { return SvRV(m_sv); }
};

// scalar proxy reference is used for array and hash index operator[] overloads
struct scalar_proxy
{
  scalar_proxy() = delete;
  scalar_proxy(PerlInterpreter* interp, scalar&& value) noexcept
    : my_perl(interp), m_value(std::move(value)) {}

  SV* sv() const { return m_value; }
  const char* c_str() const { return static_cast<const char*>(m_value); }

  template <typename T>
  T as() const { return m_value.as<T>(); }

  operator std::string() const { return m_value; }

  // copying value to supported conversion types (e.g. int val = arr[i])
  template <typename T, std::enable_if_t<!std::is_base_of<type_base, T>::value, bool> = true>
  operator T() const
  {
    return static_cast<T>(m_value);
  }

  // taking a reference to the source SV (e.g. scalar val = arr[i])
  template <typename T, std::enable_if_t<std::is_same<T, scalar>::value, bool> = true>
  operator T() const
  {
    return SvREFCNT_inc(m_value);
  }

  // assigning scalar to proxy, the source SV is modified (arr[i] = "new value")
  scalar_proxy& operator=(scalar value)
  {
    m_value = value;
    return *this;
  }

  scalar_proxy& operator=(const scalar_proxy& other)
  {
    m_value = other.m_value;
    return *this;
  }

  // todo: nested proxy[]

private:
  PerlInterpreter* my_perl = nullptr;
  scalar m_value;
};

using ref = reference;

} // namespace perlbind
