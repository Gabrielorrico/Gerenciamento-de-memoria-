
CC = gcc
FLAGS = -Wall -Wextra -pthread

all: vm

vm: src/main.c
	${CC} src/main.c ${FLAGS} -o vm

clean:
	rm -f vm

.PHONY: all clean
