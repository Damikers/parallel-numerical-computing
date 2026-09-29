#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <omp.h>

static int quick_mode = 0;
static int validation_ok = 1;

void task1() {
    printf("----------- Task 1 -----------\n");

#ifdef _OPENMP
    printf("OpenMP is supported. Version: %d\n", _OPENMP);
#else
    printf("OpenMP is not supported.\n");
#endif

    double start_time, end_time, tick;
    start_time = omp_get_wtime();
    end_time = omp_get_wtime();
    tick = omp_get_wtick();

    printf("Time for measuring time: %lf s\n", end_time - start_time);
    printf("Accuracy for measuring time: %lf s\n", tick);

    #pragma omp parallel
    {
        int id = omp_get_thread_num();
        #pragma omp critical
        printf("Hello World from thread %d!\n", id + 1);
    }

    printf("---------------------------\n\n");
}


void task2() {
    printf("----------- Task 2 -----------\n");

    int full_sizes[] = {10, 20, 50, 100, 200, 500, 1000};
    int quick_sizes[] = {32, 64, 128};
    int *sizes = quick_mode ? quick_sizes : full_sizes;
    int num_sizes = quick_mode ? 3 : 7;
    int max_threads = omp_get_num_procs();  //   8
    if (quick_mode && max_threads > 4) max_threads = 4;

    for (int s = 0; s < num_sizes; s++) {
        int N = sizes[s];
        double *A = (double *)malloc(N * N * sizeof(double));
        double *B = (double *)malloc(N * N * sizeof(double));
        double *C = (double *)malloc(N * N * sizeof(double));

        for (int i = 0; i < N; i++)
            for (int j = 0; j < N; j++) {
                A[i * N + j] = i + j;
                B[i * N + j] = i - j;
            }

        printf("\n--- Matrix %dx%d: Multiplication ---\n", N, N);
        printf("Threads |      Time\n");
        printf("------------------------\n");

        for (int threads = 1; threads <= max_threads; threads++) {
            omp_set_num_threads(threads);
            double t1 = omp_get_wtime();

            #pragma omp parallel for shared(A, B, C) collapse(2)
            for (int i = 0; i < N; i++) {
                for (int j = 0; j < N; j++) {
                    double sum = 0.0;
                    for (int k = 0; k < N; k++)
                        sum += A[i * N + k] * B[k * N + j];
                    C[i * N + j] = sum;
                }
            }

            double t2 = omp_get_wtime();
            printf("%3d     | %10.5lf\n", threads, t2 - t1);

            double expected_first = 0.0;
            double expected_last = 0.0;
            for (int k = 0; k < N; ++k) {
                expected_first += (double)k * (double)k;
                expected_last += (double)(N - 1 + k) * (double)(k - (N - 1));
            }
            if (fabs(C[0] - expected_first) > 1e-9 ||
                fabs(C[(N - 1) * N + (N - 1)] - expected_last) > 1e-9) {
                validation_ok = 0;
            }
        }

        free(A);
        free(B);
        free(C);
        printf("\n");
    }

    printf("---------------------------\n\n");
}


void task3() {
    printf("----------- Task 3 -----------\n");

    int full_sizes[] = {10, 20, 50, 100, 200, 500, 1000};
    int quick_sizes[] = {32, 64, 128};
    int *sizes = quick_mode ? quick_sizes : full_sizes;
    int num_sizes = quick_mode ? 3 : 7;
    int max_threads = omp_get_num_procs();
    if (quick_mode && max_threads > 4) max_threads = 4;

    for (int s = 0; s < num_sizes; s++) {
        int N = sizes[s];
        float **a = (float **)malloc(N * sizeof(float *));
        for (int i = 0; i < N; i++)
            a[i] = (float *)malloc(N * sizeof(float));

        for (int i = 0; i < N; i++)
            for (int j = 0; j < N; j++)
                a[i][j] = 1.0f;

        if (N <= 1) {
            printf("\n--- Matrix %dx%d: Sum of each row ---\n", N, N);
            #pragma omp parallel for
            for (int i = 0; i < N; i++) {
                float sum = 0.0f;
                for (int j = 0; j < N; j++)
                    sum += a[i][j];
                #pragma omp critical
                printf("Sum of row %d = %.2f\n", i, sum);
            }
        }

        printf("\n--- Matrix %dx%d: Sum of each row ---\n", N, N);
        printf("Threads |     Sum     |   Time\n");
        printf("----------------------------------------\n");

        for (int threads = 1; threads <= max_threads; threads++) {
            omp_set_num_threads(threads);
            float total = 0.0f;
            double t1 = omp_get_wtime();

            #pragma omp parallel for reduction(+:total)
            for (int i = 0; i < N; i++) {
                float sum = 0.0f;
                for (int j = 0; j < N; j++)
                    sum += a[i][j];
                total += sum;
            }

            double t2 = omp_get_wtime();
            printf("%3d     | %11.2f | %8.5lf\n", threads, total, t2 - t1);
            if (fabsf(total - (float)(N * N)) > 1e-3f) validation_ok = 0;
        }

        for (int i = 0; i < N; i++) free(a[i]);
        free(a);
        printf("\n");
    }

    printf("---------------------------\n\n");
}

int main(int argc, char **argv) {
    quick_mode = argc > 1 && strcmp(argv[1], "--quick") == 0;
    task1();
    task2();
    task3();

    printf("Validation: %s\n", validation_ok ? "PASS" : "FAIL");
    return validation_ok ? 0 : 1;
}
