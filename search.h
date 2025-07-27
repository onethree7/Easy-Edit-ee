#ifndef SEARCH_H
#define SEARCH_H

#include "text.h"

int compare(char *string1, char *string2, int sensitive);
void goto_line(char *cmd_str);
int search(int display_message);
void search_prompt(void);
int scan_w(ee_char *line, int offset, int column);

#endif /* SEARCH_H */
