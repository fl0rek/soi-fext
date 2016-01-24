#tools makefile
CFLAGS_POST=-Wall -ggdb

OBJ=../fext_utils2.o common.o
INCLUDE_DIRS=-I../include

CFLAGS=$(CFLAGS_POST) $(INCLUDE_DIRS)

all: mkfs ls cp df

common.o: common.c
	$(CC) $(CFLAGS) common.c -c -o common.o

mkfs: mkfs.c $(OBJ)
	$(CC) $(CFLAGS) mkfs.c $(OBJ) -o mkfs

ls: ls.c $(OBJ)
	$(CC) $(CFLAGS) ls.c $(OBJ) -o ls

cp: cp.c $(OBJ)
	$(CC) $(CFLAGS) cp.c $(OBJ) -o cp

df: df.c $(OBJ)
	$(CC) $(CFLAGS) df.c $(OBJ) -o df

clean:
	rm -f mkfs ls common.o cp

.PHONY: all clean
