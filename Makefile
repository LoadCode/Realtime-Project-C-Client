
CC=gcc
LD=g++
CFLAGS= -fopenmp -c -Wall -I./ -I../dUQx -I../libs/libconio -I../libs/uqeasysocket-dist
LDFLAGS= -L../libs/libconio -L../libs/uqeasysocket-dist  -fopenmp -lm
LIBS=  -lrt -lpthread -lconio -lncurses -luqeasysocket
SOURCES= dUQxPlants.c  rs232.c utils.c

SOURCES = $(wildcard *.c) $(wildcard ../dUQx/*.c)  $(wildcard ../dUQx/RS-232/*.c)

OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=dUQxPlants

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(LD) $(LDFLAGS) -o $@ $(OBJECTS) $(LIBS)

.c.o:
	$(CC) $(CFLAGS) $< -o $@
clean:
	$(RM) $(OBJECTS) $(EXECUTABLE)


