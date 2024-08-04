#pragma once

namespace perlbind { namespace detail {

struct array_iterator
{
  array_iterator() = default;
  array_iterator(PerlInterpreter* interp, AV* av, size_t index)
    : my_perl(interp), m_av(av), m_index(index), m_scalar(interp)
  {
    fetch();
  }

  bool operator!=(const array_iterator& other) const
  {
    return m_index != other.m_index;
  }

  array_iterator& operator++()
  {
    ++m_index;
    fetch();
    return *this;
  }

  scalar* operator->()
  {
    return &m_scalar;
  }

  scalar& operator*()
  {
    return m_scalar;
  }

private:
  void fetch()
  {
    SV** sv = av_fetch(m_av, m_index, 0);
    if (sv)
      m_scalar = SvREFCNT_inc(*sv);
  }

  PerlInterpreter* my_perl;
  AV* m_av;
  size_t m_index;
  scalar m_scalar;
};

struct hash_iterator
{
  hash_iterator() = default;
  hash_iterator(PerlInterpreter* interp, HV* hv, HE* he)
    : my_perl(interp), m_hv(hv), m_he(he)
  {
    fetch();
  }

  bool operator==(const hash_iterator& other) const
  {
    return m_he == other.m_he;
  }

  bool operator!=(const hash_iterator& other) const
  {
    return !(*this == other);
  }

  hash_iterator& operator++()
  {
    m_he = hv_iternext(m_hv);
    fetch();
    return *this;
  }

  std::pair<const char*, scalar>* operator->()
  {
    return &m_pair;
  }

  std::pair<const char*, scalar>& operator*()
  {
    return m_pair;
  }

private:
  void fetch()
  {
    if (m_he)
      m_pair = { HePV(m_he, PL_na), scalar(my_perl, SvREFCNT_inc(HeVAL(m_he))) };
  }

  PerlInterpreter* my_perl;
  HV* m_hv;
  HE* m_he;
  std::pair<const char*, scalar> m_pair;
};

} // namespace detail
} // namespace perlbind
