#include <stdlib.h>
#include <string.h>
#include "new_curse.h"
#include "text.h"
#include "undo.h"

/* use the globals from ee.c */

extern void draw_screen(void);
extern struct text *txtalloc(void);

#define UNDO_DEPTH 100

static struct snapshot undo_stack[UNDO_DEPTH];
static int undo_pos = 0;
static struct snapshot redo_stack[UNDO_DEPTH];
static int redo_pos = 0;
static int chunk_saved = 1;

static void free_text_list(struct text *t)
{
    while (t != NULL) {
        struct text *n = t->next_line;
        free(t->line);
        free(t);
        t = n;
    }
}

static struct text *clone_text_list(struct text *src, struct text **out_curr,
                                   struct text *orig_curr)
{
    struct text *head = NULL, *prev = NULL, *curr = NULL;
    while (src != NULL) {
        struct text *node = txtalloc();
        node->line = malloc(src->max_length);
        memcpy(node->line, src->line, src->line_length + 1);
        node->line_length = src->line_length;
        node->max_length = src->max_length;
        node->line_number = src->line_number;
        node->prev_line = prev;
        if (prev)
            prev->next_line = node;
        else
            head = node;
        if (src == orig_curr)
            curr = node;
        prev = node;
        src = src->next_line;
    }
    if (prev)
        prev->next_line = NULL;
    if (out_curr)
        *out_curr = curr;
    return head;
}

static struct snapshot take_snapshot(void)
{
    struct snapshot snap;
    snap.first = clone_text_list(first_line, &snap.curr, curr_line);
    snap.position = position;
    snap.absolute_lin = absolute_lin;
    snap.scr_vert = scr_vert;
    snap.scr_horz = scr_horz;
    snap.horiz_offset = horiz_offset;
    return snap;
}

static void apply_snapshot(struct snapshot *snap)
{
    free_text_list(first_line);
    first_line = snap->first;
    curr_line = snap->curr;
    position = snap->position;
    absolute_lin = snap->absolute_lin;
    scr_vert = snap->scr_vert;
    scr_horz = snap->scr_horz;
    horiz_offset = snap->horiz_offset;
    point = curr_line->line + position - 1;
    draw_screen();
}

void undo_init(void)
{
    undo_pos = redo_pos = 0;
    chunk_saved = 1;
}

void undo_push_state(void)
{
    struct snapshot snap = take_snapshot();
    if (undo_pos == UNDO_DEPTH) {
        free_text_list(undo_stack[0].first);
        memmove(&undo_stack[0], &undo_stack[1],
                sizeof(struct snapshot) * (UNDO_DEPTH - 1));
        undo_pos--;
    }
    undo_stack[undo_pos++] = snap;
    while (redo_pos > 0) {
        redo_pos--;
        free_text_list(redo_stack[redo_pos].first);
    }
}

void undo_begin_chunk(void)
{
    if (chunk_saved) {
        undo_push_state();
        chunk_saved = 0;
    }
}

void undo_end_chunk(void)
{
    chunk_saved = 1;
}

void undo_action(void)
{
    if (undo_pos == 0)
        return;
    struct snapshot curr = take_snapshot();
    if (redo_pos == UNDO_DEPTH) {
        free_text_list(redo_stack[0].first);
        memmove(&redo_stack[0], &redo_stack[1],
                sizeof(struct snapshot) * (UNDO_DEPTH - 1));
        redo_pos--;
    }
    redo_stack[redo_pos++] = curr;
    undo_pos--;
    apply_snapshot(&undo_stack[undo_pos]);
}

void redo_action(void)
{
    if (redo_pos == 0)
        return;
    struct snapshot curr = take_snapshot();
    if (undo_pos == UNDO_DEPTH) {
        free_text_list(undo_stack[0].first);
        memmove(&undo_stack[0], &undo_stack[1],
                sizeof(struct snapshot) * (UNDO_DEPTH - 1));
        undo_pos--;
    }
    undo_stack[undo_pos++] = curr;
    redo_pos--;
    apply_snapshot(&redo_stack[redo_pos]);
}

