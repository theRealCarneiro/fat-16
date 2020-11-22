CC = gcc
CCFLAGS= 
SRC_PATH = src/

TARGET = fat_shell
FILES = fat_shell.c fat.c
SRC = ${FILES:%.c=$(SRC_PATH)%.c}
OBJ = ${FILES:.c=.o}

${TARGET}:	.c.o	${OBJ}
	${CC} ${OBJ} -o $@ ${CCFLAGS}

.c.o:
	${CC} -c ${SRC}

${OBJ}:	headers/fat_shell.h headers/fat.h

clean:
	rm -rf *.o ${TARGET}

.PHONY: clean 
