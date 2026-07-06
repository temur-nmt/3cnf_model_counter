# Parallel Model Counting in C

A small research / implementation project for SAT model counting using C and a planned OpenMP-based parallel execution strategy.

## Overview

This repository provides a DIMACS CNF parser and a lightweight model-counting framework in C.
It is designed to read SAT instances from `test_instances/`, parse clauses into an internal formula representation, and serve as the basis for sequential or parallel counting engines.

## What is included

- `main.c` — driver code that enumerates CNF files, parses them, and prints clause sets when debugging is enabled.
- `parser.c` — DIMACS parser that supports comment lines (`c`), header lines (`p cnf`), and clause termination.
- `header.h` — shared data structures and helper macros.
- `bruteforce.c` — placeholder for a brute-force model counting implementation.
- `dpll.c` — placeholder for a DPLL-based counting engine.
- `test_instances/` — example DIMACS files for testing.
- `Makefile` — build helper for compiling the project.

## Building

### Standard C build

From the project root:

```sh
make main
```

This compiles `main.c` and `parser.c` into the executable `main`.

### OpenMP / parallel build

The current implementation is primarily sequential, but the architecture is intended to support OpenMP-based parallel counting.
Once OpenMP support is added, compile with:

```sh
gcc -Wall -Wextra -std=c11 -fopenmp -o main main.c parser.c bruteforce.c dpll.c
```

Then update `main.c`, `bruteforce.c`, or `dpll.c` to use OpenMP pragmas for parallel work distribution.

## Usage

Run the executable from the project root:

```sh
./main
```

The program currently scans all non-hidden files in `test_instances/`, parses them, and prints concise results by default. Enable verbose parsing output by setting `DEBUG` in `header.h`.

Console output
---------------

The driver prints concise, one-line-per-result summaries that are easy to parse
or grep. Current format:

- `<path>: sequential count = <N> (time <secs> s)`
- `<path>: parallel count = <N> with <threads> threads (time <secs> s)`

Example:

```
test_instances/uf20-019.cnf: sequential count = 2 (time 0.009448 s)
test_instances/uf20-019.cnf: parallel count = 2 with 4 threads (time 0.008266 s)
```

## Input format

The parser expects standard DIMACS CNF input:

- comment lines starting with `c`
- one header line starting with `p cnf <num-vars> <num-clauses>`
- clause lines ending with `0`

Example:

```cnf
c example formula
p cnf 3 2
1 -2 3 0
-1 2 0
```

## File structure

- `header.h` — shared definitions for `Formula`, `Clause`, and I/O helpers
- `main.c` — top-level driver and file enumeration
- `parser.c` — DIMACS parser implementation
- `bruteforce.c` — planned sequential brute-force counting
- `dpll.c` — planned DPLL-based counting and conflict-driven improvements
- `test_instances/` — CNF test files

## Future work

Possible next steps:

- implement sequential model counting in `bruteforce.c`
- implement DPLL / cube enumeration in `dpll.c`
- add OpenMP parallelism for clause evaluation and search splitting
- add command-line options for input directory, output directory, and debug control
- add tests and a simple results summary output

## Notes

 - `DEBUG` is disabled by default in `header.h`; enable it for more verbose logging.
- `INPUT_DIR` is currently fixed to `test_instances`; consider making it configurable.
- The current parser is robust to leading comments and ignores `%` clauses termination.

## DPLL & Hybrid counting plan

Planned approach to support larger CNFs and scale beyond single-word bitmask limits:

- Component decomposition: find variable-connected components (union-find or BFS). The model count of the full formula is the product of component counts.
- Cutoff: if a component has <= 16 variables, enumerate it with the fast brute-force (`bruteforce.c`). 16 variables => 2^16 = 65k assignments.
- DPLL for larger components: implement a recursive DPLL-style counter with:
	- unit propagation and pure-literal elimination,
	- simple branching heuristic (e.g. highest-occurrence variable),
	- component detection during search to multiply independent subcounts,
	- optional caching of simplified-subformula -> count to avoid repeated work.
- Parallelism: run independent components in parallel first. Inside DPLL, use OpenMP tasks for branching at shallow depths (limit tasking depth to avoid overhead).

This hybrid strategy allows continuing to use the fast 64-bit brute-force where it is best, and fall back to DPLL+cache for larger, structured components.

Next implementation steps:

- implement union-find component extraction and the hybrid `count_formula` runner,
- implement a minimal DPLL with unit propagation and caching, then optimize (watch lists, better heuristics) as needed.
