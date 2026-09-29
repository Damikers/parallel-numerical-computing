#include <iostream>
#include <cmath>
#include <omp.h>
#include <vector>
#include <iomanip>
#include <string>

class Function {
public:
    virtual ~Function() {}
    virtual double compute(double x) const = 0;
};


class MyFunction : public Function {
public:
    double compute(double x) const override {
        return sin(2 * x) * cos(3 * x);
    }
};


class IntegralCalculator {
public:
    double calculate(double a, double b, int n, int threads, const Function& f) {
        double h = (b - a) / n;
        double sum = 0.0;

        omp_set_num_threads(threads);

        double start = omp_get_wtime();
        #pragma omp parallel for reduction(+:sum)
        for (int i = 0; i < n; ++i) {
            double x = a + i * h + h / 2.0; // central rectangle method
            sum += f.compute(x);
        }
        double end = omp_get_wtime();

        double result = sum * h;

        std::cout << "Threads: " << threads 
                  << " | Steps: " << n 
                  << " | Result: " << std::setprecision(10) << result
                  << " | Time: " << std::fixed << std::setprecision(6) << (end - start)
                  << " s" << std::endl;

        return result;
    }
};

int main(int argc, char **argv) {
    MyFunction f;
    IntegralCalculator calc;

    double a = 0.0;
    double b = M_PI / 2.0;

    const bool quick = argc > 1 && std::string(argv[1]) == "--quick";
    std::vector<int> stepCounts = quick
        ? std::vector<int>{100, 100000}
        : std::vector<int>{100, 10000000};
    const int maxThreads = quick ? 4 : 20;
    bool valid = true;

    std::cout << "=== Integral of f(t) = sin(2t)*cos(3t) on [0, pi/2] ===" << std::endl;

    for (int n : stepCounts) {
        std::cout << "\n--- Testing with " << n << " intervals ---" << std::endl;
        for (int threads = 1; threads <= maxThreads; ++threads) {
            const double result = calc.calculate(a, b, n, threads, f);
            if (std::fabs(result - (-0.4)) > 1e-3) valid = false;
        }
    }

    std::cout << "Validation: " << (valid ? "PASS" : "FAIL") << std::endl;
    return valid ? 0 : 1;
}
