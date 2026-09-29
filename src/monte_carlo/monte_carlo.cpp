#include <cmath>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <random>
#include <string>
#include <vector>
#include <omp.h>

struct Estimate {
    double pi;
    double seconds;
};

static Estimate midpoint_pi(int samples, int threads) {
    omp_set_num_threads(threads);
    double sum = 0.0;
    const double width = 1.0 / static_cast<double>(samples);
    const double start = omp_get_wtime();

    #pragma omp parallel for reduction(+ : sum) schedule(static)
    for (int i = 0; i < samples; ++i) {
        const double x = (static_cast<double>(i) + 0.5) * width;
        sum += 4.0 / (1.0 + x * x);
    }

    return {width * sum, omp_get_wtime() - start};
}

static Estimate monte_carlo_pi(int samples, int threads) {
    omp_set_num_threads(threads);
    long long inside = 0;
    const double start = omp_get_wtime();

    #pragma omp parallel reduction(+ : inside)
    {
        const int thread_id = omp_get_thread_num();
        const int thread_count = omp_get_num_threads();
        std::mt19937_64 generator(
            UINT64_C(0x9e3779b97f4a7c15) + static_cast<std::uint64_t>(thread_id)
        );
        std::uniform_real_distribution<double> uniform(-1.0, 1.0);

        const int base = samples / thread_count;
        const int count = base + (thread_id < samples % thread_count ? 1 : 0);
        long long local_inside = 0;
        for (int i = 0; i < count; ++i) {
            const double x = uniform(generator);
            const double y = uniform(generator);
            if (x * x + y * y <= 1.0) ++local_inside;
        }
        inside += local_inside;
    }

    return {
        4.0 * static_cast<double>(inside) / static_cast<double>(samples),
        omp_get_wtime() - start
    };
}

static void print_result(
    const char *method,
    int threads,
    int samples,
    const Estimate &estimate
) {
    std::cout << method << '\t' << threads << '\t' << samples << '\t'
              << std::setprecision(10) << estimate.pi << '\t'
              << std::fixed << std::setprecision(6) << estimate.seconds << '\t'
              << std::scientific << std::fabs(estimate.pi - std::acos(-1.0)) << '\n';
}

int main(int argc, char **argv) {
    const bool quick = argc > 1 && std::string(argv[1]) == "--quick";
    const std::vector<int> sample_counts = quick
        ? std::vector<int>{100000, 1000000}
        : std::vector<int>{100000, 1000000, 10000000, 100000000};
    const int max_threads = quick ? 4 : 8;
    bool valid = true;

    std::cout << "method\tthreads\tsamples\tpi\tseconds\tabs_error\n";
    for (int threads = 1; threads <= max_threads; ++threads) {
        for (int samples : sample_counts) {
            const Estimate estimate = midpoint_pi(samples, threads);
            print_result("midpoint", threads, samples, estimate);
            if (std::fabs(estimate.pi - std::acos(-1.0)) > 1e-8) valid = false;
        }
    }

    for (int threads = 1; threads <= max_threads; ++threads) {
        for (int samples : sample_counts) {
            const Estimate estimate = monte_carlo_pi(samples, threads);
            print_result("monte_carlo", threads, samples, estimate);
            if (std::fabs(estimate.pi - std::acos(-1.0)) > 0.03) valid = false;
        }
    }

    std::cout << "Validation: " << (valid ? "PASS" : "FAIL") << '\n';
    return valid ? 0 : 1;
}
