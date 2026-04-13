#include <iostream>
#include <numeric>
#include <vector>
#include <cmath>
#include <chrono>
#include <iomanip>
#include <omp.h>

std::vector<std::vector<double>> generateMatrix(int N) {
    std::vector<std::vector<double>> matrix(N, std::vector<double>(N, 1.0));
    for (int i = 0; i < N; ++i) {
        matrix[i][i] = 2.0;
    }
    return matrix;
}

std::vector<double> simple_iteration(
    const std::vector<std::vector<double>>& A,
    const std::vector<double>& b,
    double tau = 0.000005,
    double tol = 1e-7,
    int max_iter = 100000000)
{
    int n = A.size();
    std::vector<double> x(n, 0.0);
    std::vector<double> x_new(n, 0.0);
    double previous_diff = 1.0/0.0;
    
    for (int iter = 0; iter < max_iter; ++iter)
    {
        #pragma omp parallel for
        for (int i = 0; i < n; ++i) {
            double sum = 0.0;
            for (int j = 0; j < n; ++j) {
                sum += A[i][j] * x[j];
            }
            x_new[i] = x[i] - tau * (sum - b[i]);
        }

        double max_diff = 0.0;
        #pragma omp parallel for
        for (int i = 0; i < n; ++i) {
            max_diff = std::max(max_diff, std::abs(x_new[i] - x[i]));
        }
        
        if(std::abs(max_diff) > std::abs(previous_diff)){
            tau = -tau;
        }
        previous_diff = max_diff;
        
        if (max_diff < tol) break;
        x = x_new;
    }
    return x;
}

int main() {
    const int N = 8192;
    std::vector<std::vector<double>> A = generateMatrix(N); 
    std::vector<double> b(N, 17.0);
    
    std::vector<int> thread_counts(40);
    std::iota(thread_counts.begin(),thread_counts.end(),1);
    
    std::cout << "Thread Count | Time (seconds) | Solution norm\n";
    std::cout << "-------------|----------------|--------------\n";
    
    for (int num_threads : thread_counts) {
        omp_set_num_threads(num_threads);
        
        auto start = std::chrono::high_resolution_clock::now();
        auto x = simple_iteration(A, b);
        auto end = std::chrono::high_resolution_clock::now();
        
        std::chrono::duration<double> elapsed = end - start;
        
        double norm = 0.0;
        for (double val : x) {
            norm += val * val;
        }
        norm = std::sqrt(norm);
        
        std::cout << std::setw(12) << num_threads << " | " 
                  << std::setw(14) << std::fixed << std::setprecision(4) << elapsed.count() << " | "
                  << std::setw(13) << std::scientific << std::setprecision(4) << norm << "\n";
    }
    
    return 0;
}