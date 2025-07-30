#ifndef UNDO_H
#define UNDO_H

struct text;

struct snapshot {
    struct text *first;
    struct text *curr;
    int position;
    int absolute_lin;
    int scr_vert;
    int scr_horz;
    int horiz_offset;
};

void undo_begin_chunk(void);
void undo_end_chunk(void);
void undo_push_state(void);
void undo_action(void);
void redo_action(void);

#endif /* UNDO_H */
