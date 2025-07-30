# Proposed Module Layout

This project currently relies on a single `ee.c` file. That source mixes
initialization, text buffer logic, screen drawing and command handling in one
place. The following outline describes a split into focused modules so future
contributions can be smaller and easier to maintain.

## Why Split?

- **Separation of concerns** – keeping unrelated features in separate files makes
  navigation and review simpler.
- **Build clarity** – each module can be compiled on its own and listed in the
  Makefile.
- **Easier collaboration** – contributors can work on different areas without
  touching the same monolithic file.

## File Structure

```
editor.c        --> main(), run_editor(), ee_init(), strings_init(),
                    set_up_term(), command_prompt(), high-level loop
buffer.c        --> insert(), delete(), insert_line(), del_line(), resiz_line()
fileio.c        --> get_file(), write_file(), get_line()
screen.c        --> draw_line(), redraw(), paint_info_win(), midscreen()
search.c        --> search(), search_prompt(), next_word(), prev_word(),
                    goto_line(), scan_w()
menu.c          --> paint_menu(), menu_op(), modes_op(), help(), modes_menu data
input.c         --> control(), emacs_control(), function_key(),
                    collect_input_chunk(), start_action()
config.c        --> ee_version, strings for menus, reading .init.ee,
                    dump_ee_conf(), unique_test()
undo.c (existing) --> undo_*() (already split)
new_curse.c/.h  --> remains as provided
text.h          --> core text buffer structures
```

### Completed Splits

- `buffer.c` extracted from `ee.c`
- `fileio.c` extracted from `ee.c`
- `screen.c` extracted from `ee.c`
- `editor.c` extracted from `ee.c`
- `search.c` extracted from `ee.c`

## Workflow Diagram

```
+-------------+      init       +------------+
|  editor.c   |---------------> |  config.c  |
+-------------+                 +------------+
      |                                |
      | calls                          | loads .init.ee, sets globals
      v                                v
+-------------+      updates   +------------+
|  input.c    |--------------->|  buffer.c  |
+-------------+                +------------+
      |                              |
      | triggers redraw via          | modifies text structures
      v                              v
+-------------+      uses      +------------+
|  screen.c   |<-------------- |  undo.c    |
+-------------+                +------------+
      ^                              ^
      | search / menu UI             |
      |                               \
      |           +-----------+        \
      +---------->| search.c  |<--------+
                  +-----------+
```

This layout keeps text manipulation, display logic and command processing
separate. As modules grow, remember to update this document and the Makefile.
