#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <pwd.h>
#include <locale.h>
#include <ncursesw/curses.h>
#include <nl_types.h>

#include "config.h"
extern WINDOW *com_win;
extern int clear_com_win;

#include "search.h"

nl_catd catalog;
int info_window = TRUE;
int case_sen;

int expand_tabs = TRUE;
int observ_margins = TRUE;
int auto_format = FALSE;
int right_margin = 0;
int restricted = FALSE;
int nohighlight = FALSE;
int eightbit = TRUE;
int emacs_keys_mode = FALSE;
int ee_chinese = FALSE;
char *print_command = "lpr";
char *start_at_line = NULL;

char *commands[32];
char *init_strings[22];
char *help_text[23];
char *control_keys[5];
char *emacs_help_text[22];
char *emacs_control_keys[5];
char *command_strings[5];
char *init_name[3] = {"/usr/share/misc/init.ee", NULL, ".init.ee"};

char *com_win_message;
char *no_file_string;
char *ascii_code_str;
char *printer_msg_str;
char *command_str;
char *file_write_prompt_str;
char *file_read_prompt_str;
char *char_str;
char *unkn_cmd_str;
char *non_unique_cmd_msg;
char *line_num_str;
char *line_len_str;
char *current_file_str;
char *usage0;
char *usage1;
char *usage2;
char *usage3;
char *usage4;
char *file_is_dir_msg;
char *new_file_msg;
char *cant_open_msg;
char *open_file_msg;
char *file_read_fin_msg;
char *reading_file_msg;
char *read_only_msg;
char *file_read_lines_msg;
char *save_file_name_prompt;
char *file_not_saved_msg;
char *changes_made_prompt;
char *yes_char;
char *file_exists_prompt;
char *create_file_fail_msg;
char *writing_file_msg;
char *file_written_msg;
char *searching_msg;
char *str_not_found_msg;
char *search_prompt_str;
char *exec_err_msg;
char *continue_msg;
char *menu_cancel_msg;
char *menu_size_err_msg;
char *press_any_key_msg;
char *shell_prompt;
char *formatting_msg;
char *shell_echo_msg;
char *spell_in_prog_msg;
char *margin_prompt;
char *restricted_msg;
char *ON;
char *OFF;
char *HELP;
char *WRITE;
char *READ;
char *LINE;
char *FILE_str;
char *CHARACTER;
char *REDRAW;
char *RESEQUENCE;
char *AUTHOR;
char *VERSION;
char *CASE;
char *NOCASE;
char *EXPAND;
char *NOEXPAND;
char *Exit_string;
char *QUIT_string;
char *INFO;
char *NOINFO;
char *MARGINS;
char *NOMARGINS;
char *AUTOFORMAT;
char *NOAUTOFORMAT;
char *Echo;
char *PRINTCOMMAND;
char *RIGHTMARGIN;
char *HIGHLIGHT;
char *NOHIGHLIGHT;
char *EIGHTBIT;
char *NOEIGHTBIT;
char *EMACS_string;
char *NOEMACS_string;
char *conf_dump_err_msg;
char *conf_dump_success_msg;
char *conf_not_saved_msg;
char *ree_no_file_msg;
char *cancel_string;
char *menu_too_lrg_msg;
char *more_above_str, *more_below_str;
char *separator = "============================================================="
                    "==================";
char *chinese_cmd, *nochinese_cmd;

struct menu_entries config_dump_menu[] = {
    {"", NULL, NULL, NULL, NULL, 0},
    {"", NULL, NULL, NULL, NULL, -1},
    {"", NULL, NULL, NULL, NULL, -1},
    {NULL, NULL, NULL, NULL, NULL, -1}
};

char *is_in_string(char *string, char *substring)
{
    char *full, *sub;
    for (sub = substring; (sub != NULL) && (*sub != '\0'); sub++) {
        for (full = string; (full != NULL) && (*full != '\0'); full++) {
            if (*sub == *full)
                return full;
        }
    }
    return NULL;
}

char *resolve_name(char *name)
{
    char long_buffer[1024];
    char short_buffer[128];
    char *buffer;
    char *slash;
    char *tmp;
    char *start_of_var;
    int offset;
    int index;
    int counter;
    struct passwd *user;

    if (name[0] == '~') {
        if (name[1] == '/') {
            index = getuid();
            user = getpwuid(index);
            slash = name + 1;
        } else {
            slash = strchr(name, '/');
            if (slash == NULL)
                return name;
            *slash = '\0';
            user = getpwnam((name + 1));
            *slash = '/';
        }
        if (user == NULL)
            return name;
        buffer = malloc(strlen(user->pw_dir) + strlen(slash) + 1);
        strcpy(buffer, user->pw_dir);
        strcat(buffer, slash);
    } else
        buffer = name;

    if (is_in_string(buffer, "$")) {
        tmp = buffer;
        index = 0;
        while ((*tmp != '\0') && (index < 1024)) {
            while ((*tmp != '\0') && (*tmp != '$') && (index < 1024)) {
                long_buffer[index] = *tmp;
                tmp++;
                index++;
            }
            if ((*tmp == '$') && (index < 1024)) {
                counter = 0;
                start_of_var = tmp;
                tmp++;
                if (*tmp == '{') {
                    tmp++;
                    while ((*tmp != '\0') && (*tmp != '}') && (counter < 128)) {
                        short_buffer[counter] = *tmp;
                        counter++;
                        tmp++;
                    }
                    if (*tmp == '}')
                        tmp++;
                } else {
                    while ((*tmp != '\0') && (*tmp != '/') && (*tmp != '$') &&
                           (counter < 128)) {
                        short_buffer[counter] = *tmp;
                        counter++;
                        tmp++;
                    }
                }
                short_buffer[counter] = '\0';
                if ((slash = getenv(short_buffer)) != NULL) {
                    offset = strlen(slash);
                    if ((offset + index) < 1024)
                        strcpy(&long_buffer[index], slash);
                    index += offset;
                } else {
                    while ((start_of_var != tmp) && (index < 1024)) {
                        long_buffer[index] = *start_of_var;
                        start_of_var++;
                        index++;
                    }
                }
            }
        }
        if (index == 1024)
            return buffer;
        else
            long_buffer[index] = '\0';
        if (name != buffer)
            free(buffer);
        buffer = malloc(index + 1);
        strcpy(buffer, long_buffer);
    }
    return buffer;
}

int restrict_mode(void)
{
    if (!restricted)
        return FALSE;
    wmove(com_win, 0, 0);
    wprintw(com_win, "%s", restricted_msg);
    wclrtoeol(com_win);
    wrefresh(com_win);
    clear_com_win = TRUE;
    return TRUE;
}

int unique_test(char *string, char *list[])
{
    int counter;
    int num_match = 0;
    int result;
    for (counter = 0; list[counter] != NULL; counter++) {
        result = compare(string, list[counter], FALSE);
        if (result)
            num_match++;
    }
    return num_match;
}

void dump_ee_conf(void)
{
    FILE *init_file;
    FILE *old_init_file = NULL;
    char *file_name = ".init.ee";
    char *home_dir = "~/.init.ee";
    char buffer[512];
    struct stat buf;
    char *string;
    int length;
    int option = 0;

    if (restrict_mode())
        return;

    option = menu_op(config_dump_menu);
    werase(com_win);
    wmove(com_win, 0, 0);
    if (option == 0) {
        wprintw(com_win, "%s", conf_not_saved_msg);
        wrefresh(com_win);
        return;
    } else if (option == 2)
        file_name = resolve_name(home_dir);

    if (stat(file_name, &buf) != -1) {
        snprintf(buffer, sizeof(buffer), "%s.old", file_name);
        unlink(buffer);
        if (link(file_name, buffer) != 0)
            perror("link");
        unlink(file_name);
        old_init_file = fopen(buffer, "r");
    }

    init_file = fopen(file_name, "w");
    if (init_file == NULL) {
        wprintw(com_win, "%s", conf_dump_err_msg);
        wrefresh(com_win);
        return;
    }

    if (old_init_file != NULL) {
        while ((string = fgets(buffer, 512, old_init_file)) != NULL) {
            length = strlen(string);
            string[length - 1] = '\0';
            if (unique_test(string, init_strings) == 1) {
                if (compare(string, Echo, FALSE))
                    fprintf(init_file, "%s\n", string);
            } else
                fprintf(init_file, "%s\n", string);
        }
        fclose(old_init_file);
    }

    fprintf(init_file, "%s\n", case_sen ? CASE : NOCASE);
    fprintf(init_file, "%s\n", expand_tabs ? EXPAND : NOEXPAND);
    fprintf(init_file, "%s\n", info_window ? INFO : NOINFO);
    fprintf(init_file, "%s\n", observ_margins ? MARGINS : NOMARGINS);
    fprintf(init_file, "%s\n", auto_format ? AUTOFORMAT : NOAUTOFORMAT);
    fprintf(init_file, "%s %s\n", PRINTCOMMAND, print_command);
    fprintf(init_file, "%s %d\n", RIGHTMARGIN, right_margin);
    fprintf(init_file, "%s\n", nohighlight ? NOHIGHLIGHT : HIGHLIGHT);
    fprintf(init_file, "%s\n", eightbit ? EIGHTBIT : NOEIGHTBIT);
    fprintf(init_file, "%s\n", emacs_keys_mode ? EMACS_string : NOEMACS_string);
    fprintf(init_file, "%s\n", ee_chinese ? chinese_cmd : nochinese_cmd);
    fclose(init_file);
    wprintw(com_win, conf_dump_success_msg, file_name);
    wrefresh(com_win);
    if ((option == 2) && (file_name != home_dir))
        free(file_name);
}

void echo_string(char *string)
{
    char *temp = string;
    int Counter;
    while (*temp != '\0') {
        if (*temp == '\\') {
            temp++;
            if (*temp == 'n')
                putchar('\n');
            else if (*temp == 't')
                putchar('\t');
            else if (*temp == 'b')
                putchar('\b');
            else if (*temp == 'r')
                putchar('\r');
            else if (*temp == 'f')
                putchar('\f');
            else if ((*temp == 'e') || (*temp == 'E'))
                putchar('\033');
            else if (*temp == '\\')
                putchar('\\');
            else if (*temp == '\'')
                putchar('\'');
            else if ((*temp >= '0') && (*temp <= '9')) {
                Counter = 0;
                while ((*temp >= '0') && (*temp <= '9')) {
                    Counter = (8 * Counter) + (*temp - '0');
                    temp++;
                }
                putchar(Counter);
                temp--;
            }
        } else {
            putchar(*temp);
        }
        temp++;
    }
    fflush(stdout);
}

#ifndef NO_CATGETS
char *catgetlocal(int number, char *string)
{
    char *temp1;
    char *temp2;
    temp1 = catgets(catalog, 1, number, string);
    if (temp1 != string) {
        temp2 = malloc(strlen(temp1) + 1);
        strcpy(temp2, temp1);
        temp1 = temp2;
    }
    return temp1;
}
#endif
