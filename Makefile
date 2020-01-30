
all: lispy

lispy: prompt.c
	gcc -std=c99 -Wall -ledit -lm prompt.c -o lispy


