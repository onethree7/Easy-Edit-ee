#define _XOPEN_SOURCE_EXTENDED 1
#define _XOPEN_SOURCE 600
#include <stdlib.h>
#include <time.h>
#include <wchar.h>
#include <ncursesw/curses.h>

#include "undo.h"
#include "input.h"
#include "screen.h"
#include "text.h"
#include "menu.h"

#include "config.h"
#define max(a, b) ((a) > (b) ? (a) : (b))

int collecting_paste = 0;
/* globals from ee.c */
extern int in;
extern int scr_vert;
extern int scr_horz;
extern int horiz_offset;
extern int last_line;
extern int gold;
extern WINDOW *text_win;
extern ee_char *point;

/* helpers from ee.c */
char *get_string(const char *prompt, int advance);
void bottom(void);
void command_prompt(void);
void down(void);
void search_prompt(void);
void redo_action(void);
void bol(void);
void delete(int disp);
void insert_line(int disp);
void undo_action(void);
void left(int disp);
void insert(int character);
void move_rel(int direction, int lines);
void eol(void);
void top(void);
void up(void);
void undel_word(void);
void del_word(void);
int search(int display_message);
void del_line(void);
void undel_line(void);
extern struct menu_entries main_menu[];
void adv_word(void);
void adv_line(void);
void paint_info_win(void);
void midscreen(int line, ee_char *pnt);
static int read_seq(wint_t *seq, int len)
{
    for (int i = 0; i < len; i++) {
        if (wget_wch(text_win, &seq[i]) == ERR) {
            for (int j = i - 1; j >= 0; j--)
                unget_wch(seq[j]);
            return 0;
        }
    }
    return 1;
}

static int check_start_seq(void)
{
    wint_t s[5];
    if (!read_seq(s, 5))
        return 0;
    if (s[0] == '[' && s[1] == '2' && s[2] == '0' && s[3] == '0' && s[4] == '~')
        return 1;
    for (int i = 4; i >= 0; i--)
        unget_wch(s[i]);
    return 0;
}

static int check_end_seq(void)
{
    wint_t s[5];
    if (!read_seq(s, 5))
        return 0;
    if (s[0] == '[' && s[1] == '2' && s[2] == '0' && s[3] == '1' && s[4] == '~')
        return 1;
    for (int i = 4; i >= 0; i--)
        unget_wch(s[i]);
    return 0;
}

int collect_input_chunk(int *buf, int max)
{
    collecting_paste = 0;
    int len = 0;
    wint_t ch;
    int rc = wget_wch(text_win, &ch);
    if (rc == ERR)
        return 0;

    if (ch == 27) {
        nodelay(text_win, TRUE);
        if (check_start_seq()) {
            undo_begin_chunk();
            collecting_paste = 1;
            size_t cap = 256, plen = 0;
            int *pbuf = malloc(cap * sizeof(int));
            while (1) {
                rc = wget_wch(text_win, &ch);
                if (rc == ERR)
                    continue;
                if (ch == 27 && check_end_seq()) {
                    nodelay(text_win, FALSE);
                    int out = plen < (size_t)max ? plen : (size_t)max;
                    for (int i = 0; i < out; i++)
                        buf[i] = pbuf[i];
                    free(pbuf);
                    return out;
                }
                if (plen >= cap) {
                    cap *= 2;
                    pbuf = realloc(pbuf, cap * sizeof(int));
                }
                pbuf[plen++] = (int)ch;
            }
        }
        nodelay(text_win, FALSE);
    }

    buf[len++] = (int)ch;

    nodelay(text_win, TRUE);
    struct timespec delay = {0, 30000000};
    while (len < max) {
        rc = wget_wch(text_win, &ch);
        if (rc == ERR) {
            nanosleep(&delay, NULL);
            rc = wget_wch(text_win, &ch);
            if (rc == ERR)
                break;
        }
        if (ch == 27 && check_start_seq()) {
            unget_wch('~');
            unget_wch('0');
            unget_wch('0');
            unget_wch('2');
            unget_wch('[');
            unget_wch(27);
            break;
        }
        buf[len++] = (int)ch;
    }
    nodelay(text_win, FALSE);
    return len;
}

void start_action(void)
{
    undo_begin_chunk();
}


void
control(void)
{
	char *string;

	if (in == 1)		/* control a	*/
	{
		string = get_string(ascii_code_str, TRUE);
		if (*string != '\0')
		{
			in = atoi(string);
			wmove(text_win, scr_vert, (scr_horz - horiz_offset));
			insert(in);
		}
		free(string);
	}
	else if (in == 2)	/* control b	*/
		bottom();
	else if (in == 3)	/* control c	*/
	{
		command_prompt();
	}
	else if (in == 4)	/* control d	*/
		down();
	else if (in == 5)	/* control e	*/
		search_prompt();
	else if (in == 6)	/* control f	*/
		redo_action();
	else if (in == 7)	/* control g	*/
		bol();
	else if (in == 8)	/* control h	*/
		delete(TRUE);
	else if (in == 9)	/* control i	*/
		;
	else if (in == 10)	/* control j	*/
		insert_line(TRUE);
	else if (in == 11)	/* control k	*/
		undo_action();
	else if (in == 12)	/* control l	*/
		left(TRUE);
	else if (in == 13)	/* control m	*/
		insert_line(TRUE);
	else if (in == 14)	/* control n	*/
		move_rel('d', max(5, (last_line - 5)));
	else if (in == 15)	/* control o	*/
		eol();
	else if (in == 16)	/* control p	*/
		move_rel('u', max(5, (last_line - 5)));
	else if (in == 17)	/* control q	*/
		;
	else if (in == 18)	/* control r	*/
		right(TRUE);
	else if (in == 19)	/* control s	*/
		;
	else if (in == 20)	/* control t	*/
		top();
	else if (in == 21)	/* control u	*/
		up();
	else if (in == 22)	/* control v	*/
		undel_word();
	else if (in == 23)	/* control w	*/
		del_word();
	else if (in == 24)	/* control x	*/
		search(TRUE);
	else if (in == 25)	/* control y	*/
		del_line();
	else if (in == 26)	/* control z	*/
		undel_line();
	else if (in == 27)	/* control [ (escape)	*/
	{
		menu_op(main_menu);
	}	
}
/*
 |      Emacs control-key bindings
 */

void
emacs_control(void)
{
	char *string;

	if (in == 1)		/* control a	*/
		bol();
	else if (in == 2)	/* control b	*/
		left(TRUE);
	else if (in == 3)	/* control c	*/
	{
		command_prompt();
	}
	else if (in == 4)	/* control d	*/
		undo_action();
	else if (in == 5)	/* control e	*/
		eol();
	else if (in == 6)	/* control f	*/
		right(TRUE);
	else if (in == 7)	/* control g	*/
		move_rel('u', max(5, (last_line - 5)));
	else if (in == 8)	/* control h	*/
		delete(TRUE);
	else if (in == 9)	/* control i	*/
		;
	else if (in == 10)	/* control j	*/
		redo_action();
	else if (in == 11)	/* control k	*/
		del_line();
	else if (in == 12)	/* control l	*/
		undel_line();
	else if (in == 13)	/* control m	*/
		insert_line(TRUE);
	else if (in == 14)	/* control n	*/
		down();
	else if (in == 15)	/* control o	*/
	{
		string = get_string(ascii_code_str, TRUE);
		if (*string != '\0')
		{
			in = atoi(string);
			wmove(text_win, scr_vert, (scr_horz - horiz_offset));
			insert(in);
		}
		free(string);
	}
	else if (in == 16)	/* control p	*/
		up();
	else if (in == 17)	/* control q	*/
		;
	else if (in == 18)	/* control r	*/
		undel_word();
	else if (in == 19)	/* control s	*/
		;
	else if (in == 20)	/* control t	*/
		top();
	else if (in == 21)	/* control u	*/
		bottom();
	else if (in == 22)	/* control v	*/
		move_rel('d', max(5, (last_line - 5)));
	else if (in == 23)	/* control w	*/
		del_word();
	else if (in == 24)	/* control x	*/
		search(TRUE);
	else if (in == 25)	/* control y	*/
		search_prompt();
	else if (in == 26)	/* control z	*/
		adv_word();
	else if (in == 27)	/* control [ (escape)	*/
	{
		menu_op(main_menu);
	}	
}


void
function_key(void)
{
	if (in == KEY_LEFT)
		left(TRUE);
	else if (in == KEY_RIGHT)
		right(TRUE);
	else if (in == KEY_HOME)
		bol();
	else if (in == KEY_END)
		eol();
	else if (in == KEY_UP)
		up();
	else if (in == KEY_DOWN)
		down();
	else if (in == KEY_NPAGE)
		move_rel('d', max( 5, (last_line - 5)));
	else if (in == KEY_PPAGE)
		move_rel('u', max(5, (last_line - 5)));
	else if (in == KEY_DL)
		del_line();
	else if (in == KEY_DC)
		undo_action();
	else if (in == KEY_BACKSPACE)
		delete(TRUE);
	else if (in == KEY_IL)
	{		/* insert a line before current line	*/
		insert_line(TRUE);
		left(TRUE);
	}
	else if (in == KEY_F(1))
		gold = !gold;
	else if (in == KEY_F(2))
	{
		if (gold)
		{
			gold = FALSE;
			undel_line();
		}
		else
			redo_action();
	}
	else if (in == KEY_F(3))
	{
		if (gold)
		{
			gold = FALSE;
			undel_word();
		}
		else
			del_word();
	}
	else if (in == KEY_F(4))
	{
		if (gold)
		{
			gold = FALSE;
			paint_info_win();
			midscreen(scr_vert, point);
		}
		else
			adv_word();
	}
	else if (in == KEY_F(5))
	{
		if (gold)
		{
			gold = FALSE;
			search_prompt();
		}
		else
			search(TRUE);
	}
	else if (in == KEY_F(6))
	{
		if (gold)
		{
			gold = FALSE;
			bottom();
		}
		else
			top();
	}
	else if (in == KEY_F(7))
	{
		if (gold)
		{
			gold = FALSE;
			eol();
		}
		else
			bol();
	}
	else if (in == KEY_F(8))
	{
		if (gold)
		{
			gold = FALSE;
			command_prompt();
		} 
		else
                adv_line();
        }
}

