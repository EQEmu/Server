#pragma once

#include <string>

namespace perlbind {

class package
{
public:
  virtual ~package() = default;
  package() = delete;
  package(PerlInterpreter* interp, const char* name)
    : my_perl(interp), m_name(name), m_stash(gv_stashpv(name, GV_ADD))
  {}

  // bind a function pointer to a function name in the package
  // overloads with same name must be explicit (default parameters not supported)
  // overloads have a runtime lookup cost and chooses the first compatible overload
  template <typename T>
  void add(const char* name, T func)
  {
    // ownership of function object is given to perl
    auto function = new detail::function<T>(my_perl, func);
    add_impl(name, static_cast<detail::function_base*>(function));
  }

  // specify a base class name for object inheritance (must be registered)
  // calling object methods missing from the package will search parent classes
  // base classes are searched in registered order and include any grandparents
  void add_base_class(const char* name)
  {
    std::string package_isa = m_name + "::ISA";
    AV* av = get_av(package_isa.c_str(), GV_ADD);
    array isa_array = reinterpret_cast<AV*>(SvREFCNT_inc(av));
    isa_array.push_back(name);
  }

  // add a constant value to this package namespace
  template <typename T>
  void add_const(const char* name, T&& value)
  {
    newCONSTSUB(m_stash, name, scalar(value).release());
  }

private:
  void add_impl(const char* name, detail::function_base* function);

  std::string m_name;
  PerlInterpreter* my_perl = nullptr;
  HV* m_stash = nullptr;
};

template <typename T>
struct class_ : public package
{
  using package::package;
};

} // namespace perlbind
