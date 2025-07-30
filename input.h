#ifndef INPUT_H
#define INPUT_H

int collect_input_chunk(int **buf);
extern int collecting_paste;
void start_action(void);
void control(void);
void emacs_control(void);
void function_key(void);
void right(int disp);
char *get_string(const char *prompt, int advance);
void command(char *cmd);

#endif /* INPUT_H */
