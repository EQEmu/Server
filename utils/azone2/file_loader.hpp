#ifndef __OPENEQ_FL_API__
#define __OPENEQ_FL_API__

#include "archive.hpp"
#include "3d.hpp"

class FileLoader {
public:
  FileLoader() {}
  virtual ~FileLoader() {}

  virtual int Open(char *base_path, char *zone_name, Archive *archive) = 0;
  virtual int Close() = 0;

  Content_3D model_data;
protected:
  uchar *buffer;
  int buf_len;
  Archive *archive;

  int status;
};

#endif
