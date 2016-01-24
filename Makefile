CC=cc
CFLAGS_POST=-Wall -ggdb

INCLUDE_DIRS=-I./include

CFLAGS=$(INCLUDE_DIRS) $(CLFAGS_POST)

all: fext_utils2.o tools

dynamic_headers: fext_utils2.c
	makeheaders `find . -not -empty -iname \*.c -not -path "./legacy/*"  | tr '\n' ' '`

clean_headers:
	rm -i `find . -not -empty -iname \*.h -not -path "./legacy/*"  | tr '\n' ' '`

fext_utils2.o: fext_utils2.c fext_utils2.h
	$(CC) $(CFLAGS) fext_utils2.c -c -o fext_utils2.o

tools:
	$(MAKE) -C tools

clean:
	rm -f *.so
	rm -f *.o
	$(MAKE) -C tools clean

.PHONY: dynamic_headers clean_headers tools
