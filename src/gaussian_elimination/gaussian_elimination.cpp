#include <algorithm>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
#include <omp.h>

struct RunResult {
    double seconds;
    double max_error;
};

static std::vector<double> solve_gaussian(
    std::vector<std::vector<double>> matrix,
    int threads
) {
    const int n = static_cast<int>(matrix.size());
    omp_set_num_threads(threads);

    for (int pivot = 0; pivot < n; ++pivot) {
        int best_row = pivot;
        for (int row = pivot + 1; row < n; ++row) {
            if (std::fabs(matrix[row][pivot]) > std::fabs(matrix[best_row][pivot])) {
                best_row = row;
            }
        }
        if (std::fabs(matrix[best_row][pivot]) < 1e-14) {
            throw std::runtime_error("singular matrix");
        }
        std::swap(matrix[pivot], matrix[best_row]);

        const double diagonal = matrix[pivot][pivot];
        for (int column = pivot; column <= n; ++column) {
            matrix[pivot][column] /= diagonal;
        }

        #pragma omp parallel for schedule(static)
        for (int row = pivot + 1; row < n; ++row) {
            const double factor = matrix[row][pivot];
            for (int column = pivot; column <= n; ++column) {
                matrix[row][column] -= factor * matrix[pivot][column];
            }
        }
    }

    std::vector<double> solution(n, 0.0);
    for (int row = n - 1; row >= 0; --row) {
        solution[row] = matrix[row][n];
        for (int column = row + 1; column < n; ++column) {
            solution[row] -= matrix[row][column] * solution[column];
        }
    }
    return solution;
}

static RunResult run_test(int n, int threads) {
    std::vector<std::vector<double>> augmented(
        n, std::vector<double>(static_cast<std::size_t>(n) + 1, 0.0)
    );

    // A deterministic, strictly diagonally dominant system with x = 1.
    for (int row = 0; row < n; ++row) {
        double rhs = 0.0;
        for (int column = 0; column < n; ++column) {
            double value = static_cast<double>((row * 17 + column * 13) % 11 - 5) / 100.0;
            if (row == column) value += static_cast<double>(n);
            augmented[row][column] = value;
            rhs += value;
        }
        augmented[row][n] = rhs;
    }

    const double start = omp_get_wtime();
    const std::vector<double> solution = solve_gaussian(std::move(augmented), threads);
    const double end = omp_get_wtime();

    double max_error = 0.0;
    for (double value : solution) {
        max_error = std::max(max_error, std::fabs(value - 1.0));
    }
    return {end - start, max_error};
}

int main(int argc, char **argv) {
    const bool quick = argc > 1 && std::string(argv[1]) == "--quick";
    const std::vector<int> sizes = quick
        ? std::vector<int>{32, 64, 128}
        : std::vector<int>{10, 20, 50, 100, 200, 500, 1000};
    const int max_threads = quick ? 4 : 8;
    bool valid = true;

    std::cout << "threads\tsize\tseconds\tmax_error\n";
    for (int threads = 1; threads <= max_threads; ++threads) {
        for (int n : sizes) {
            const RunResult result = run_test(n, threads);
            std::cout << threads << '\t' << n << '\t'
                      << std::fixed << std::setprecision(6) << result.seconds << '\t'
                      << std::scientific << result.max_error << '\n';
            if (result.max_error > 1e-9) valid = false;
        }
    }

    std::cout << "Validation: " << (valid ? "PASS" : "FAIL") << '\n';
    return valid ? 0 : 1;
}
