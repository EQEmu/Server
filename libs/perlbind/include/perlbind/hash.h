#pragma once

#include "types.h"
#include <string>

namespace perlbind {

struct hash : public type_base
{
  using iterator = detail::hash_iterator;

  ~hash() noexcept
  {
    SvREFCNT_dec(m_hv);
  }

  hash() noexcept
    : type_base(), m_hv(newHV()) {}
  hash(PerlInterpreter* interp) noexcept
    : type_base(interp), m_hv(newHV()) {}
  hash(const hash& other) noexcept
    : type_base(other.my_perl), m_hv(copy_hash(other.m_hv)) {}
  hash(hash&& other) noexcept
    : type_base(other.my_perl), m_hv(other.m_hv)
  {
    other.m_hv = newHV();
  }
  hash(HV*& value) noexcept
    : type_base(), m_hv(copy_hash(value)) {}
  hash(HV*&& value) noexcept
    : type_base(), m_hv(value) {} // take ownership
  hash(scalar ref);
  hash(scalar_proxy proxy);

  hash& operator=(const hash& other) noexcept
  {
    if (this != &other)
      m_hv = copy_hash(other.m_hv);

    return *this;
  }

  hash& operator=(hash&& other) noexcept
  {
    if (this != &other)
      std::swap(m_hv, other.m_hv);

    return *this;
  }

  hash& operator=(HV*& value) noexcept
  {
    if (m_hv != value)
      m_hv = copy_hash(value);

    return *this;
  }

  hash& operator=(HV*&& value) noexcept
  {
    reset(value);
    return *this;
  }

  operator HV*() const { return m_hv; }
  operator SV*() const { return reinterpret_cast<SV*>(m_hv); }

  HV* release() noexcept
  {
    HV* tmp = m_hv;
    m_hv = newHV();
    return tmp;
  }

  void reset(HV* value) noexcept
  {
    SvREFCNT_dec(m_hv);
    m_hv = value;
  }

  scalar at(const char* key);
  scalar at(const std::string& key);
  void clear() noexcept { hv_clear(m_hv); }
  bool exists(const char* key) const
  {
    return hv_exists(m_hv, key, static_cast<I32>(strlen(key)));
  }
  bool exists(const std::string& key) const
  {
    return hv_exists(m_hv, key.c_str(), static_cast<I32>(key.size()));
  }
  void insert(const char* key, scalar value);
  void insert(const std::string& key, scalar value);
  void remove(const char* key)
  {
    hv_delete(m_hv, key, static_cast<I32>(strlen(key)), 0);
  }
  void remove(const std::string& key)
  {
    hv_delete(m_hv, key.c_str(), static_cast<I32>(key.size()), 0);
  }
  size_t size() const { return HvTOTALKEYS(m_hv); }
  SV* sv() const { return reinterpret_cast<SV*>(m_hv); }

  // returns a proxy that takes ownership of one reference to the SV value
  // creates an undef SV entry for the key if it doesn't exist
  scalar_proxy operator[](const std::string& key);

  iterator begin() const noexcept;
  iterator end() const noexcept;
  iterator find(const char* key);
  iterator find(const std::string& key);

private:
  scalar at(const char* key, size_t size);
  iterator find(const char* key, size_t size);
  void insert(const char* key, size_t size, scalar value);

  HV* copy_hash(HV* other) noexcept;

  HV* m_hv = nullptr;
};

} // namespace perlbind
