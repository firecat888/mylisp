MPC=./mpc-0.8.7
INC= parsing.h lval.h ${MPC}/mpc.h
SRC= parsing.c prompt.c ${MPC}/mpc.c evaluation.c lval.c

all: lispy_app


lispy_app: ${INC} ${SRC}
	gcc -std=c99 -Wall -ledit -lm -I${MPC} ${SRC} -o lispy
