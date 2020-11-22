CC = gcc
CCFLAGS= 

TARGET = fat_shell
SRC = fat_shell.c fat.c
OBJ = ${SRC:.c=.o}

${TARGET}:	${OBJ}
	${CC} ${OBJ} -o $@ ${CCFLAGS}

.c.o:
	${CC} -c ${SRC}

${OBJ}:	fat_shell.h fat.h

clean:
	rm -rf *.o ${TARGET}

.PHONY: clean 
