CC = gcc
FLAGS = -Wall -Wextra

main.c: src/main.c
	${CC} src/main.c ${FLAGS} -o vm

clean:
	rm -f vm

.PHONY: all clean