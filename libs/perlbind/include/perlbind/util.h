#pragma once

#include <string>
#include <typeinfo>
#ifndef _MSC_VER
#include <cxxabi.h>
#endif

namespace perlbind { namespace util {

inline std::string demangle(const char* name)
{
#ifndef _MSC_VER
  int status = 0;
  char* res = abi::__cxa_demangle(name, nullptr, nullptr, &status);
  if (res)
  {
    std::string demangled = res;
    free(res);
    return demangled;
  }
  return "<unknown>";
#else
  return name;
#endif
}

template <typename... Args>
struct type_name;

template <>
struct type_name<>
{
  static std::string str() { return "void"; }
};

template <typename T>
struct type_name<T>
{
  static std::string str() { return demangle(typeid(T).name()); }
};

template <typename T, typename... Args>
struct type_name<T, Args...>
{
  static std::string str() { return type_name<T>::str() + "," + type_name<Args...>::str(); }
};

} // namespace util
} // namespace perlbind
