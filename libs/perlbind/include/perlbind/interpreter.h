#pragma once

namespace perlbind {

class interpreter
{
public:
  interpreter();
  interpreter(PerlInterpreter* interp) : my_perl(interp) {}
  interpreter(int argc, const char** argv);
  interpreter(const interpreter& other) = delete;
  interpreter(interpreter&& other) = delete;
  interpreter& operator=(const interpreter& other) = delete;
  interpreter& operator=(interpreter&& other) = delete;
  ~interpreter();

  PerlInterpreter* get() const { return my_perl; }

  void load_script(std::string packagename, std::string filename);
  void eval(const char* str);

  template <typename T, typename... Args>
  T call_sub(const char* subname, Args&&... args) const
  {
    detail::sub_caller caller(my_perl);
    return caller.call_sub<T>(subname, std::forward<Args>(args)...);
  }

  // returns interface to add bindings to package name
  package new_package(const char* name)
  {
    return package(my_perl, name);
  }

  // registers type for blessing objects, returns interface
  template <typename T>
  class_<T> new_class(const char* name)
  {
    static_assert(!std::is_pointer<T>::value && !std::is_reference<T>::value,
                  "new_class<T> 'T' should not be a pointer or reference");

    auto typemap = detail::typemap::get(my_perl);
    auto type_id = detail::usertype<T*>::id();
    typemap[type_id] = name;

    return class_<T>(my_perl, name);
  }

  // helper to bind functions in default main:: package
  template <typename T>
  void add(const char* name, T&& func)
  {
    new_package("main").add(name, std::forward<T>(func));
  }

private:
  void init(int argc, const char** argv);

  bool m_is_owner = false;
  PerlInterpreter* my_perl = nullptr;
};

} // namespace perlbind
