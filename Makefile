CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -fopenmp
SRC = main.c parser.c bruteforce.c dpll.c
OBJ = $(SRC:.c=.o)
TARGET = main

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c header.h
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f $(OBJ) $(TARGET)
