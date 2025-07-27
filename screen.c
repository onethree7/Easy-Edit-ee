#include <string.h>
#include <wctype.h>
#include <wchar.h>
#include <ncursesw/curses.h>
#include "screen.h"
#include "buffer.h"
#include "fileio.h"
#include "undo.h"

/* needed declarations from ee.c */
extern void scanline(ee_char *pos);
extern void set_up_term(void);

#define TAB 9
#define CONTROL_KEYS 1
#define COMMANDS 2
#ifndef max
#define max(a,b) ((a) > (b) ? (a) : (b))
#endif
#ifndef min
#define min(a,b) ((a) < (b) ? (a) : (b))
#endif

/* external globals from ee.c */
extern WINDOW *text_win;
extern WINDOW *com_win;
extern WINDOW *info_win;
extern int info_window;
extern int last_col;
extern int scr_vert;
extern int scr_horz;
extern int horiz_offset;
extern int clear_com_win;
extern int info_type;
extern char *separator;
extern char *control_keys[];
extern char *emacs_control_keys[];
extern char *command_strings[];
extern int nohighlight;
extern int emacs_keys_mode;
extern int local_LINES;
extern int local_COLS;
extern int eightbit;
extern int ee_chinese;
extern int last_line;
extern WINDOW *help_win;

/* tab shift for column alignment */
int tabshift(int temp_int)
{
    int leftover;

    leftover = ((temp_int + 1) % 8);
    if (leftover == 0)
        return 1;
    else
        return 9 - leftover;
}

/* output a printable or control character */
int out_char(WINDOW *window, ee_char character, int column)
{
    int i1, i2;
    char *string;
    char string2[16];

    if (character == TAB) {
        i1 = tabshift(column);
        for (i2 = 0; (i2 < i1) && (((column+i2+1)-horiz_offset) < last_col); i2++)
            waddch(window, ' ');
        return i1;
    } else if ((character >= L'\0') && (character < L' ')) {
        extern char *table[];
        string = table[(int) character];
    } else if (!iswprint(character)) {
        if (character == 127)
            string = "^?";
        else if (!eightbit) {
            snprintf(string2, sizeof(string2), "<%x>", (unsigned int)character);
            string = string2;
        } else {
            waddnwstr(window, &character, 1);
            return 1;
        }
    } else {
        waddnwstr(window, &character, 1);
        i1 = wcwidth(character);
        return i1 > 0 ? i1 : 1;
    }
    for (i2 = 0; (string[i2] != '\0') && (((column+i2+1)-horiz_offset) < last_col); i2++)
        waddch(window, (unsigned char)string[i2]);
    return strlen(string);
}

/* return display width of character */
int len_char(ee_char character, int column)
{
    int length;

    if (character == '\t')
        length = tabshift(column);
    else {
        int w = wcwidth(character);
        if (w > 0)
            length = w;
        else if ((character >= 0 && character < 32) || character == 127)
            length = 2;
        else if (!eightbit)
            length = 5;
        else
            length = 1;
    }
    return length;
}

/* redraw a line from the buffer */
void draw_line(int vertical, int horiz, ee_char *ptr, int t_pos, int length)
{
    int d;
    ee_char *temp;
    int abs_column;
    int column;
    int row;
    int posit;

    abs_column = horiz;
    column = horiz - horiz_offset;
    row = vertical;
    temp = ptr;
    d = 0;
    posit = t_pos;
    if (column < 0) {
        wmove(text_win, row, 0);
        wclrtoeol(text_win);
    }
    while (column < 0) {
        d = len_char(*temp, abs_column);
        abs_column += d;
        column += d;
        posit++;
        temp++;
    }
    wmove(text_win, row, column);
    wclrtoeol(text_win);
    while ((posit < length) && (column <= last_col)) {
        if (!iswprint(*temp)) {
            column += len_char(*temp, abs_column);
            abs_column += out_char(text_win, *temp, abs_column);
        } else {
            abs_column += wcwidth(*temp);
            column += wcwidth(*temp);
            waddnwstr(text_win, temp, 1);
        }
        posit++;
        temp++;
    }
    if (column < last_col)
        wclrtoeol(text_win);
    wmove(text_win, vertical, (horiz - horiz_offset));
}

void paint_info_win(void)
{
    int counter;

    if (!info_window)
        return;

    werase(info_win);
    for (counter = 0; counter < 5; counter++) {
        wmove(info_win, counter, 0);
        wclrtoeol(info_win);
        if (info_type == CONTROL_KEYS)
            waddstr(info_win, (emacs_keys_mode) ?
                   emacs_control_keys[counter] : control_keys[counter]);
        else if (info_type == COMMANDS)
            waddstr(info_win, command_strings[counter]);
    }
    wmove(info_win, 5, 0);
    if (!nohighlight)
        wstandout(info_win);
    waddstr(info_win, separator);
    wstandend(info_win);
    wrefresh(info_win);
}

void no_info_window(void)
{
    if (!info_window)
        return;
    delwin(info_win);
    delwin(text_win);
    info_window = FALSE;
    last_line = LINES - 2;
    text_win = newwin((LINES - 1), COLS, 0, 0);
    keypad(text_win, TRUE);
    idlok(text_win, TRUE);
    clearok(text_win, TRUE);
    midscreen(scr_vert, point);
    wrefresh(text_win);
    clear_com_win = TRUE;
}

void create_info_window(void)
{
    if (info_window)
        return;
    last_line = LINES - 8;
    delwin(text_win);
    text_win = newwin((LINES - 7), COLS, 6, 0);
    keypad(text_win, TRUE);
    idlok(text_win, TRUE);
    werase(text_win);
    info_window = TRUE;
    info_win = newwin(6, COLS, 0, 0);
    werase(info_win);
    info_type = CONTROL_KEYS;
    midscreen(min(scr_vert, last_line), point);
    clearok(info_win, TRUE);
    paint_info_win();
    wrefresh(text_win);
    clear_com_win = TRUE;
}

void midscreen(int line, ee_char *pnt)
{
    struct text *mid_line;
    int i;

    line = min(line, last_line);
    mid_line = curr_line;
    for (i = 0; ((i < line) && (curr_line->prev_line != NULL)); i++)
        curr_line = curr_line->prev_line;
    scr_vert = scr_horz = 0;
    wmove(text_win, 0, 0);
    draw_screen();
    scr_vert = i;
    curr_line = mid_line;
    scanline(pnt);
    wmove(text_win, scr_vert, (scr_horz - horiz_offset));
}

void draw_screen(void)
{
    struct text *temp_line;
    ee_char *line_out;
    int temp_vert;

    temp_line = curr_line;
    temp_vert = scr_vert;
    wclrtobot(text_win);
    while ((temp_line != NULL) && (temp_vert <= last_line)) {
        line_out = temp_line->line;
        draw_line(temp_vert, 0, line_out, 1, temp_line->line_length);
        temp_vert++;
        temp_line = temp_line->next_line;
    }
    wmove(text_win, temp_vert, 0);
    wmove(text_win, scr_vert, (scr_horz - horiz_offset));
}

void redraw(void)
{
    if (info_window) {
        clearok(info_win, TRUE);
        paint_info_win();
    } else
        clearok(text_win, TRUE);
    midscreen(scr_vert, point);
}

void resize_check(void)
{
    if ((LINES == local_LINES) && (COLS == local_COLS))
        return;

    if (info_window)
        delwin(info_win);
    delwin(text_win);
    delwin(com_win);
    delwin(help_win);
    set_up_term();
    redraw();
    wrefresh(text_win);
}

