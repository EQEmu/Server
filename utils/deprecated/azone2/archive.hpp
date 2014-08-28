#ifndef __OPENEQ_ARCHIVE_API__
#define __OPENEQ_ARCHIVE_API__

#include "global.hpp"

#include <stdio.h>

class Archive {
public:
  Archive() {}
  virtual ~Archive() {}

  virtual int Open(FILE *fp) = 0;
  virtual int Close() = 0;

  virtual int GetFile(char *name, uchar **buf, int *len) = 0;
  virtual const char *FindExtension(const char *ext) = 0;

  char **filenames;
  int count;

protected:
  uchar *buffer;
  int buf_len;

  int status;

  uint32 *files;
  FILE *fp;
};

#endif
