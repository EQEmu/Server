#ifndef __OPENEQ_FILE__
#define __OPENEQ_FILE__

#include "archive.hpp"

int GetFile(uchar **buffer, int *buf_len, char *base_path, char *file_name, Archive *archive);

#endif
