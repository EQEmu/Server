#include <perlbind/perlbind.h>
#include <perlbind/iterator.h>
#include <stdexcept>

namespace perlbind {

hash::hash(scalar ref)
  : type_base(ref.my_perl)
{
  if (!ref.is_hash_ref())
    throw std::runtime_error("cannot construct hash from non-hash reference");

  reset(reinterpret_cast<HV*>(SvREFCNT_inc(*ref)));
}

hash::hash(scalar_proxy proxy)
  : hash(scalar(SvREFCNT_inc(proxy.sv())))
{}

scalar hash::at(const char* key)
{
  return at(key, strlen(key));
}

scalar hash::at(const std::string& key)
{
  return at(key.c_str(), key.size());
}

scalar hash::at(const char* key, size_t size)
{
  SV** sv = hv_fetch(m_hv, key, static_cast<I32>(size), 1);
  return SvREFCNT_inc(*sv);
}

void hash::insert(const char* key, scalar value)
{
  insert(key, strlen(key), value);
}

void hash::insert(const std::string& key, scalar value)
{
  insert(key.c_str(), key.size(), value);
}

scalar_proxy hash::operator[](const std::string& key)
{
  return scalar_proxy(my_perl, at(key.c_str(), key.size()));
}

hash::iterator hash::begin() const noexcept
{
  hv_iterinit(m_hv);
  return { my_perl, m_hv, hv_iternext(m_hv) };
}

hash::iterator hash::end() const noexcept
{
  return { my_perl, m_hv, nullptr };
}

hash::iterator hash::find(const char* key)
{
  return find(key, static_cast<I32>(strlen(key)));
}

hash::iterator hash::find(const std::string& key)
{
  return find(key.c_str(), static_cast<I32>(key.size()));
}

hash::iterator hash::find(const char* key, size_t size)
{
  // key sv made mortal with SVs_TEMP flag
  SV* keysv = newSVpvn_flags(key, static_cast<I32>(size), SVs_TEMP);
  HE* he = hv_fetch_ent(m_hv, keysv, 0, 0);
  return { my_perl, m_hv, he };
}

void hash::insert(const char* key, size_t size, scalar value)
{
  if (!hv_store(m_hv, key, static_cast<I32>(size), SvREFCNT_inc(value), 0))
  {
    SvREFCNT_dec(value);
  }
}

HV* hash::copy_hash(HV* other) noexcept
{
  HV* hv = newHV();

  hv_iterinit(other);
  while (HE* entry = hv_iternext(other))
  {
    size_t key_size;
    auto key   = HePV(entry, key_size);
    auto value = newSVsv(HeVAL(entry));
    if (!hv_store(hv, key, static_cast<I32>(key_size), value, HeHASH(entry)))
    {
      SvREFCNT_dec(value);
    }
  }

  return hv;
}

} // namespace perlbind
