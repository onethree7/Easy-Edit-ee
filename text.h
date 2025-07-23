#ifndef TEXT_H
#define TEXT_H

#include <wchar.h>

typedef wchar_t ee_char;

struct text {
    ee_char *line;
    int line_number;
    int line_length;
    int max_length;
    struct text *next_line;
    struct text *prev_line;
};

extern struct text *first_line;
extern struct text *dlt_line;
extern struct text *curr_line;
extern struct text *tmp_line;
extern struct text *srch_line;
extern ee_char *point;
extern int position;
extern int absolute_lin;
extern int scr_vert;
extern int scr_horz;
extern int horiz_offset;

struct text *txtalloc(void);

#endif /* TEXT_H */
