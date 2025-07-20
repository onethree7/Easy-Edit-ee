# ee (Easy Editor) – Linux Port

Minimal port of BSD `ee` (Easy Editor) for Linux. Based on the [FreeBSD source tree](https://cgit.freebsd.org/src/tree/contrib/ee). Just works on Linux, uses `ncurses`.

This exists because I kept typing `ee` out of habit. Something in this editor is charming.

## Requirements

- GCC (or any C99-compatible compiler)
- `libncurses-dev & build-essentials`

## Installation

Clone the repo and build:

```sh
git clone https://github.com/onethree7/Easy-Edit-ee.git
cd Easy-Edit-ee
sudo apt-get install build-essential libncurses-dev
make
strip ee
sudo mv ee /usr/local/bin/
```

This puts `ee` in your `$PATH` for all users.

## Usage

Typical invocation:

```sh
ee filename.txt
```

See the built-in help (`help` command or `^c`) for keybindings.

## Features

- Simple, terminal-based text editor
- Minimal dependencies (`ncurses` only)
- Portable codebase for basic editing needs
- Retains much of the classic BSD behavior and keymap
- Supports undo and redo (`Ctrl+_` and `Ctrl+Shift+_` by default)

## Porting Notes

This port is a “dirty” but functional adaptation. Changes:
- Fixed `ncurses` header usage for modern distros
- Made minimal edits for compiler compatibility (GCC/Clang)
- No changes to upstream logic unless required for build/run

Refer to the original [FreeBSD ee sources](https://cgit.freebsd.org/src/tree/contrib/ee) for clean reference.

## Licensing

Original copyright:
BSD-2-Clause license (see FreeBSD license)
Modifications: retain BSD licensing, attribute upstream and downstream authors

## Disclaimer

This port is provided as-is and for my personal use/fun.

---

Upstream: [https://cgit.freebsd.org/src/tree/contrib/ee](https://cgit.freebsd.org/src/tree/contrib/ee)
Ported for Linux by [Florian Kefferpuetz]
