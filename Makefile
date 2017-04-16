.PHONY: all

INCLUDE_PATH = ./includes/

all: client.c
	gcc -Wall -I${INCLUDE_PATH} -o client ./includes/RS-232/rs232.c dUQx.c client.c libuqeasysocket.a -pthread


main: main.c
	gcc -Wall -I${INCLUDE_PATH} -o main main.c libuqeasysocket.a -lm

clean:
	rm main
	