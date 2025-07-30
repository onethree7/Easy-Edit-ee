#ifndef FILEIO_H
#define FILEIO_H

#include "text.h"

void get_file(char *file_name);
void get_line(int length, ee_char *in_string, int *append);
int write_file(char *file_name, int warn_if_exists);

#endif /* FILEIO_H */
