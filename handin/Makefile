bench_cnf: benchmaker.c $(SRCS)
	gcc -fopenmp -o bench_cnf benchmaker.c parser.c parallel_bruteforce.c sequential.c -lm

bench_par: parallel_bench.c parser.c parallel.c
	gcc -fopenmp -o bench_par parallel_bench.c parser.c parallel.c -lm
