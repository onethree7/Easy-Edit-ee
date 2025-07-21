#ifndef EE_UNDO_H
#define EE_UNDO_H

#include <stdbool.h>
#include <time.h>

struct text;

#define UNDO_DEPTH 1000

struct snapshot {
    struct text *first;
    struct text *curr;
    int position;
    int absolute_lin;
    int scr_vert;
    int scr_horz;
    int horiz_offset;
};

enum action_type {
    ACT_NONE = 0,
    ACT_INSERT,
    ACT_DELETE,
    ACT_DEL_WORD,
    ACT_UNDEL_WORD,
    ACT_DEL_LINE,
    ACT_UNDEL_LINE
};

void undo_init(void);
void start_action(enum action_type act);
void push_undo_state(void);
void note_input(struct timespec now, int buf_len);
void undo_action(void);
void redo_action(void);
void reset_chunk(void);

#endif /* EE_UNDO_H */
