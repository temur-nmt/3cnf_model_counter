CC = gcc
CFLAGS = -Wall -Wextra -g -Iparsers -Isequential

seq_bruteforce: main.c parsers/parser_temur.c sequential/sequential_bruteforce.c
	$(CC) $(CFLAGS) -o seq_bruteforce main.c parsers/parser_temur.c sequential/sequential_bruteforce.c

par_temur: main.c parsers/parser_temur.c parallel/parallel_temur.c sequential/sequential_bruteforce.c
	$(CC) $(CFLAGS) -o par_temur main.c parsers/parser_temur.c parallel/parallel_temur.c sequential/sequential_bruteforce.c