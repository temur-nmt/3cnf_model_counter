# How to

## Running the parallel implementation
1. `make bench_par` to compile the benchmaker for the paralllel DPLL implementation
2. `./bench_par ./<folder with problem instances B>` to bench against problem instances B

*NOTE: The benchmarker is only setup to read the files from the B problemset. If you want to try problemset A you have to change inside the `parallel_bench` the file names.*

## Running the naive sequential against the naive parallel

1. `make bench_cnf` to compile the benchmaker
2. `./bench_cnf ./<folder with problem instances A>`

# Further Information

For further information. The Pysat test cases and the generator visit the [GitHub](https://github.com/temur-nmt/3cnf_model_counter) Repository.