#include <stdio.h>
#include <string.h>
#include "file.hpp"

int GetFile(uchar **buffer, int *buf_len, char *base_path, char *file_name, Archive *archive) {
  FILE *fp;
  char *temp;

  if(base_path) {
    temp = new char[strlen(base_path) + strlen(file_name) + 2];
    sprintf(temp, "%s/%s", base_path, file_name);

    fp = fopen(temp, "rb");

    delete[] temp;

    if(!fp)
      return 0;

    fseek(fp, 0, SEEK_END);
    *buf_len = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    *buffer = new uchar[*buf_len];
    fread(*buffer, 1, *buf_len, fp);

    fclose(fp);
    return 1;
  }
  else
    return archive->GetFile(file_name, buffer, buf_len);
}
