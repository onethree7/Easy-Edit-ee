#include "undo.h"
#include "new_curse.h"
#include <stdlib.h>
#include <string.h>
struct text {
    unsigned char *line;
    int line_number;
    int line_length;
    int max_length;
    struct text *next_line;
    struct text *prev_line;
};


/* Externs from ee.c */
extern struct text *first_line;
extern struct text *curr_line;
extern int position;
extern int absolute_lin;
extern int scr_vert;
extern int scr_horz;
extern int horiz_offset;
extern unsigned char *point;
extern int scr_pos;
extern WINDOW *text_win;
extern WINDOW *com_win;
extern WINDOW *info_win;
extern int info_window;
extern struct text *txtalloc(void);

void draw_screen(void);

/* Internal helpers */
static struct snapshot clone_current_state(void);
static void apply_snapshot(struct snapshot *snap);
static void free_text_list(struct text *t);
static struct text *clone_text_list(struct text *src, struct text **out_curr,
                                   struct text *orig_curr);
static void refresh_windows(void);

/* Undo/Redo stacks */
static struct snapshot undo_stack[UNDO_DEPTH];
static int undo_pos;
static struct snapshot redo_stack[UNDO_DEPTH];
static int redo_pos;

/* Chunk tracking */
static enum action_type last_action = ACT_NONE;
static int chunk_saved = 1;
static struct timespec last_input_time;

void undo_init(void)
{
    undo_pos = 0;
    redo_pos = 0;
    chunk_saved = 1;
    last_action = ACT_NONE;
    last_input_time.tv_sec = 0;
    last_input_time.tv_nsec = 0;
}

void reset_chunk(void)
{
    chunk_saved = 0;
}

void note_input(struct timespec now, int buf_len)
{
    long diff_ms = (now.tv_sec - last_input_time.tv_sec) * 1000L +
                   (now.tv_nsec - last_input_time.tv_nsec) / 1000000L;
    if (last_input_time.tv_sec == 0 || diff_ms > 500 || buf_len > 1) {
        last_action = ACT_NONE;
        chunk_saved = 0;
    }
    last_input_time = now;
}

void start_action(enum action_type act)
{
    if (!chunk_saved) {
        push_undo_state();
        chunk_saved = 1;
    }
    last_action = act;
}

void push_undo_state(void)
{
    struct snapshot snap = clone_current_state();
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

void undo_action(void)
{
    last_action = ACT_NONE;
    if (undo_pos == 0)
        return;
    struct snapshot curr = clone_current_state();
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
    last_action = ACT_NONE;
    if (redo_pos == 0)
        return;
    struct snapshot curr = clone_current_state();
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

/* --------- snapshot helpers ---------- */

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

static void free_text_list(struct text *t)
{
    while (t != NULL) {
        struct text *n = t->next_line;
        free(t->line);
        free(t);
        t = n;
    }
}

static struct snapshot clone_current_state(void)
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
    scr_pos = scr_horz;
    draw_screen();
    refresh_windows();
}

static void refresh_windows(void)
{
    touchwin(text_win);
    wrefresh(text_win);
    if (info_window) {
        touchwin(info_win);
        wrefresh(info_win);
    }
    wrefresh(com_win);
}

