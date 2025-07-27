#include <stdlib.h>
#include <stdio.h>
#include <wchar.h>
#include "buffer.h"
#ifndef TRUE
#define TRUE 1
#endif

/* resize the line to length + factor */
ee_char *resiz_line(int factor, struct text *rline, int rpos)
{
    ee_char *rpoint;
    int resiz_var;

    rline->max_length += factor;
    rpoint = rline->line = realloc(rline->line, rline->max_length * sizeof(ee_char));
    for (resiz_var = 1 ; (resiz_var < rpos) ; resiz_var++)
        rpoint++;
    return rpoint;
}

/* allocate space for line structure */
struct text *txtalloc(void)
{
    struct text *t = malloc(sizeof(struct text));
    if (!t) {
        perror("malloc");
        exit(1);
    }
    return t;
}

/* allocate space for file name list node */
struct files *name_alloc(void)
{
    struct files *f = malloc(sizeof(struct files));
    if (!f) {
        perror("malloc");
        exit(1);
    }
    return f;
}

/* move to next word in string */
ee_char *next_word(ee_char *string)
{
    while ((*string != '\0') && ((*string != 32) && (*string != 9)))
        string++;
    while ((*string != '\0') && ((*string == 32) || (*string == 9)))
        string++;
    return string;
}

/* move to next word in an ASCII string */
char *next_ascii_word(char *string)
{
    while ((*string != '\0') && ((*string != ' ') && (*string != '\t')))
        string++;
    while ((*string != '\0') && ((*string == ' ') || (*string == '\t')))
        string++;
    return string;
}

/* move to start of previous word in text */
void prev_word(void)
{
    extern int position;       /* globals from ee.c */
    extern ee_char *point;
    void left(int disp);       /* functions still in ee.c */
    void right(int disp);

    if (position != 1)
    {
        if ((position != 1) && ((point[-1] == ' ') || (point[-1] == '\t')))
        {
            while ((position != 1) && ((*point != ' ') && (*point != '\t')))
                left(TRUE);
        }
        while ((position != 1) && ((*point == ' ') || (*point == '\t')))
            left(TRUE);
        while ((position != 1) && ((*point != ' ') && (*point != '\t')))
            left(TRUE);
        if ((position != 1) && ((*point == ' ') || (*point == '\t')))
            right(TRUE);
    }
    else
        left(TRUE);
}
