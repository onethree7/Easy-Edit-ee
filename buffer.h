#ifndef BUFFER_H
#define BUFFER_H

#include "text.h"

struct files {
    char *name;
    struct files *next_name;
};

struct text *txtalloc(void);
struct files *name_alloc(void);
ee_char *resiz_line(int factor, struct text *rline, int rpos);
ee_char *next_word(ee_char *string);
void prev_word(void);
char *next_ascii_word(char *string);

#endif /* BUFFER_H */
