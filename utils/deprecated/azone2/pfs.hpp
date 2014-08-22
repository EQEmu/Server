#ifndef __OPENEQ_PFS__
#define __OPENEQ_PFS__

#include <stdio.h>
#include "global.hpp"
#include "archive.hpp"


class PFSLoader : public Archive {
public:
  PFSLoader();
  ~PFSLoader();

  virtual int Open(FILE *fp);
  virtual int Close();

  virtual int GetFile(char *name, uchar **buf, int *len);
  virtual const char *FindExtension(const char *ext);
};

#endif
