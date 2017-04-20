# Author: Julio César Echeverri Marulanda (c) 2017

INCLUDE_PATH = ./include/
OBJS = utils.o dUQx.o rs232.o
SRC  = ./src/

all: ${SRC}main.c ${OBJS}
	gcc -Wall -I${INCLUDE_PATH} -o client $^ libuqeasysocket.a -lm

dUQx.o: ${SRC}dUQx.c
	gcc -Wall -I${INCLUDE_PATH} -c $< -o $@

utils.o: ${SRC}utils.c
	gcc -Wall -I${INCLUDE_PATH} -c $< -o $@
	
rs232.o: ${SRC}rs232.c
	gcc -Wall -I${INCLUDE_PATH} -c $< -o $@

clean:
	rm client ${OBJS}
