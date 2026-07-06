CC = gcc
CFLAGS = -Wall -Wextra -std=c11
SRC = main.c parser.c
OBJ = $(SRC:.c=.o)
TARGET = main

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c header.h
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f $(OBJ) $(TARGET)
