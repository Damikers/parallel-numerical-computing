# Parallel numerical computing in C and C++

KPI Distributed and Cloud Computing coursework focused on shared-memory parallelism, numerical workloads and empirical performance analysis.

## Implementations

- pthread-based synchronised simulation
- OpenMP matrix operations and row calculations
- parallel numerical integration with reduction
- parallel Gaussian elimination
- deterministic quadrature and Monte Carlo estimation of π
- execution-time, speedup and scaling comparisons across thread counts and input sizes

## Structure

```text
src/pthreads/
src/openmp_matrix/
src/parallel_integration/
src/gaussian_elimination/
src/monte_carlo/
report/
```

## Building

The four command-line programs have a shared build and validation target. On Linux with GCC:

```bash
make test
```

On macOS with Homebrew GCC:

```bash
make test OPENMP_CC=gcc-15 OPENMP_CXX=g++-15
```

The test target compiles with strict warnings and runs shortened workloads that verify matrix results, integration accuracy, linear-system solutions and both π estimators. Full benchmark sizes are used when an executable is run without `--quick`. GitHub Actions repeats the command-line tests on Ubuntu.

The pthread simulation uses SDL3, and the graphical integration implementation uses SFML. They are kept as platform-dependent demonstrations; the `lab_3_nogui.cpp` source provides the portable integration benchmark. The graphical integration program also accepts `--test` to run its unit checks without opening a window.

## Important scope note

These projects demonstrate pthread/OpenMP shared-memory parallel computing. They should not be described as an MPI cluster or distributed-service implementation.

## Academic context

KPI parallel-computing laboratory portfolio, 2025. Reports include timing tables and discussion of synchronisation and scaling overhead.
