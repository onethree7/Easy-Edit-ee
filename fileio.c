#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <wchar.h>
#include <wctype.h>
#include <ncursesw/curses.h>
#include "buffer.h"
#include "fileio.h"

#include "config.h"
/* extern globals from ee.c */
extern int recv_file;
extern int input_file;
extern int text_changes;
extern int get_fd;
extern char *in_file_name;
extern char in_string[513];
extern WINDOW *com_win;
extern WINDOW *text_win;
extern int scr_vert;
extern int scr_horz;
extern int horiz_offset;
extern struct text *curr_line;
extern struct text *first_line;
extern ee_char *point;
extern int clear_com_win;
extern int in;
extern FILE *temp_fp;

/* functions from ee.c */
extern void insert_line(int disp);
extern void left(int disp);
extern ee_char *resiz_line(int factor, struct text *rline, int rpos);
extern char *get_string(const char *prompt, int advance);

void get_file(char *file_name)
{
    int can_read;
    int length;
    int append;
    struct text *temp_line;
    char ro_flag = FALSE;

    if (recv_file)
    {
        wmove(com_win, 0, 0);
        wclrtoeol(com_win);
        wprintw(com_win, reading_file_msg, file_name);
        if (access(file_name, 2))
        {
            if ((errno == ENOTDIR) || (errno == EACCES) || (errno == EROFS) ||
                (errno == ETXTBSY) || (errno == EFAULT))
            {
                wprintw(com_win, "%s", read_only_msg);
                ro_flag = TRUE;
            }
        }
        wrefresh(com_win);
    }
    if (curr_line->line_length > 1)
    {
        insert_line(FALSE);
        left(FALSE);
        append = FALSE;
    }
    else
        append = TRUE;
    can_read = FALSE;
    ee_char wbuf[513];
    while (((length = read(get_fd, in_string, 512)) != 0) && (length != -1))
    {
        can_read = TRUE;
        in_string[length] = '\0';
        int wlen = mbstowcs(wbuf, in_string, 513);
        if (wlen < 0)
            wlen = 0;
        get_line(wlen, wbuf, &append);
    }
    if ((can_read) && (curr_line->line_length == 1))
    {
        temp_line = curr_line->prev_line;
        temp_line->next_line = curr_line->next_line;
        if (temp_line->next_line != NULL)
            temp_line->next_line->prev_line = temp_line;
        if (curr_line->line != NULL)
            free(curr_line->line);
        free(curr_line);
        curr_line = temp_line;
    }
    if (input_file)
    {
        wmove(com_win, 0, 0);
        wclrtoeol(com_win);
        wprintw(com_win, file_read_lines_msg, in_file_name, curr_line->line_number);
        if (ro_flag)
            wprintw(com_win, "%s", read_only_msg);
        wrefresh(com_win);
    }
    else if (can_read)
        text_changes = TRUE;

    if (recv_file)
    {
        in = EOF;
    }
}

void get_line(int length, ee_char *in_string, int *append)
{
    ee_char *str1;
    ee_char *str2;
    int num;
    int char_count;
    int temp_counter;
    struct text *tline;
    int first_time;

    str2 = in_string;
    num = 0;
    first_time = TRUE;
    while (num < length)
    {
        if (!first_time)
        {
            if (num < length)
            {
                str2++;
                num++;
            }
        }
        else
            first_time = FALSE;
        str1 = str2;
        char_count = 1;
        while ((*str2 != '\n') && (num < length))
        {
            str2++;
            num++;
            char_count++;
        }
        if (!(*append))
        {
            tline = txtalloc();
            tline->line_number = curr_line->line_number + 1;
            tline->next_line = curr_line->next_line;
            tline->prev_line = curr_line;
            curr_line->next_line = tline;
            if (tline->next_line != NULL)
                tline->next_line->prev_line = tline;
            curr_line = tline;
            curr_line->line = point = malloc(char_count * sizeof(ee_char));
            curr_line->line_length = char_count;
            curr_line->max_length = char_count;
        }
        else
        {
            point = resiz_line(char_count, curr_line, curr_line->line_length);
            curr_line->line_length += (char_count - 1);
        }
        for (temp_counter = 1; temp_counter < char_count; temp_counter++)
        {
            *point = *str1;
            point++;
            str1++;
        }
        *point = '\0';
        *append = FALSE;
        if ((num == length) && (*str2 != '\n'))
            *append = TRUE;
    }
}

int write_file(char *file_name, int warn_if_exists)
{
    char cr;
    char *tmp_point;
    struct text *out_line;
    int lines, charac;
    int write_flag = TRUE;

    charac = lines = 0;
    if (warn_if_exists &&
        ((in_file_name == NULL) || strcmp(in_file_name, file_name)))
    {
        if ((temp_fp = fopen(file_name, "r")))
        {
            tmp_point = get_string(file_exists_prompt, TRUE);
            if (towupper(*tmp_point) == towupper(*yes_char))
                write_flag = TRUE;
            else
                write_flag = FALSE;
            fclose(temp_fp);
            free(tmp_point);
        }
    }

    clear_com_win = TRUE;

    if (write_flag)
    {
        if ((temp_fp = fopen(file_name, "w")) == NULL)
        {
            clear_com_win = TRUE;
            wmove(com_win,0,0);
            wclrtoeol(com_win);
            wprintw(com_win, create_file_fail_msg, file_name);
            wrefresh(com_win);
            return(FALSE);
        }
        else
        {
            wmove(com_win,0,0);
            wclrtoeol(com_win);
            wprintw(com_win, writing_file_msg, file_name);
            wrefresh(com_win);
            cr = '\n';
            out_line = first_line;
            while (out_line != NULL)
            {
                size_t bytes;
                char mbbuf[4096];
                bytes = wcstombs(mbbuf, out_line->line, sizeof(mbbuf));
                if (bytes == (size_t)-1)
                    bytes = 0;
                fwrite(mbbuf, 1, bytes, temp_fp);
                charac += bytes;
                out_line = out_line->next_line;
                putc(cr, temp_fp);
                lines++;
            }
            fclose(temp_fp);
            wmove(com_win,0,0);
            wclrtoeol(com_win);
            wprintw(com_win, file_written_msg, file_name, lines, charac);
            wrefresh(com_win);
            return(TRUE);
        }
    }
    else
        return(FALSE);
}
