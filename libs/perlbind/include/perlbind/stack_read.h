#pragma once

#include <string>

namespace perlbind { namespace stack {

// perl stack reader to convert types, throws if perl stack value isn't type compatible
template <typename T, typename = void>
struct read_as;

template <typename T>
struct read_as<T, std::enable_if_t<std::is_integral<T>::value || std::is_enum<T>::value>>
{
  static bool check(PerlInterpreter* my_perl, int i, int ax, int items)
  {
#ifdef PERLBIND_NO_STRICT_SCALAR_TYPES
    return SvTYPE(ST(i)) < SVt_PVAV;
#elif !defined PERLBIND_STRICT_NUMERIC_TYPES
    return SvNIOK(ST(i));
#else
    return SvIOK(ST(i));
#endif
  }

  static T get(PerlInterpreter* my_perl, int i, int ax, int items)
  {
    if (!check(my_perl, i, ax, items))
    {
      throw std::runtime_error("expected argument " + std::to_string(i+1) + " to be an integer");
    }
    return static_cast<T>(SvIV(ST(i))); // unsigned and bools casted
  }
};

template <typename T>
struct read_as<T, std::enable_if_t<std::is_floating_point<T>::value>>
{
  static bool check(PerlInterpreter* my_perl, int i, int ax, int items)
  {
#ifdef PERLBIND_NO_STRICT_SCALAR_TYPES
    return SvTYPE(ST(i)) < SVt_PVAV;
#elif !defined PERLBIND_STRICT_NUMERIC_TYPES
    return SvNIOK(ST(i));
#else
    return SvNOK(ST(i));
#endif
  }

  static T get(PerlInterpreter* my_perl, int i, int ax, int items)
  {
    if (!check(my_perl, i, ax, items))
    {
      throw std::runtime_error("expected argument " + std::to_string(i+1) + " to be a floating point");
    }
    return static_cast<T>(SvNV(ST(i)));
  }
};

template <>
struct read_as<const char*>
{
  static bool check(PerlInterpreter* my_perl, int i, int ax, int items)
  {
#ifdef PERLBIND_NO_STRICT_SCALAR_TYPES
    return SvTYPE(ST(i)) < SVt_PVAV;
#else
    return SvPOK(ST(i));
#endif
  }

  static const char* get(PerlInterpreter* my_perl, int i, int ax, int items)
  {
    if (!check(my_perl, i, ax, items))
    {
      throw std::runtime_error("expected argument " + std::to_string(i+1) + " to be a string");
    }
    return static_cast<const char*>(SvPV_nolen(ST(i)));
  }
};

template <>
struct read_as<std::string> : read_as<const char*>
{
};

template <>
struct read_as<void*>
{
  static bool check(PerlInterpreter* my_perl, int i, int ax, int items)
  {
    return sv_isobject(ST(i));
  }

  static void* get(PerlInterpreter* my_perl, int i, int ax, int items)
  {
    if (!check(my_perl, i, ax, items))
    {
      throw std::runtime_error("expected argument " + std::to_string(i+1) + " to be a reference to an object");
    }

    IV tmp = SvIV(SvRV(ST(i)));
    return INT2PTR(void*, tmp);
  }
};

template <typename T>
struct read_as<T, std::enable_if_t<std::is_pointer<T>::value>>
{
  static bool check(PerlInterpreter* my_perl, int i, int ax, int items)
  {
    const char* type_name = detail::typemap::get_name<T>(my_perl);
    return type_name && sv_isobject(ST(i)) && sv_derived_from(ST(i), type_name);
  }

  static T get(PerlInterpreter* my_perl, int i, int ax, int items)
  {
    if (!check(my_perl, i, ax, items))
    {
      // would prefer to check for unregistered types at compile time (not possible?)
      const char* type_name = detail::typemap::get_name<T>(my_perl);
      if (!type_name)
      {
        throw std::runtime_error("expected argument " + std::to_string(i+1) + " to be a reference to an unregistered type (method unusable)");
      }
      throw std::runtime_error("expected argument " + std::to_string(i+1) + " to be a reference to an object of type '" + type_name + "'");
    }

    IV tmp = SvIV(SvRV(ST(i)));
    return INT2PTR(T, tmp);
  }
};

template <typename T>
struct read_as<nullable<T>>
{
  static bool check(PerlInterpreter* my_perl, int i, int ax, int items)
  {
    return true;
  }

  static nullable<T> get(PerlInterpreter* my_perl, int i, int ax, int items)
  {
    if (sv_isobject(ST(i)))
    {
      const char* type_name = detail::typemap::get_name<T>(my_perl);
      if (type_name && sv_derived_from(ST(i), type_name))
      {
        IV tmp = SvIV(SvRV(ST(i)));
        return INT2PTR(T, tmp);
      }
    }
    return nullptr;
  }
};

template <>
struct read_as<SV*>
{
  static bool check(PerlInterpreter* my_perl, int i, int ax, int items)
  {
    return i < items;
  }

  static SV* get(PerlInterpreter* my_perl, int i, int ax, int items)
  {
    if (!check(my_perl, i, ax, items))
    {
      throw std::runtime_error("expected argument " + std::to_string(i+1) + " to be valid scalar value");
    }
    return ST(i);
  }
};

// scalar, array, and hash readers return reference to stack items (not copies)
template <>
struct read_as<scalar>
{
  static bool check(PerlInterpreter* my_perl, int i, int ax, int items)
  {
    return (SvROK(ST(i)) && SvTYPE(SvRV(ST(i))) < SVt_PVAV) || SvTYPE(ST(i)) < SVt_PVAV;
  }

  static scalar get(PerlInterpreter* my_perl, int i, int ax, int items)
  {
    if (!check(my_perl, i, ax, items))
    {
      throw std::runtime_error("expected argument " + std::to_string(i+1) + " to be a scalar or reference to a scalar");
    }
    return SvROK(ST(i)) ? SvREFCNT_inc(SvRV(ST(i))) : SvREFCNT_inc(ST(i));
  }
};

template <>
struct read_as<reference>
{
  static bool check(PerlInterpreter* my_perl, int i, int ax, int items)
  {
    return SvROK(ST(i));
  }

  static reference get(PerlInterpreter* my_perl, int i, int ax, int items)
  {
    if (!check(my_perl, i, ax, items))
    {
      throw std::runtime_error("expected argument " + std::to_string(i+1) + " to be a reference");
    }
    // take ownership of a reference to the RV itself (avoid reference to a reference)
    reference result;
    result.reset(SvREFCNT_inc(ST(i)));
    return result;
  }
};

template <>
struct read_as<array>
{
  static bool check(PerlInterpreter* my_perl, int i, int ax, int items)
  {
    return items > i;
  }

  static array get(PerlInterpreter* my_perl, int i, int ax, int items)
  {
    if (!check(my_perl, i, ax, items))
    {
      throw std::runtime_error("expected argument " + std::to_string(i+1) + " to be start of a perl array");
    }

    array result;
    result.reserve(items - i);
    for (int index = i; index < items; ++index)
    {
      result.push_back(SvREFCNT_inc(ST(index)));
    }
    return result;
  }
};

template <>
struct read_as<hash>
{
  static bool check(PerlInterpreter* my_perl, int i, int ax, int items)
  {
    int remaining = items - i;
    return remaining > 0 && remaining % 2 == 0 && SvTYPE(ST(i)) < SVt_PVAV;
  }

  static hash get(PerlInterpreter* my_perl, int i, int ax, int items)
  {
    if (!check(my_perl, i, ax, items))
    {
      throw std::runtime_error("expected argument " + std::to_string(i+1) + " to be start of a perl hash");
    }

    hash result;
    for (int index = i; index < items; index += 2)
    {
      const char* key = SvPV_nolen(ST(index));
      result[key] = SvREFCNT_inc(ST(index + 1));
    }
    return result;
  }
};

} // namespace stack
} // namespace perlbind
