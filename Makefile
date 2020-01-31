MPC=./mpc-0.8.7
INC= parsing.h ${MPC}/mpc.h
SRC= parsing.c prompt.c ${MPC}/mpc.c 

all: parsing


parsing: ${INC} ${SRC}
	gcc -std=c99 -Wall -ledit -lm -I${MPC} ${SRC} -o lispy
