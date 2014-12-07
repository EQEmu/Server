#ifndef __EQCLIENT_S3D_H_
#define __EQCLIENT_S3D_H_
#include <stdio.h>
#include "types.h"

typedef struct struct_header {
  uint32 offset;
  char magicCookie[4];
  uint32 unknown;
} struct_header;

typedef struct struct_directory_header {
  uint32 count;
} struct_directory_header;

typedef struct struct_directory {
  uint32 crc, offset, size;
} struct_directory;

typedef struct struct_data_block {
  uint32 deflen, inflen;
} struct_data_block;

typedef struct struct_fn_header {
  uint32 fncount;
} struct_fn_header;

typedef struct struct_fn_entry {
  uint32 fnlen;
} struct_fn_entry;

typedef struct s3d_object {
  FILE *fp;
  long count;
  char **filenames;
  uint32 *files;
} s3d_object;

#ifdef __cplusplus
extern "C" {
#endif

int S3D_Init(s3d_object *obj, FILE *fp);
size_t S3D_GetFile(s3d_object *obj, char *filename, uchar **out);

#ifdef __cplusplus
}
#endif


#endif
