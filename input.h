#ifndef INPUT_H
#define INPUT_H

int collect_input_chunk(int *buf, int max);
void start_action(void);
void control(void);
void emacs_control(void);
void function_key(void);

#endif /* INPUT_H */
