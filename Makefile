MPC=./mpc-0.8.7

all: parsing


parsing:
	gcc -std=c99 -Wall -ledit -lm parsing.c -I${MPC} ${MPC}/mpc.c -o parsing
