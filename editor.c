#define _XOPEN_SOURCE_EXTENDED 1
#define _XOPEN_SOURCE 600

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <locale.h>
#include <wchar.h>
#include <wctype.h>
#include <time.h>
#include <signal.h>
#include <nl_types.h>
#define CONTROL_KEYS 1
#define COMMANDS 2
#include <unistd.h>
#define NUM_MODES_ITEMS 10
#include <sys/stat.h>
#include <errno.h>
#include <ncursesw/curses.h>

#include "ee_version.h"
#include "text.h"
#include "buffer.h"
#include "fileio.h"
#include "screen.h"
#include "search.h"
#include "undo.h"
#include "editor.h"

struct menu_entries {
    char *item_string;
    int (*procedure)(struct menu_entries *);
    struct menu_entries *ptr_argument;
    int (*iprocedure)(int);
    void (*nprocedure)(void);
    int argument;
};

/* globals defined in ee.c */
extern struct text *first_line;
extern struct text *dlt_line;
extern struct text *curr_line;
extern ee_char *point;
extern ee_char *srch_str;
extern ee_char *u_srch_str;
extern int position;
extern int scr_pos;
extern int scr_vert;
extern int scr_horz;
extern int absolute_lin;
extern int edit;
extern int info_window;
extern int nohighlight;
extern int clear_com_win;
extern int emacs_keys_mode;
extern int right_margin;
extern int horiz_offset;
extern int gold;
extern int case_sen;
extern int shell_fork;
extern int in;
extern char *in_file_name;
extern char *no_file_string;
extern char *com_win_message;
extern char *separator;
extern char *ree_no_file_msg;
extern WINDOW *com_win;
extern WINDOW *text_win;
extern WINDOW *info_win;
extern FILE *bit_bucket;
extern struct files *top_of_stack;
extern ee_char *d_char;
extern ee_char *d_word;
extern ee_char *d_line;
extern WINDOW *help_win;
extern int curses_initialized;
extern int last_line;
extern int last_col;
extern int local_LINES;
extern int local_COLS;
extern int info_type;
extern int eightbit;
extern int ee_chinese;
extern char *print_command;
extern char *mode_strings[];
extern struct menu_entries modes_menu[];
extern struct menu_entries leave_menu[];
extern struct menu_entries file_menu[];
extern struct menu_entries search_menu[];
extern struct menu_entries spell_menu[];
extern struct menu_entries misc_menu[];
extern struct menu_entries main_menu[];
extern char *help_text[];
extern char *emacs_help_text[];
extern char *control_keys[];
extern char *emacs_control_keys[];
extern char *command_strings[];
extern char *ascii_code_str;
extern char *printer_msg_str;
extern char *command_str;
extern char *file_write_prompt_str;
extern char *file_read_prompt_str;
extern char *char_str;
extern char *unkn_cmd_str;
extern char *non_unique_cmd_msg;
extern char *line_num_str;
extern char *line_len_str;
extern char *current_file_str;
extern char *usage0;
extern char *usage1;
extern char *usage2;
extern char *usage3;
extern char *usage4;
extern char *file_is_dir_msg;
extern char *new_file_msg;
extern char *cant_open_msg;
extern char *open_file_msg;
extern char *file_read_fin_msg;
extern char *reading_file_msg;
extern char *read_only_msg;
extern char *file_read_lines_msg;
extern char *save_file_name_prompt;
extern char *file_not_saved_msg;
extern char *changes_made_prompt;
extern char *yes_char;
extern char *file_exists_prompt;
extern char *create_file_fail_msg;
extern char *writing_file_msg;
extern char *file_written_msg;
extern char *searching_msg;
extern char *str_not_found_msg;
extern char *search_prompt_str;
extern char *exec_err_msg;
extern char *continue_msg;
extern char *menu_cancel_msg;
extern char *menu_size_err_msg;
extern char *press_any_key_msg;
extern char *shell_prompt;
extern char *formatting_msg;
extern char *shell_echo_msg;
extern char *spell_in_prog_msg;
extern char *margin_prompt;
extern char *restricted_msg;
extern char *ON;
extern char *OFF;
extern char *HELP;
extern char *WRITE;
extern char *READ;
extern char *LINE;
extern char *FILE_str;
extern char *CHARACTER;
extern char *REDRAW;
extern char *RESEQUENCE;
extern char *AUTHOR;
extern char *VERSION;
extern char *CASE;
extern char *NOCASE;
extern char *EXPAND;
extern char *NOEXPAND;
extern char *Exit_string;
extern char *QUIT_string;
extern char *INFO;
extern char *NOINFO;
extern char *MARGINS;
extern char *NOMARGINS;
extern char *AUTOFORMAT;
extern char *NOAUTOFORMAT;
extern char *Echo;
extern char *PRINTCOMMAND;
extern char *RIGHTMARGIN;
extern char *HIGHLIGHT;
extern char *NOHIGHLIGHT;
extern char *EIGHTBIT;
extern char *NOEIGHTBIT;
extern char *EMACS_string;
extern char *NOEMACS_string;
extern char *conf_dump_err_msg;
extern char *conf_dump_success_msg;
extern struct menu_entries config_dump_menu[];
extern char *conf_not_saved_msg;
extern char *menu_too_lrg_msg;
extern char *more_above_str;
extern char *more_below_str;
extern char *chinese_cmd;
extern char *nochinese_cmd;
extern char *commands[];
extern char *init_strings[];
extern nl_catd catalog;
extern char *init_name[];
extern int expand_tabs;
extern int observ_margins;
extern int auto_format;

extern int unique_test(char *string, char *list[]);
extern int compare(char *string1, char *string2, int sensitive);
extern char *next_ascii_word(char *string);
extern void echo_string(char *string);
extern char *catgetlocal(int, const char *);
extern char *get_string(const char *prompt, int advance);
extern void command(char *cmd_str1);

/* external helpers still in ee.c */
extern void get_options(int numargs, char *arguments[]);
extern int restrict_mode(void);
extern void check_fp(void);
extern void delete(int disp);
extern void insert_line(int disp);
extern void insert(int character);
extern void control(void);
extern void emacs_control(void);
extern void finish(void);
extern void edit_abort(int arg);
extern void function_key(void);

static struct timespec last_input_time = {0};

static int collect_input_chunk(int *buf, int max)
{
    int len = 0;
    wint_t ch;
    int rc = wget_wch(text_win, &ch);
    if (rc == ERR)
        return 0;
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
        buf[len++] = (int)ch;
    }
    nodelay(text_win, FALSE);
    return len;
}

int main(int argc, char *argv[])
{
    int counter;

    for (counter = 1; counter < 24; counter++)
        signal(counter, SIG_IGN);

    if (!isatty(STDIN_FILENO) || !isatty(STDOUT_FILENO)) {
        fprintf(stderr,
                "ee's standard input and output must be a terminal\n");
        exit(1);
    }

    signal(SIGCHLD, SIG_DFL);
    signal(SIGSEGV, SIG_DFL);
    signal(SIGINT, edit_abort);

    d_char = malloc(3 * sizeof(ee_char));
    d_word = malloc(150 * sizeof(ee_char));
    *d_word = '\0';
    d_line = NULL;
    dlt_line = txtalloc();
    dlt_line->line = d_line;
    dlt_line->line_length = 0;
    curr_line = first_line = txtalloc();
    curr_line->line = point = malloc(10 * sizeof(ee_char));
    curr_line->line_length = 1;
    curr_line->max_length = 10;
    curr_line->prev_line = NULL;
    curr_line->next_line = NULL;
    curr_line->line_number  = 1;
    srch_str = NULL;
    u_srch_str = NULL;
    position = 1;
    scr_pos = 0;
    scr_vert = 0;
    scr_horz = 0;
    absolute_lin = 1;
    bit_bucket = fopen("/dev/null", "w");
    edit = TRUE;
    gold = case_sen = FALSE;
    shell_fork = TRUE;
    strings_init();
    ee_init();
    if (argc > 0)
        get_options(argc, argv);
    set_up_term();
    if (right_margin == 0)
        right_margin = COLS - 1;
    if (top_of_stack == NULL) {
        if (restrict_mode()) {
            wmove(com_win, 0, 0);
            werase(com_win);
            wprintw(com_win, "%s", ree_no_file_msg);
            wrefresh(com_win);
            edit_abort(0);
        }
        wprintw(com_win, "%s", no_file_string);
        wrefresh(com_win);
    } else
        check_fp();

    undo_push_state();
    run_editor();
    return 0;
}

void run_editor(void)
{
    clear_com_win = TRUE;

    while (edit) {
        if (info_window) {
            if (!nohighlight)
                wstandout(info_win);
            wmove(info_win, 5, 0);
            wprintw(info_win, "%s", separator);
            wmove(info_win, 5, 5);
            wprintw(info_win, "line %d col %d lines from top %d ",
                    curr_line->line_number, scr_horz, absolute_lin);
            wstandend(info_win);
            wrefresh(info_win);
        }

        wrefresh(text_win);

        int buf[4096];
        int buf_len = collect_input_chunk(buf, 4096);

        struct timespec now;
        clock_gettime(CLOCK_MONOTONIC, &now);
        long diff_ms = (now.tv_sec - last_input_time.tv_sec) * 1000L +
                       (now.tv_nsec - last_input_time.tv_nsec) / 1000000L;
        if (last_input_time.tv_sec == 0 || diff_ms > 500 || buf_len > 1)
            undo_end_chunk();
        last_input_time = now;

        for (int i = 0; i < buf_len; i++) {
            in = buf[i];

            resize_check();

            if (clear_com_win) {
                clear_com_win = FALSE;
                wmove(com_win, 0, 0);
                werase(com_win);
                if (!info_window)
                    wprintw(com_win, "%s", com_win_message);
                wrefresh(com_win);
            }

            if (in > 255) {
                function_key();
            } else if ((in == '\10') || (in == 127)) {
                in = 8;
                delete(TRUE);
            } else if (in == '\n' || in == '\r') {
                insert_line(TRUE);
            } else if ((in > 31) || (in == 9))
                insert(in);
            else if ((in >= 0) && (in <= 31)) {
                if (emacs_keys_mode)
                    emacs_control();
                else
                    control();
            }
        }
        undo_end_chunk();
    }
}

void set_up_term(void)
{
    if (!curses_initialized) {
        initscr();
        savetty();
        noecho();
        raw();
        nonl();
        meta(stdscr, TRUE);
        curses_initialized = TRUE;
    }

    if (((LINES > 15) && (COLS >= 80)) && info_window)
        last_line = LINES - 8;
    else {
        info_window = FALSE;
        last_line = LINES - 2;
    }

    idlok(stdscr, TRUE);
    com_win = newwin(1, COLS, (LINES - 1), 0);
    keypad(com_win, TRUE);
    idlok(com_win, TRUE);
    meta(com_win, TRUE);
    wrefresh(com_win);
    if (!info_window)
        text_win = newwin((LINES - 1), COLS, 0, 0);
    else
        text_win = newwin((LINES - 7), COLS, 6, 0);
    keypad(text_win, TRUE);
    idlok(text_win, TRUE);
    meta(text_win, TRUE);
    wrefresh(text_win);
    help_win = newwin((LINES - 1), COLS, 0, 0);
    keypad(help_win, TRUE);
    idlok(help_win, TRUE);
    meta(help_win, TRUE);
    if (info_window) {
        info_type = CONTROL_KEYS;
        info_win = newwin(6, COLS, 0, 0);
        werase(info_win);
        paint_info_win();
    }

    last_col = COLS - 1;
    local_LINES = LINES;
    local_COLS = COLS;

#ifdef NCURSE
    if (ee_chinese)
        nc_setattrib(A_NC_BIG5);
#endif /* NCURSE */
}

void ee_init(void)
{
    FILE *init_file;
    char *string;
    char *str1;
    char *str2;
    char *home;
    int counter;
    int temp_int;

    string = getenv("HOME");
    if (string == NULL)
        string = "/tmp";
    str1 = home = malloc(strlen(string)+10);
    strcpy(home, string);
    strcat(home, "/.init.ee");
    init_name[1] = home;
    string = malloc(512);

    for (counter = 0; counter < 3; counter++) {
        if (!(access(init_name[counter], 4))) {
            init_file = fopen(init_name[counter], "r");
            while ((str2 = fgets(string, 512, init_file)) != NULL) {
                str1 = str2 = string;
                while (*str2 != '\n')
                    str2++;
                *str2 = '\0';

                if (unique_test(string, init_strings) != 1)
                    continue;

                if (compare(str1, CASE, FALSE))
                    case_sen = TRUE;
                else if (compare(str1, NOCASE, FALSE))
                    case_sen = FALSE;
                else if (compare(str1, EXPAND, FALSE))
                    expand_tabs = TRUE;
                else if (compare(str1, NOEXPAND, FALSE))
                    expand_tabs = FALSE;
                else if (compare(str1, INFO, FALSE))
                    info_window = TRUE;
                else if (compare(str1, NOINFO, FALSE))
                    info_window = FALSE;
                else if (compare(str1, MARGINS, FALSE))
                    observ_margins = TRUE;
                else if (compare(str1, NOMARGINS, FALSE))
                    observ_margins = FALSE;
                else if (compare(str1, AUTOFORMAT, FALSE)) {
                    auto_format = TRUE;
                    observ_margins = TRUE;
                } else if (compare(str1, NOAUTOFORMAT, FALSE))
                    auto_format = FALSE;
                else if (compare(str1, Echo, FALSE)) {
                    str1 = next_ascii_word(str1);
                    if (*str1 != '\0')
                        echo_string(str1);
                } else if (compare(str1, PRINTCOMMAND, FALSE)) {
                    str1 = next_ascii_word(str1);
                    print_command = malloc(strlen(str1)+1);
                    strcpy(print_command, str1);
                } else if (compare(str1, RIGHTMARGIN, FALSE)) {
                    str1 = next_ascii_word(str1);
                    if ((*str1 >= '0') && (*str1 <= '9')) {
                        temp_int = atoi(str1);
                        if (temp_int > 0)
                            right_margin = temp_int;
                    }
                } else if (compare(str1, HIGHLIGHT, FALSE))
                    nohighlight = FALSE;
                else if (compare(str1, NOHIGHLIGHT, FALSE))
                    nohighlight = TRUE;
                else if (compare(str1, EIGHTBIT, FALSE))
                    eightbit = TRUE;
                else if (compare(str1, NOEIGHTBIT, FALSE)) {
                    eightbit = FALSE;
                    ee_chinese = FALSE;
                } else if (compare(str1, EMACS_string, FALSE))
                    emacs_keys_mode = TRUE;
                else if (compare(str1, NOEMACS_string, FALSE))
                    emacs_keys_mode = FALSE;
                else if (compare(str1, chinese_cmd, FALSE)) {
                    ee_chinese = TRUE;
                    eightbit = TRUE;
                } else if (compare(str1, nochinese_cmd, FALSE))
                    ee_chinese = FALSE;
            }
            fclose(init_file);
        }
    }
    free(string);
    free(home);

    string = getenv("LANG");
    if (string != NULL) {
        if (strcmp(string, "zh_TW.big5") == 0) {
            ee_chinese = TRUE;
            eightbit = TRUE;
        }
    }
}

void strings_init(void)
{
    int counter;

    if (!setlocale(LC_ALL, ""))
        setlocale(LC_ALL, "C.UTF-8");
#ifndef NO_CATGETS
    catalog = catopen("ee", NL_CAT_LOCALE);
#endif /* NO_CATGETS */

    modes_menu[0].item_string = catgetlocal( 1, "modes menu");
    mode_strings[1]  = catgetlocal( 2, "tabs to spaces       ");
    mode_strings[2]  = catgetlocal( 3, "case sensitive search");
    mode_strings[3]  = catgetlocal( 4, "margins observed     ");
    mode_strings[4]  = catgetlocal( 5, "auto-paragraph format");
    mode_strings[5]  = catgetlocal( 6, "eightbit characters  ");
    mode_strings[6]  = catgetlocal( 7, "info window          ");
    mode_strings[8]  = catgetlocal( 8, "right margin         ");
    leave_menu[0].item_string  = catgetlocal( 9, "leave menu");
    leave_menu[1].item_string  = catgetlocal( 10, "save changes");
    leave_menu[2].item_string  = catgetlocal( 11, "no save");
    file_menu[0].item_string  = catgetlocal( 12, "file menu");
    file_menu[1].item_string  = catgetlocal( 13, "read a file");
    file_menu[2].item_string  = catgetlocal( 14, "write a file");
    file_menu[3].item_string  = catgetlocal( 15, "save file");
    file_menu[4].item_string  = catgetlocal( 16, "print editor contents");
    search_menu[0].item_string = catgetlocal( 17, "search menu");
    search_menu[1].item_string = catgetlocal( 18, "search for ...");
    search_menu[2].item_string = catgetlocal( 19, "search");
    spell_menu[0].item_string = catgetlocal( 20, "spell menu");
    spell_menu[1].item_string = catgetlocal( 21, "use 'spell'");
    spell_menu[2].item_string = catgetlocal( 22, "use 'ispell'");
    misc_menu[0].item_string = catgetlocal( 23, "miscellaneous menu");
    misc_menu[1].item_string = catgetlocal( 24, "format paragraph");
    misc_menu[2].item_string = catgetlocal( 25, "shell command");
    misc_menu[3].item_string = catgetlocal( 26, "check spelling");
    main_menu[0].item_string  = catgetlocal( 27, "main menu");
    main_menu[1].item_string  = catgetlocal( 28, "leave editor");
    main_menu[2].item_string  = catgetlocal( 29, "help");
    main_menu[3].item_string  = catgetlocal( 30, "file operations");
    main_menu[4].item_string  = catgetlocal( 31, "redraw screen");
    main_menu[5].item_string  = catgetlocal( 32, "settings");
    main_menu[6].item_string  = catgetlocal( 33, "search");
    main_menu[7].item_string  = catgetlocal( 34, "miscellaneous");
    help_text[0] = catgetlocal( 35, "Control keys:");
    help_text[1] = catgetlocal( 36, "^a ascii code           ^i tab         ^r right");
    help_text[2] = catgetlocal( 37, "^b bottom of text       ^j newline     ^t top of text");
    help_text[3] = catgetlocal( 38, "^c command              ^k undo  ^u up");
    help_text[4] = catgetlocal( 39, "^d down                 ^l left        ^v undelete word");
    help_text[5] = catgetlocal( 40, "^e search prompt        ^m newline     ^w delete word");
    help_text[6] = catgetlocal( 41, "^f redo        ^n next page ^x search");
    help_text[7] = catgetlocal( 42, "^g begin of line        ^o end of line ^y delete line");
    help_text[8] = catgetlocal( 43, "^h backspace            ^p prev page   ^z undelete line");
    help_text[9] = catgetlocal( 44, "^[ (escape) menu        ESC-Enter: exit ee");
    help_text[10] = catgetlocal( 45, "");
    help_text[11] = catgetlocal( 46, "Commands:");
    help_text[12] = catgetlocal( 47, "help    : get this info  |file  : print file name");
    help_text[13] = catgetlocal( 48, "read    : read a file    |char  : ascii code of char");
    help_text[14] = catgetlocal( 49, "write   : write a file   |case  : case sensitive search");
    help_text[15] = catgetlocal( 50, "exit    : leave and save |nocase  : case insensitive search");
    help_text[16] = catgetlocal( 51, "quit    : leave, no save |!cmd    : execute 'cmd' in shell");
    help_text[17] = catgetlocal( 52, "line    : display line # |0-9     : go to line '#'");
    help_text[18] = catgetlocal( 53, "expand  : expand tabs   |noexpand: do not expand tabs");
    help_text[19] = catgetlocal( 54, "");
    help_text[20] = catgetlocal( 55, "  ee [+#] [-i] [-e] [-h] [file(s)]");
    help_text[21] = catgetlocal( 56, "+# :go to line #  -i :no info window -e : don't expand tabs  -h :no highlight");
    control_keys[0] = catgetlocal( 57, "^[ (escape) menu  ^e search prompt ^y delete line    ^u up     ^p prev page");
    control_keys[1] = catgetlocal( 58, "^a ascii code     ^x search ^z undelete line  ^d down   ^n next page");
    control_keys[2] = catgetlocal( 59, "^b bottom of text ^g begin of line ^w delete word    ^l left");
    control_keys[3] = catgetlocal( 60, "^t top of text    ^o end of line   ^v undelete word  ^r right");
    control_keys[4] = catgetlocal( 61, "^c command        ^k undo    ^f redo      ESC-Enter: exit ee");
    command_strings[0] = catgetlocal( 62, "help : get help info  |file  : print file name         |line : print line #");
    command_strings[1] = catgetlocal( 63, "read : read a file    |char  : ascii code of char      |0-9 : go to line '#'");
    command_strings[2] = catgetlocal( 64, "write: write a file   |case  : case sensitive search   |exit : leave and save");
    command_strings[3] = catgetlocal( 65, "!cmd : shell 'cmd'    |nocase: ignore case in search   |quit : leave, no save");
    command_strings[4] = catgetlocal( 66, "expand: expand tabs   |noexpand: do not expand tabs");
    com_win_message = catgetlocal( 67, "    press Escape (^[) for menu");
    no_file_string = catgetlocal( 68, "no file");
    ascii_code_str = catgetlocal( 69, "ascii code: ");
    printer_msg_str = catgetlocal( 70, "sending contents of buffer to '%s'");
    command_str = catgetlocal( 71, "command: ");
    file_write_prompt_str = catgetlocal( 72, "name of file to write: ");
    file_read_prompt_str = catgetlocal( 73, "name of file to read: ");
    char_str = catgetlocal( 74, "character = %d");
    unkn_cmd_str = catgetlocal( 75, "unknown command '%s'");
    non_unique_cmd_msg = catgetlocal( 76, "entered command is not unique");
    line_num_str = catgetlocal( 77, "line %d  ");
    line_len_str = catgetlocal( 78, "length = %d");
    current_file_str = catgetlocal( 79, "current file is '%s' ");
    usage0 = catgetlocal( 80, "usage: %s [-i] [-e] [-h] [+line_number] [file(s)]\n");
    usage1 = catgetlocal( 81, "       -i   turn off info window\n");
    usage2 = catgetlocal( 82, "       -e   do not convert tabs to spaces\n");
    usage3 = catgetlocal( 83, "       -h   do not use highlighting\n");
    file_is_dir_msg = catgetlocal( 84, "file '%s' is a directory");
    new_file_msg = catgetlocal( 85, "new file '%s'");
    cant_open_msg = catgetlocal( 86, "can't open '%s'");
    open_file_msg = catgetlocal( 87, "file '%s', %d lines");
    file_read_fin_msg = catgetlocal( 88, "finished reading file '%s'");
    reading_file_msg = catgetlocal( 89, "reading file '%s'");
    read_only_msg = catgetlocal( 90, ", read only");
    file_read_lines_msg = catgetlocal( 91, "file '%s', %d lines");
    save_file_name_prompt = catgetlocal( 92, "enter name of file: ");
    file_not_saved_msg = catgetlocal( 93, "no filename entered: file not saved");
    changes_made_prompt = catgetlocal( 94, "changes have been made, are you sure? (y/n [n]) ");
    yes_char = catgetlocal( 95, "y");
    file_exists_prompt = catgetlocal( 96, "file already exists, overwrite? (y/n) [n] ");
    create_file_fail_msg = catgetlocal( 97, "unable to create file '%s'");
    writing_file_msg = catgetlocal( 98, "writing file '%s'");
    file_written_msg = catgetlocal( 99, "'%s' %d lines, %d characters");
    searching_msg = catgetlocal( 100, "           ...searching");
    str_not_found_msg = catgetlocal( 101, "string '%s' not found");
    search_prompt_str = catgetlocal( 102, "search for: ");
    exec_err_msg = catgetlocal( 103, "could not exec %s\n");
    continue_msg = catgetlocal( 104, "press return to continue ");
    menu_cancel_msg = catgetlocal( 105, "press Esc to cancel");
    menu_size_err_msg = catgetlocal( 106, "menu too large for window");
    press_any_key_msg = catgetlocal( 107, "press any key to continue ");
    shell_prompt = catgetlocal( 108, "shell command: ");
    formatting_msg = catgetlocal( 109, "...formatting paragraph...");
    shell_echo_msg = catgetlocal( 110, "<!echo 'list of unrecognized words'; echo -=-=-=-=-=-");
    spell_in_prog_msg = catgetlocal( 111, "sending contents of edit buffer to 'spell'");
    margin_prompt = catgetlocal( 112, "right margin is: ");
    restricted_msg = catgetlocal( 113, "restricted mode: unable to perform requested operation");
    ON = catgetlocal( 114, "ON");
    OFF = catgetlocal( 115, "OFF");
    HELP = catgetlocal( 116, "HELP");
    WRITE = catgetlocal( 117, "WRITE");
    READ = catgetlocal( 118, "READ");
    LINE = catgetlocal( 119, "LINE");
    FILE_str = catgetlocal( 120, "FILE");
    CHARACTER = catgetlocal( 121, "CHARACTER");
    REDRAW = catgetlocal( 122, "REDRAW");
    RESEQUENCE = catgetlocal( 123, "RESEQUENCE");
    AUTHOR = catgetlocal( 124, "AUTHOR");
    VERSION = catgetlocal( 125, "VERSION");
    CASE = catgetlocal( 126, "CASE");
    NOCASE = catgetlocal( 127, "NOCASE");
    EXPAND = catgetlocal( 128, "EXPAND");
    NOEXPAND = catgetlocal( 129, "NOEXPAND");
    Exit_string = catgetlocal( 130, "EXIT");
    QUIT_string = catgetlocal( 131, "QUIT");
    INFO = catgetlocal( 132, "INFO");
    NOINFO = catgetlocal( 133, "NOINFO");
    MARGINS = catgetlocal( 134, "MARGINS");
    NOMARGINS = catgetlocal( 135, "NOMARGINS");
    AUTOFORMAT = catgetlocal( 136, "AUTOFORMAT");
    NOAUTOFORMAT = catgetlocal( 137, "NOAUTOFORMAT");
    Echo = catgetlocal( 138, "ECHO");
    PRINTCOMMAND = catgetlocal( 139, "PRINTCOMMAND");
    RIGHTMARGIN = catgetlocal( 140, "RIGHTMARGIN");
    HIGHLIGHT = catgetlocal( 141, "HIGHLIGHT");
    NOHIGHLIGHT = catgetlocal( 142, "NOHIGHLIGHT");
    EIGHTBIT = catgetlocal( 143, "EIGHTBIT");
    NOEIGHTBIT = catgetlocal( 144, "NOEIGHTBIT");
    mode_strings[7] = catgetlocal( 145, "emacs key bindings   ");
    emacs_help_text[0] = help_text[0];
    emacs_help_text[1] = catgetlocal( 146, "^a beginning of line    ^i tab                ^r restore word");
    emacs_help_text[2] = catgetlocal( 147, "^b back 1 char          ^j undel char           ^t top of text");
    emacs_help_text[3] = catgetlocal( 148, "^c command              ^k delete line          ^u bottom of text");
    emacs_help_text[4] = catgetlocal( 149, "^d undo          ^l undelete line        ^v next page");
    emacs_help_text[5] = catgetlocal( 150, "^e end of line          ^m newline              ^w delete word");
    emacs_help_text[6] = catgetlocal( 151, "^f forward 1 char       ^n next line            ^x search");
    emacs_help_text[7] = catgetlocal( 152, "^g go back 1 page       ^o ascii char insert    ^y search prompt");
    emacs_help_text[8] = catgetlocal( 153, "^h backspace            ^p prev line            ^z next word");
    emacs_help_text[9] = help_text[9];
    emacs_help_text[10] = help_text[10];
    emacs_help_text[11] = help_text[11];
    emacs_help_text[12] = help_text[12];
    emacs_help_text[13] = help_text[13];
    emacs_help_text[14] = help_text[14];
    emacs_help_text[15] = help_text[15];
    emacs_help_text[16] = help_text[16];
    emacs_help_text[17] = help_text[17];
    emacs_help_text[18] = help_text[18];
    emacs_help_text[19] = help_text[19];
    emacs_help_text[20] = help_text[20];
    emacs_help_text[21] = help_text[21];
    emacs_control_keys[0] = catgetlocal( 154, "^[ (escape) menu ^y search prompt ^k delete line   ^p prev li     ^g prev page");
    emacs_control_keys[1] = catgetlocal( 155, "^o ascii code    ^x search     ^l undelete line ^n next li     ^v next page");
    emacs_control_keys[2] = catgetlocal( 156, "^u end of file   ^a begin of line ^w delete word   ^b back 1 char ^z next word");
    emacs_control_keys[3] = catgetlocal( 157, "^t top of text   ^e end of line   ^r restore word  ^f forward char");
    emacs_control_keys[4] = catgetlocal( 158, "^c command       ^d undo   ^j redo              ESC-Enter: exit");
    EMACS_string = catgetlocal( 159, "EMACS");
    NOEMACS_string = catgetlocal( 160, "NOEMACS");
    usage4 = catgetlocal( 161, "       +#   put cursor at line #\n");
    conf_dump_err_msg = catgetlocal( 162, "unable to open .init.ee for writing, no configuration saved!");
    conf_dump_success_msg = catgetlocal( 163, "ee configuration saved in file %s");
    modes_menu[10].item_string = catgetlocal( 164, "save editor configuration");
    config_dump_menu[0].item_string = catgetlocal( 165, "save ee configuration");
    config_dump_menu[1].item_string = catgetlocal( 166, "save in current directory");
    config_dump_menu[2].item_string = catgetlocal( 167, "save in home directory");
    conf_not_saved_msg = catgetlocal( 168, "ee configuration not saved");
    ree_no_file_msg = catgetlocal( 169, "must specify a file when invoking ree");
    menu_too_lrg_msg = catgetlocal( 180, "menu too large for window");
    more_above_str = catgetlocal( 181, "^^more^^");
    more_below_str = catgetlocal( 182, "VVmoreVV");
    mode_strings[9] = catgetlocal( 183, "16 bit characters    ");
    chinese_cmd = catgetlocal( 184, "16BIT");
    nochinese_cmd = catgetlocal( 185, "NO16BIT");

    commands[0] = HELP;
    commands[1] = WRITE;
    commands[2] = READ;
    commands[3] = LINE;
    commands[4] = FILE_str;
    commands[5] = REDRAW;
    commands[6] = RESEQUENCE;
    commands[7] = AUTHOR;
    commands[8] = VERSION;
    commands[9] = CASE;
    commands[10] = NOCASE;
    commands[11] = EXPAND;
    commands[12] = NOEXPAND;
    commands[13] = Exit_string;
    commands[14] = QUIT_string;
    commands[15] = "<";
    commands[16] = ">";
    commands[17] = "!";
    commands[18] = "0";
    commands[19] = "1";
    commands[20] = "2";
    commands[21] = "3";
    commands[22] = "4";
    commands[23] = "5";
    commands[24] = "6";
    commands[25] = "7";
    commands[26] = "8";
    commands[27] = "9";
    commands[28] = CHARACTER;
    commands[29] = chinese_cmd;
    commands[30] = nochinese_cmd;
    commands[31] = NULL;
    init_strings[0] = CASE;
    init_strings[1] = NOCASE;
    init_strings[2] = EXPAND;
    init_strings[3] = NOEXPAND;
    init_strings[4] = INFO;
    init_strings[5] = NOINFO;
    init_strings[6] = MARGINS;
    init_strings[7] = NOMARGINS;
    init_strings[8] = AUTOFORMAT;
    init_strings[9] = NOAUTOFORMAT;
    init_strings[10] = Echo;
    init_strings[11] = PRINTCOMMAND;
    init_strings[12] = RIGHTMARGIN;
    init_strings[13] = HIGHLIGHT;
    init_strings[14] = NOHIGHLIGHT;
    init_strings[15] = EIGHTBIT;
    init_strings[16] = NOEIGHTBIT;
    init_strings[17] = EMACS_string;
    init_strings[18] = NOEMACS_string;
    init_strings[19] = chinese_cmd;
    init_strings[20] = nochinese_cmd;
    init_strings[21] = NULL;

    for (counter = 1; counter < NUM_MODES_ITEMS; counter++)
        modes_menu[counter].item_string = malloc(80);

#ifndef NO_CATGETS
    catclose(catalog);
#endif /* NO_CATGETS */
}

void command_prompt(void)
{
    char *cmd_str;
    int result;

    info_type = COMMANDS;
    paint_info_win();
    cmd_str = get_string(command_str, TRUE);
    if ((result = unique_test(cmd_str, commands)) != 1) {
        werase(com_win);
        wmove(com_win, 0, 0);
        if (result == 0)
            wprintw(com_win, unkn_cmd_str, cmd_str);
        else
            wprintw(com_win, "%s", non_unique_cmd_msg);

        wrefresh(com_win);

        info_type = CONTROL_KEYS;
        paint_info_win();

        if (cmd_str != NULL)
            free(cmd_str);
        return;
    }
    command(cmd_str);
    wrefresh(com_win);
    wmove(text_win, scr_vert, (scr_horz - horiz_offset));
    info_type = CONTROL_KEYS;
    paint_info_win();
    if (cmd_str != NULL)
        free(cmd_str);
}

