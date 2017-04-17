
INCLUDE_PATH = ./includes/


all: main.c
	gcc -Wall -I${INCLUDE_PATH} -o main ./includes/RS-232/rs232.c dUQx.c main.c libuqeasysocket.a -lm

clean:
	rm main
