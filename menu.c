#define _XOPEN_SOURCE 600
#define _XOPEN_SOURCE_EXTENDED 1
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <ncursesw/curses.h>

#include "menu.h"

#include "config.h"
void nc_setattrib(int);
void nc_clearattrib(int);


/* globals from ee.c */
extern int in;
extern WINDOW *com_win;
extern WINDOW *help_win;
extern WINDOW *text_win;
extern int clear_com_win;
extern void paint_info_win(void);
extern void redraw(void);

/* helpers from ee.c */
char *get_string(const char *prompt, int advance);
void dump_ee_conf(void);
void no_info_window(void);
void create_info_window(void);

/* local helper */
static char item_alpha[] = "abcdefghijklmnopqrstuvwxyz0123456789 ";
#define MENU_WARN 1
#define max_alpha_char 36

struct menu_entries modes_menu[] = {
    {"", NULL, NULL, NULL, NULL, 0},
    {"", NULL, NULL, NULL, NULL, -1},
    {"", NULL, NULL, NULL, NULL, -1},
    {"", NULL, NULL, NULL, NULL, -1},
    {"", NULL, NULL, NULL, NULL, -1},
    {"", NULL, NULL, NULL, NULL, -1},
    {"", NULL, NULL, NULL, NULL, -1},
    {"", NULL, NULL, NULL, NULL, -1},
    {"", NULL, NULL, NULL, NULL, -1},
    {"", NULL, NULL, NULL, NULL, -1},
    {"", NULL, NULL, NULL, dump_ee_conf, -1},
    {NULL, NULL, NULL, NULL, NULL, -1}
};

char *mode_strings[11];

int menu_op(struct menu_entries menu_list[])
{
    WINDOW *temp_win;
    int max_width, max_height;
    int x_off, y_off;
    int counter;
    int length;
    int input;
    int temp = 0;
    int list_size;
    int top_offset;
    int vert_size;
    int off_start = 1;

    list_size = 1;
    while (menu_list[list_size + 1].item_string != NULL)
        list_size++;
    max_width = 0;
    for (counter = 0; counter <= list_size; counter++) {
        if ((length = strlen(menu_list[counter].item_string)) > max_width)
            max_width = length;
    }
    max_width += 3;
    max_width = max(max_width, (int)strlen(menu_cancel_msg));
    max_width = max(max_width, max((int)strlen(more_above_str),
                                   (int)strlen(more_below_str)));
    max_width += 6;

    if (max_width > COLS) {
        wmove(com_win, 0, 0);
        werase(com_win);
        wprintw(com_win, "%s", menu_too_lrg_msg);
        wrefresh(com_win);
        clear_com_win = TRUE;
        return 0;
    }

    top_offset = 0;

    if (list_size > LINES) {
        max_height = LINES;
        vert_size = (max_height > 11) ? max_height - 8 : max_height;
    } else {
        vert_size = list_size;
        max_height = list_size;
    }

    if (LINES >= (vert_size + 8)) {
        max_height = vert_size + ((menu_list[0].argument != MENU_WARN) ? 8 : 7);
        top_offset = 4;
    }
    x_off = (COLS - max_width) / 2;
    y_off = (LINES - max_height - 1) / 2;
    temp_win = newwin(max_height, max_width, y_off, x_off);
    keypad(temp_win, TRUE);

    paint_menu(menu_list, max_width, max_height, list_size, top_offset,
               temp_win, off_start, vert_size);

    counter = 1;
    do {
        if (off_start > 2)
            wmove(temp_win, 1 + counter + top_offset - off_start, 3);
        else
            wmove(temp_win, counter + top_offset - off_start, 3);

        wrefresh(temp_win);
        in = wgetch(temp_win);
        input = in;
        if (input == -1)
            exit(0);

        if (isascii(input) && isalnum(input)) {
            if (isalpha(input))
                temp = 1 + tolower(input) - 'a';
            else if (isdigit(input))
                temp = (2 + 'z' - 'a') + (input - '0');

            if (temp <= list_size) {
                input = '\n';
                counter = temp;
            }
        } else {
            switch (input) {
            case ' ':
            case '\004':
            case KEY_RIGHT:
            case KEY_DOWN:
                counter++;
                if (counter > list_size)
                    counter = 1;
                break;
            case '\010':
            case '\025':
            case 127:
            case KEY_BACKSPACE:
            case KEY_LEFT:
            case KEY_UP:
                counter--;
                if (counter == 0)
                    counter = list_size;
                break;
            case '\033':
                if (menu_list[0].argument != MENU_WARN)
                    counter = 0;
                break;
            case '\014':
            case '\022':
                paint_menu(menu_list, max_width, max_height, list_size,
                           top_offset, temp_win, off_start, vert_size);
                break;
            default:
                break;
            }
        }

        if (((list_size - off_start) >= (vert_size - 1)) &&
            (counter > (off_start + vert_size - 3)) &&
            (off_start > 1)) {
            if (counter == list_size)
                off_start = (list_size - vert_size) + 2;
            else
                off_start++;

            paint_menu(menu_list, max_width, max_height, list_size,
                       top_offset, temp_win, off_start, vert_size);
        } else if ((list_size != vert_size) &&
                   (counter > (off_start + vert_size - 2))) {
            if (counter == list_size)
                off_start = 2 + (list_size - vert_size);
            else if (off_start == 1)
                off_start = 3;
            else
                off_start++;

            paint_menu(menu_list, max_width, max_height, list_size,
                       top_offset, temp_win, off_start, vert_size);
        } else if (counter < off_start) {
            if (counter <= 2)
                off_start = 1;
            else
                off_start = counter;

            paint_menu(menu_list, max_width, max_height, list_size,
                       top_offset, temp_win, off_start, vert_size);
        }
    } while ((input != '\r') && (input != '\n') && (counter != 0));

    werase(temp_win);
    wrefresh(temp_win);
    delwin(temp_win);

    if (menu_list[counter].procedure || menu_list[counter].iprocedure ||
        menu_list[counter].nprocedure) {
        if (menu_list[counter].argument != -1)
            (*menu_list[counter].iprocedure)(menu_list[counter].argument);
        else if (menu_list[counter].ptr_argument)
            (*menu_list[counter].procedure)(menu_list[counter].ptr_argument);
        else
            (*menu_list[counter].nprocedure)();
    }

    if (info_window)
        paint_info_win();
    redraw();

    return counter;
}

void paint_menu(struct menu_entries menu_list[], int max_width, int max_height,
                int list_size, int top_offset, WINDOW *menu_win, int off_start,
                int vert_size)
{
    int counter, temp_int;

    werase(menu_win);

    if (max_height > vert_size) {
        wmove(menu_win, 1, 1);
        if (!nohighlight)
            wstandout(menu_win);
        waddch(menu_win, '+');
        for (counter = 0; counter < (max_width - 4); counter++)
            waddch(menu_win, '-');
        waddch(menu_win, '+');

        wmove(menu_win, max_height - 2, 1);
        waddch(menu_win, '+');
        for (counter = 0; counter < (max_width - 4); counter++)
            waddch(menu_win, '-');
        waddch(menu_win, '+');
        wstandend(menu_win);
        wmove(menu_win, 2, 3);
        waddstr(menu_win, menu_list[0].item_string);
        wmove(menu_win, max_height - 3, 3);
        if (menu_list[0].argument != MENU_WARN)
            waddstr(menu_win, menu_cancel_msg);
    }
    if (!nohighlight)
        wstandout(menu_win);

    for (counter = 0; counter < vert_size + top_offset; counter++) {
        temp_int = (top_offset == 4) ? counter + 2 : counter;
        wmove(menu_win, temp_int, 1);
        waddch(menu_win, '|');
        wmove(menu_win, temp_int, max_width - 2);
        waddch(menu_win, '|');
    }
    wstandend(menu_win);

    if (list_size > vert_size) {
        if (off_start >= 3) {
            temp_int = 1;
            wmove(menu_win, top_offset, 3);
            waddstr(menu_win, more_above_str);
        } else
            temp_int = 0;

        for (counter = off_start;
             (temp_int + counter - off_start) < (vert_size - 1); counter++) {
            wmove(menu_win, top_offset + temp_int + (counter - off_start), 3);
            if (list_size > 1)
                wprintw(menu_win, "%c) ",
                        item_alpha[min((counter - 1), max_alpha_char)]);
            waddstr(menu_win, menu_list[counter].item_string);
        }

        wmove(menu_win, top_offset + vert_size - 1, 3);

        if (counter == list_size) {
            if (list_size > 1)
                wprintw(menu_win, "%c) ",
                        item_alpha[min((counter - 1), max_alpha_char)]);
            wprintw(menu_win, "%s", menu_list[counter].item_string);
        } else
            wprintw(menu_win, "%s", more_below_str);
    } else {
        for (counter = 1; counter <= list_size; counter++) {
            wmove(menu_win, top_offset + counter - 1, 3);
            if (list_size > 1)
                wprintw(menu_win, "%c) ",
                        item_alpha[min((counter - 1), max_alpha_char)]);
            waddstr(menu_win, menu_list[counter].item_string);
        }
    }
}

void help(void)
{
    int counter;

    werase(help_win);
    clearok(help_win, TRUE);
    for (counter = 0; counter < 22; counter++) {
        wmove(help_win, counter, 0);
        waddstr(help_win,
                emacs_keys_mode ? emacs_help_text[counter] : help_text[counter]);
    }
    wrefresh(help_win);
    werase(com_win);
    wmove(com_win, 0, 0);
    wprintw(com_win, "%s", press_any_key_msg);
    wrefresh(com_win);
    counter = wgetch(com_win);
    if (counter == -1)
        exit(0);
    werase(com_win);
    wmove(com_win, 0, 0);
    werase(help_win);
    wrefresh(help_win);
    wrefresh(com_win);
    redraw();
}

void modes_op(void)
{
    int ret_value;
    int counter;
    char *string;

    do {
        snprintf(modes_menu[1].item_string, 80, "%s %s", mode_strings[1],
                 expand_tabs ? ON : OFF);
        snprintf(modes_menu[2].item_string, 80, "%s %s", mode_strings[2],
                 case_sen ? ON : OFF);
        snprintf(modes_menu[3].item_string, 80, "%s %s", mode_strings[3],
                 observ_margins ? ON : OFF);
        snprintf(modes_menu[4].item_string, 80, "%s %s", mode_strings[4],
                 auto_format ? ON : OFF);
        snprintf(modes_menu[5].item_string, 80, "%s %s", mode_strings[5],
                 eightbit ? ON : OFF);
        snprintf(modes_menu[6].item_string, 80, "%s %s", mode_strings[6],
                 info_window ? ON : OFF);
        snprintf(modes_menu[7].item_string, 80, "%s %s", mode_strings[7],
                 emacs_keys_mode ? ON : OFF);
        snprintf(modes_menu[8].item_string, 80, "%s %d", mode_strings[8],
                 right_margin);
        snprintf(modes_menu[9].item_string, 80, "%s %s", mode_strings[9],
                 ee_chinese ? ON : OFF);

        ret_value = menu_op(modes_menu);

        switch (ret_value) {
        case 1:
            expand_tabs = !expand_tabs;
            break;
        case 2:
            case_sen = !case_sen;
            break;
        case 3:
            observ_margins = !observ_margins;
            break;
        case 4:
            auto_format = !auto_format;
            if (auto_format)
                observ_margins = TRUE;
            break;
        case 5:
            eightbit = !eightbit;
            if (!eightbit)
                ee_chinese = FALSE;
#ifdef NCURSE
            if (ee_chinese)
                nc_setattrib(A_NC_BIG5);
            else
                nc_clearattrib(A_NC_BIG5);
#endif /* NCURSE */
            redraw();
            wnoutrefresh(text_win);
            break;
        case 6:
            if (info_window)
                no_info_window();
            else
                create_info_window();
            break;
        case 7:
            emacs_keys_mode = !emacs_keys_mode;
            if (info_window)
                paint_info_win();
            break;
        case 8:
            string = get_string(margin_prompt, TRUE);
            if (string != NULL) {
                counter = atoi(string);
                if (counter > 0)
                    right_margin = counter;
                free(string);
            }
            break;
        case 9:
            ee_chinese = !ee_chinese;
            if (ee_chinese != FALSE)
                eightbit = TRUE;
#ifdef NCURSE
            if (ee_chinese)
                nc_setattrib(A_NC_BIG5);
            else
                nc_clearattrib(A_NC_BIG5);
#endif /* NCURSE */
            redraw();
            break;
        default:
            break;
        }
    } while (ret_value != 0);
}

