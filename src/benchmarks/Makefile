CC = gcc
CFLAGS = -fopenmp

SRCS = parser_temur.c sequential.c parallel.c

bench_cnf: benchmaker.c $(SRCS)
	$(CC) $(CFLAGS) -o bench_cnf benchmaker.c $(SRCS) -lm

bench_par: parallel_bench.c $(SRCS)
	$(CC) $(CFLAGS) -o bench_par parallel_bench.c $(SRCS) -lm
