#include <iostream>
#include <omp.h>
#include <vector>
#include <chrono>
#include <iomanip>
#include <random>

using Matrix = std::vector<std::vector<double>>;
using Vector = std::vector<double>;

void initialize_parallel(Matrix& matrix, Vector& input, int m, int n) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<double> dist(0.0, 1.0);
    
    matrix.assign(m, Vector(n));
    input.assign(n, 0.0);
    
    // Parallel matrix initialization
    #pragma omp parallel
    {
        // Thread-local random generator for better performance
        std::mt19937 local_gen(rd() ^ (omp_get_thread_num() << 1));
        std::uniform_real_distribution<double> local_dist(0.0, 1.0);
        
        int nthreads = omp_get_num_threads();
        int threadid = omp_get_thread_num();
        int rows_per_thread = m / nthreads;
        int lb = threadid * rows_per_thread;
        int ub = (threadid == nthreads - 1) ? (m - 1) : (lb + rows_per_thread - 1);
        
        // Initialize matrix rows in parallel
        for (int i = lb; i <= ub; i++) {
            for (int j = 0; j < n; j++) {
                matrix[i][j] = local_dist(local_gen);
            }
        }
    }
    
    // Parallel vector initialization
    #pragma omp parallel
    {
        std::mt19937 local_gen(rd() ^ (omp_get_thread_num() << 1));
        std::uniform_real_distribution<double> local_dist(0.0, 1.0);
        
        int nthreads = omp_get_num_threads();
        int threadid = omp_get_thread_num();
        int items_per_thread = n / nthreads;
        int lb = threadid * items_per_thread;
        int ub = (threadid == nthreads - 1) ? (n - 1) : (lb + items_per_thread - 1);
        
        for (int j = lb; j <= ub; j++) {
            input[j] = local_dist(local_gen);
        }
    }
}

Vector serial_product(const Matrix& matrix, const Vector& input, int m, int n) {
    Vector output(m, 0.0);
    for (int i = 0; i < m; i++) {
        double sum = 0.0;
        for (int j = 0; j < n; j++) {
            sum += matrix[i][j] * input[j];
        }
        output[i] = sum;
    }
    return output;
}

Vector parallel_product(const Matrix& matrix, const Vector& input, int m, int n) {
    Vector output(m, 0.0);
    #pragma omp parallel
    {
        int nthreads = omp_get_num_threads();
        int threadid = omp_get_thread_num();
        int items_per_thread = m / nthreads;
        int lb = threadid * items_per_thread;
        int ub = (threadid == nthreads - 1) ? (m - 1) : (lb + items_per_thread - 1);
        
        for (int i = lb; i <= ub; i++) {
            double sum = 0.0;
            for (int j = 0; j < n; j++) {
                sum += matrix[i][j] * input[j];
            }
            output[i] = sum;
        }
    }
    return output;
}

double measure_time(auto&& func, const Matrix& matrix, const Vector& input, int m, int n) {
    auto start = std::chrono::high_resolution_clock::now();
    auto output = func(matrix, input, m, n);
    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration<double>(end - start).count();
}

void run_experiment(int m, int n, const std::vector<int>& threads) {
    std::cout << "\n=== Matrix size: " << m << "x" << n << " ===\n";
    
    Matrix matrix;
    Vector input;
    
    // Measure initialization time
    std::cout << "Initializing with " << omp_get_max_threads() << " threads...\n";
    auto init_start = std::chrono::high_resolution_clock::now();
    initialize_parallel(matrix, input, m, n);
    auto init_end = std::chrono::high_resolution_clock::now();
    double init_time = std::chrono::duration<double>(init_end - init_start).count();
    std::cout << "Initialization time: " << std::fixed << std::setprecision(4) 
              << init_time << " seconds\n\n";
    
    // Serial execution
    double serial_time = measure_time(serial_product, matrix, input, m, n);
    std::cout << std::fixed << std::setprecision(4);
    std::cout << "Serial:    " << serial_time << " seconds\n";
    std::cout << "Parallel:\n";
    
    // Parallel execution with different thread counts
    for (int t : threads) {
        omp_set_num_threads(t);
        double parallel_time = measure_time(parallel_product, matrix, input, m, n);
        double speedup = serial_time / parallel_time;
        double efficiency = (speedup / t) * 100.0;
        std::cout << "  " << t << " thread(s): " << parallel_time 
                  << " seconds (speedup: " << std::setprecision(2) << speedup << "x, "
                  << "efficiency: " << std::setprecision(1) << efficiency << "%)\n";
    }
}

int main() {
    std::vector<int> thread_counts = {1,2, 4, 7, 8,16,20,40};
    
    std::cout << "Matrix-Vector Multiplication Performance Test\n";
    std::cout << "Data type: double, Random values in [0,1]\n";
    std::cout << "Parallel initialization enabled\n";
    
    // Test with 20000x20000
    run_experiment(20000, 20000, thread_counts);
    
    // Test with 40000x40000
    //run_experiment(40000, 40000, thread_counts);
    
    return 0;
}