#pragma once

constexpr int perlbind_version_major = 1;
constexpr int perlbind_version_minor = 1;
constexpr int perlbind_version_patch = 0;

constexpr int perlbind_version()
{
  return perlbind_version_major * 10000 + perlbind_version_minor * 100 + perlbind_version_patch;
}
