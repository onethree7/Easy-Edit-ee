# EE (Easy Editor) for Linux - Makefile

# ---- Config ----
CC       ?= gcc
CFLAGS   ?= -Wall -Wextra -O2
CPPFLAGS ?=
LDFLAGS  ?= -lncurses
PREFIX   ?= /usr/local
BINDIR   ?= $(PREFIX)/bin
SRC      = ee.c undo.c
BIN      = ee

# ---- Targets ----

all: $(BIN)

$(BIN): $(SRC) new_curse.h ee_version.h
	$(CC) $(CPPFLAGS) $(CFLAGS) $(SRC) -o $@ $(LDFLAGS)

install: $(BIN)
	install -d $(DESTDIR)$(BINDIR)
	install -m 755 $(BIN) $(DESTDIR)$(BINDIR)/$(BIN)

uninstall:
	rm -f $(DESTDIR)$(BINDIR)/$(BIN)

strip: $(BIN)
	strip $(BIN)

clean:
	rm -f $(BIN)

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
