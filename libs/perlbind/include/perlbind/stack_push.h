#pragma once

#include <string>

namespace perlbind { namespace stack {

// base class for pushing value types to perl stack
// methods use macros that push new mortalized SVs but do not extend the stack
// the stack is only extended when pushing an array, hash, or using push_args().
// this is because for xsubs the "stack is always large enough to take one return value"
struct pusher
{
  virtual ~pusher() = default;

  pusher() = delete;
  pusher(PerlInterpreter* interp) : my_perl(interp), sp(PL_stack_sp) {}

  SV* pop() { return POPs; }

  void push(bool value) { PUSHs(boolSV(value)); ++m_pushed; }
  void push(const char* value)
  {
    if (!value)
      PUSHs(&PL_sv_undef);
    else
      mPUSHp(value, strlen(value));

    ++m_pushed;
  }
  void push(const std::string& value) { mPUSHp(value.c_str(), value.size()); ++m_pushed; }
  void push(scalar value) { mPUSHs(value.release()); ++m_pushed; }
  void push(reference value) { mPUSHs(value.release()); ++m_pushed; }

  void push(array value)
  {
    int count = static_cast<int>(value.size());
    EXTEND(sp, count);
    for (int i = 0; i < count; ++i)
    {
      // mortalizes one reference to array element to avoid copying
      SV** sv = av_fetch(static_cast<AV*>(value), i, true);
      mPUSHs(SvREFCNT_inc(*sv));
    }
    m_pushed += count;
  }

  void push(hash value)
  {
    // hashes are pushed to the perl stack as alternating keys and values
    // this is less efficient than pushing a reference to the hash
    auto count = hv_iterinit(value) * 2;
    EXTEND(sp, count);
    while (HE* entry = hv_iternext(value))
    {
      auto val = HeVAL(entry);
      PUSHs(hv_iterkeysv(entry)); // mortalizes new key sv (keys are not stored as sv)
      PUSHs(sv_2mortal(SvREFCNT_inc(val)));
    }
    m_pushed += count;
  }

  template <typename T, std::enable_if_t<detail::is_signed_integral_or_enum<T>::value, bool> = true>
  void push(T value) { mPUSHi(static_cast<IV>(value)); ++m_pushed; }

  template <typename T, std::enable_if_t<std::is_unsigned<T>::value, bool> = true>
  void push(T value) { mPUSHu(value); ++m_pushed; }

  template <typename T, std::enable_if_t<std::is_floating_point<T>::value, bool> = true>
  void push(T value) { mPUSHn(value); ++m_pushed; }

  template <typename T, std::enable_if_t<std::is_pointer<T>::value, bool> = true>
  void push(T value)
  {
    const char* type_name = detail::typemap::get_name<T>(my_perl);
    if (!type_name)
    {
      throw std::runtime_error("cannot push unregistered pointer of type '" + util::type_name<T>::str() + "'");
    }

    SV* sv = sv_newmortal();
    sv_setref_pv(sv, type_name, static_cast<void*>(value));
    PUSHs(sv);
    ++m_pushed;
  };

  void push(void* value)
  {
    SV* sv = sv_newmortal();
    sv_setref_pv(sv, nullptr, value); // unblessed
    PUSHs(sv);
    ++m_pushed;
  }

  template <typename... Args>
  void push_args(Args&&... args)
  {
    EXTEND(sp, sizeof...(Args));
    push_args_impl(std::forward<Args>(args)...);
  };

protected:
  PerlInterpreter* my_perl = nullptr;
  SV** sp = nullptr;
  int m_pushed = 0;

private:
  template <typename... Args>
  void push_args_impl(Args&&... args) {}

  template <typename T, typename... Args>
  void push_args_impl(T&& value, Args&&... args)
  {
    push(std::forward<T>(value));
    push_args_impl(std::forward<Args>(args)...);
  }
};

} // namespace stack
} // namespace perlbind
