CC = gcc
LD = gcc

CFLAGS  = -Wall -Werror -c
LDFLAGS =

SRC_DIRS = .
INC_DIRS = .
OBJ_DIR = .objs
OUT_DIR = .

SRCS = $(foreach DIR, ${SRC_DIRS}, $(wildcard ${DIR}/*.c))
INCS = $(addprefix -I, ${INC_DIRS})
OBJS = $(patsubst %.c, ${OBJ_DIR}/%.o, ${SRCS})
DEPS = $(patsubst ${OBJ_DIR}/%.o, ${OBJ_DIR}/%.dep, ${OBJS})

VPATH = ${SRC_DIRS}

OUT  = ${OUT_DIR}/test-sort

.PHONY: all clean run

all: ${OUT}

${OUT}: ${OBJS}
	${LD} ${LDFLAGS} -o $@ ${OBJS}

${OBJ_DIR}/%.o: %.c
	$(shell if test ! -d $(dir $@); then mkdir -p $(dir $@); fi)
	${CC} ${CFLAGS} ${INCS} -MMD -MP -MF$(@:%.o=%.dep) -MT$@ -o $@ $<

clean:
	rm -f ${OUT} ${OBJS} ${DEPS}

run:
	@${OUT}

-include ${DEPS}
