# siv version
VERSION = 0.1

# paths
PREFIX    = /usr/local
MANPREFIX = ${PREFIX}/share/man

# flags
CPPFLAGS = -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_POSIX_C_SOURCE=200809L -D_XOPEN_SOURCE=700 -DVERSION=\"${VERSION}\"
CFLAGS   = -std=c99 -pedantic -Wextra -Wall -Wno-unused-parameter -Os ${CPPFLAGS}
LDFLAGS  = -lX11 -ljpeg -lpng -s

# compiler and linker
CC = clang
