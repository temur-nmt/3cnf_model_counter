#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <omp.h>
#include <unistd.h>

#include "sequential.h"
// #include "parallel.h"

int main(int argc, char *argv[]) {

    if (argc < 2) {
        printf("ERROR: NO PATH PROVIDED!\n");
        printf("USAGE: ./bench_cnf <path_to_folder_with_tests>\n");
        return 1;
    }

    char* path = argv[1];
    struct timeval startTime, endTime;

    // ---------------------------------------------------------
    // 1. OPENMP WARMUP
    // ---------------------------------------------------------
    int max_procs = omp_get_num_procs();
    #pragma omp parallel for num_threads(max_procs)
    for (int i = 0; i < max_procs; i++);

    int procs = (max_procs >= 32) ? 32 : max_procs;
    omp_set_num_threads(procs);

    // ---------------------------------------------------------
    // 2. PARSING
    // ---------------------------------------------------------
    printf("READING FILES: %s...\n", path);
    Formula* formulae = (Formula*) malloc(20 * sizeof(Formula));
    char file_path[50]; 

    for (int i = 1; i <= 20; i++) {
        snprintf(file_path, sizeof(file_path), "%s/uf20-0%d.cnf", path, i);
        Formula f = parse_dimacs(file_path);
        formulae[i - 1] = f;
    }
    printf("FINISHED PARSING. STARTING BENCHMARK...\n\n");

    // ---------------------------------------------------------
    // 3. BENCHMARKING
    // ---------------------------------------------------------
    printf("%-12s  %8s  %10s  %10s  %7s  %10s\n", "File", "Models", "Seq (us)", "Par (us)", "Speedup", "Efficiency"); 
    printf("----------------------------------------------------------------------\n");

    long total_seq_time = 0;
    long total_par_time = 0;

    for (int i = 0; i < 20; i++) {
        
        // --- A. SEQUENTIAL TEST ---
        gettimeofday(&startTime, NULL);
        int seq_models = run_sequential_counting(&formulae[i]);
        gettimeofday(&endTime, NULL);
        
        long seq_time = endTime.tv_usec - startTime.tv_usec + (endTime.tv_sec - startTime.tv_sec) * 1000000;
        total_seq_time += seq_time;

        usleep(10000);

        // --- B. PARALLEL TEST ---
        gettimeofday(&startTime, NULL);
        // int par_models = run_parallel_counting(&formulae[i], procs); 
        int par_models = seq_models; // DUMMY
        usleep(10000); // DUMMY-TIME
        gettimeofday(&endTime, NULL);
        
        long par_time = endTime.tv_usec - startTime.tv_usec + (endTime.tv_sec - startTime.tv_sec) * 1000000;
        total_par_time += par_time;

        // --- CHECKING THE OUTPUT ---
        if (seq_models != par_models) {
            printf("uf20-%02d.cnf | ERROR: MISMATCH! Seq: %d, Par: %d\n", i + 1, seq_models, par_models);
            return -1;
        }

        // SPEEDUP CALCULATION
        float speedup = (float)seq_time / (float)(par_time ? par_time : 1);
        float efficiency = (speedup / procs) * 100;

        // RESULTS
        printf("uf20-%02d.cnf  %8d  %10ld  %10ld  %6.2fx  %9.0f%%\n", 
               i + 1, seq_models, seq_time, par_time, speedup, efficiency);
    }

    // ---------------------------------------------------------
    // 4. SUMMARY
    // ---------------------------------------------------------
    printf("----------------------------------------------------------------------\n");
    float avg_speedup = (float)total_seq_time / (float)(total_par_time ? total_par_time : 1);
    printf("Average Speedup over all 20 tests: %.2fx\n\n", avg_speedup);

    return 0;
}