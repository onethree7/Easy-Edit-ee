#ifndef SCREEN_H
#define SCREEN_H

#include <ncursesw/curses.h>
#include "text.h"

int tabshift(int temp_int);
int out_char(WINDOW *window, ee_char character, int column);
int len_char(ee_char character, int column);
void draw_line(int vertical, int horiz, ee_char *ptr, int t_pos, int length);
void paint_info_win(void);
void no_info_window(void);
void create_info_window(void);
void midscreen(int line, ee_char *pnt);
void resize_check(void);
void redraw(void);
void draw_screen(void);
void scanline(ee_char *pos);

#endif /* SCREEN_H */
