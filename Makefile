OPENMP_CC ?= gcc
OPENMP_CXX ?= g++
CFLAGS := -O2 -std=c11 -Wall -Wextra -Wpedantic -fopenmp
CXXFLAGS := -O2 -std=c++17 -Wall -Wextra -Wpedantic -fopenmp
BUILD_DIR := build

.PHONY: all test clean

all: $(BUILD_DIR)/matrix_parallel $(BUILD_DIR)/integration_cli \
	$(BUILD_DIR)/gaussian_elimination $(BUILD_DIR)/pi_estimators

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/matrix_parallel: src/openmp_matrix/matrix_parallel.c | $(BUILD_DIR)
	$(OPENMP_CC) $(CFLAGS) $< -lm -o $@

$(BUILD_DIR)/integration_cli: src/parallel_integration/lab_3_nogui.cpp | $(BUILD_DIR)
	$(OPENMP_CXX) $(CXXFLAGS) $< -o $@

$(BUILD_DIR)/gaussian_elimination: src/gaussian_elimination/gaussian_elimination.cpp | $(BUILD_DIR)
	$(OPENMP_CXX) $(CXXFLAGS) $< -o $@

$(BUILD_DIR)/pi_estimators: src/monte_carlo/monte_carlo.cpp | $(BUILD_DIR)
	$(OPENMP_CXX) $(CXXFLAGS) $< -o $@

test: all
	$(BUILD_DIR)/matrix_parallel --quick
	$(BUILD_DIR)/integration_cli --quick
	$(BUILD_DIR)/gaussian_elimination --quick
	$(BUILD_DIR)/pi_estimators --quick

clean:
	rm -rf $(BUILD_DIR)
