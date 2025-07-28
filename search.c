#include <stdlib.h>
#include <wchar.h>
#include <wctype.h>
#include <ncursesw/curses.h>

#include "text.h"
#include "screen.h"
#include "buffer.h"
#include "search.h"

#include "config.h"
/* Extern globals from ee.c */
extern struct text *curr_line;
extern struct text *srch_line;
extern ee_char *point;
extern ee_char *srch_str;
extern ee_char *u_srch_str;
extern ee_char *srch_1;
extern ee_char *srch_2;
extern ee_char *srch_3;
extern int position;
extern int absolute_lin;
extern int scr_vert;
extern int scr_horz;
extern int scr_pos;
extern int horiz_offset;
extern int case_sen;
extern int last_line;
extern WINDOW *com_win;
extern WINDOW *text_win;
extern int gold;

/* helper from ee.c */
char *get_string(const char *prompt, int advance);

/* Functions provided elsewhere */
void right(int disp);
void move_rel(int direction, int lines);
void midscreen(int line, ee_char *pnt);

/* determine horizontal position for wide string */
int
scan_w(ee_char *line, int offset, int column)
{
    ee_char *stemp = line;
    int i = 0;
    int j = column;
    while (i < offset)
    {
        i++;
        j += len_char(*stemp, j);
        stemp++;
    }
    return j;
}

/* compare two strings */
int
compare(char *string1, char *string2, int sensitive)
{
    char *strng1 = string1;
    char *strng2 = string2;
    int equal;

    if ((strng1 == NULL) || (strng2 == NULL) || (*strng1 == '\0') || (*strng2 == '\0'))
        return FALSE;
    equal = TRUE;
    while (equal)
    {
        if (sensitive)
        {
            if (*strng1 != *strng2)
                equal = FALSE;
        }
        else
        {
            if (towupper(*strng1) != towupper(*strng2))
                equal = FALSE;
        }
        strng1++;
        strng2++;
        if ((*strng1 == '\0') || (*strng2 == '\0') || (*strng1 == ' ') || (*strng2 == ' '))
            break;
    }
    return equal;
}

void
goto_line(char *cmd_str)
{
    int number;
    int i;
    char *ptr;
    char direction = '\0';
    struct text *t_line;

    ptr = cmd_str;
    i = 0;
    while ((*ptr >= '0') && (*ptr <= '9'))
    {
        i = i * 10 + (*ptr - '0');
        ptr++;
    }
    number = i;
    i = 0;
    t_line = curr_line;
    while ((t_line->line_number > number) && (t_line->prev_line != NULL))
    {
        i++;
        t_line = t_line->prev_line;
        direction = 'u';
    }
    while ((t_line->line_number < number) && (t_line->next_line != NULL))
    {
        i++;
        direction = 'd';
        t_line = t_line->next_line;
    }
    if ((i < 30) && (i > 0))
    {
        move_rel(direction, i);
    }
    else
    {
        if (direction != 'd')
            absolute_lin += i;
        else
            absolute_lin -= i;
        curr_line = t_line;
        point = curr_line->line;
        position = 1;
        midscreen((last_line / 2), point);
        scr_pos = scr_horz;
    }
    wmove(com_win, 0, 0);
    wclrtoeol(com_win);
    wprintw(com_win, line_num_str, curr_line->line_number);
    wmove(text_win, scr_vert, (scr_horz - horiz_offset));
}

/* search for string in srch_str */
int
search(int display_message)
{
    int lines_moved;
    int iter;
    int found;

    if ((srch_str == NULL) || (*srch_str == '\0'))
        return FALSE;
    if (display_message)
    {
        wmove(com_win, 0, 0);
        wclrtoeol(com_win);
        wprintw(com_win, "%s", searching_msg);
        wrefresh(com_win);
    }
    lines_moved = 0;
    found = FALSE;
    srch_line = curr_line;
    srch_1 = point;
    if (position < curr_line->line_length)
        srch_1++;
    iter = position + 1;
    while ((!found) && (srch_line != NULL))
    {
        while ((iter < srch_line->line_length) && (!found))
        {
            srch_2 = srch_1;
            if (case_sen)
            {
                srch_3 = srch_str;
                while ((*srch_2 == *srch_3) && (*srch_3 != '\0'))
                {
                    found = TRUE;
                    srch_2++;
                    srch_3++;
                }
            }
            else
            {
                srch_3 = u_srch_str;
                while ((towupper((wint_t)*srch_2) == (wint_t)*srch_3) && (*srch_3 != '\0'))
                {
                    found = TRUE;
                    srch_2++;
                    srch_3++;
                }
            }
            if (!((*srch_3 == '\0') && (found)))
            {
                found = FALSE;
                if (iter < srch_line->line_length)
                    srch_1++;
                iter++;
            }
        }
        if (!found)
        {
            srch_line = srch_line->next_line;
            if (srch_line != NULL)
                srch_1 = srch_line->line;
            iter = 1;
            lines_moved++;
        }
    }
    if (found)
    {
        if (display_message)
        {
            wmove(com_win, 0, 0);
            wclrtoeol(com_win);
            wrefresh(com_win);
        }
        if (lines_moved == 0)
        {
            while (position < iter)
                right(TRUE);
        }
        else
        {
            if (lines_moved < 30)
            {
                move_rel('d', lines_moved);
                while (position < iter)
                    right(TRUE);
            }
            else
            {
                absolute_lin += lines_moved;
                curr_line = srch_line;
                point = srch_1;
                position = iter;
                scanline(point);
                scr_pos = scr_horz;
                midscreen((last_line / 2), point);
            }
        }
    }
    else
    {
        if (display_message)
        {
            wmove(com_win, 0, 0);
            wclrtoeol(com_win);
            wprintw(com_win, str_not_found_msg, srch_str);
            wrefresh(com_win);
        }
        wmove(text_win, scr_vert, (scr_horz - horiz_offset));
    }
    return found;
}

/* prompt and read search string (srch_str) */
void
search_prompt(void)
{
    if (srch_str != NULL)
        free(srch_str);
    if ((u_srch_str != NULL) && (*u_srch_str != '\0'))
        free(u_srch_str);
    char *tmp = get_string(search_prompt_str, FALSE);
    size_t slen = mbstowcs(NULL, tmp, 0);
    srch_str = malloc((slen + 1) * sizeof(ee_char));
    mbstowcs(srch_str, tmp, slen + 1);
    free(tmp);
    gold = FALSE;
    srch_3 = srch_str;
    srch_1 = u_srch_str = malloc((slen + 1) * sizeof(ee_char));
    while (*srch_3 != '\0')
    {
        *srch_1 = towupper(*srch_3);
        srch_1++;
        srch_3++;
    }
    *srch_1 = '\0';
    search(TRUE);
}

