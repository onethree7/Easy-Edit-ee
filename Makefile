# EE (Easy Editor) for Linux - Makefile

# ---- Config ----
CC       ?= gcc
CFLAGS   ?= -Wall -Wextra -O2 -std=c99
CPPFLAGS ?=
LDFLAGS  ?= -lncursesw
PREFIX   ?= /usr/local
BINDIR   ?= $(PREFIX)/bin
SRC      = editor.c ee.c buffer.c fileio.c screen.c search.c undo.c
OBJ      = $(SRC:.c=.o)
BIN      = ee

# ---- Targets ----

all: $(BIN)

$(BIN): $(OBJ)
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ $(OBJ) $(LDFLAGS)

%.o: %.c new_curse.h ee_version.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $<

install: $(BIN)
	install -d $(DESTDIR)$(BINDIR)
	install -m 755 $(BIN) $(DESTDIR)$(BINDIR)/$(BIN)

uninstall:
	rm -f $(DESTDIR)$(BINDIR)/$(BIN)

strip: $(BIN)
	strip $(BIN)

clean:
	rm -f $(BIN) $(OBJ)

check:
	@which ncurses5-config 1>/dev/null 2>&1 || which ncursesw6-config 1>/dev/null 2>&1 || echo "WARN: ncurses[-dev] not found, install it if build fails!"

help:
	@echo "make            - build ee"
	@echo "make install    - install to $(BINDIR) (use DESTDIR=... for packaging)"
	@echo "make uninstall  - remove binary"
	@echo "make clean      - cleanup build files"
	@echo "make strip      - strip debug info from binary"
	@echo "make check      - checks ncurses"
	@echo "make help       - show this help"

.PHONY: all install uninstall strip clean check help
