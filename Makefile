
INCLUDE_PATH = ./includes/


main: main.c
	gcc -Wall -I${INCLUDE_PATH} -o main ./includes/RS-232/rs232.c dUQx.c main.c libuqeasysocket.a -lm

client: client.c
	gcc -Wall -I${INCLUDE_PATH} -o client ./includes/RS-232/rs232.c dUQx.c client.c libuqeasysocket.a -pthread

clean:
	rm main
	
