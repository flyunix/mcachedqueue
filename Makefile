TARGET=mcached
GCC=gcc

INCLUDE=.
LDLIBS=-lpthread
DFLAGS= -g -Werror -UNDEBUG

SRCS=$(wildcard *.c)
OBJS=$(patsubst %.c, %.o, $(SRCS))

${TARGET}:${OBJS}
	${GCC} *o -o $@ ${LDLIBS}

%.o:%.c
	${GCC} ${DFLAGS} -c -o $@ $< -I ${INCLUDE}

clean:
	-rm -rf *.o  ${OBJ} ${TARGET}
