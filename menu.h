#ifndef MENU_H
#define MENU_H

#include <ncursesw/curses.h>

struct menu_entries {
    char *item_string;
    int (*procedure)(struct menu_entries *);
    struct menu_entries *ptr_argument;
    int (*iprocedure)(int);
    void (*nprocedure)(void);
    int argument;
};

extern struct menu_entries modes_menu[];
extern char *mode_strings[11];

void paint_menu(struct menu_entries menu_list[], int max_width,
                int max_height, int list_size, int top_offset,
                WINDOW *menu_win, int off_start, int vert_size);
int menu_op(struct menu_entries menu_list[]);
void modes_op(void);
void help(void);

#endif /* MENU_H */
