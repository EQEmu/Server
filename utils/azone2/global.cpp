#include "global.hpp"

#define swapl
#define swaps

uint16 GetUint16(uchar **buf) {
  uint16 ret = swaps(*((uint16 *) *buf));
  *buf += 2;
  return ret;
}

uint32 GetUint32(uchar **buf) {
  uint32 ret = swapl(*((uint32 *) *buf));
  *buf += 4;
  return ret;
}
