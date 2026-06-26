CC = gcc
CFLAGS = -fopenmp -lm

SRCS = parser_temur.c sequential.c parallel.c

bench_cnf: benchmaker.c $(SRCS)
	$(CC) $(CFLAGS) -o bench_cnf benchmaker.c $(SRCS)

bench_par: parallel_bench.c $(SRCS)
	$(CC) $(CFLAGS) -o parallel_bench.c benchmaker.c $(SRCS)
