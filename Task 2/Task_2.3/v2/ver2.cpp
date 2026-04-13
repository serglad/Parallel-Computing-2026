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
    double tol = 1e-5,
    int max_iter = 100000000)
{
    int n = A.size();
    std::vector<double> x(n, 0.0);
    std::vector<double> x_new(n, 0.0);
    
    int converged=0;
    double diff_norm_sq = 0.0;
    double x_new_norm_sq = 0.0;
    #pragma omp parallel shared(x,x_new,converged)
    {
        for (int iter = 0; iter < max_iter&&!converged; ++iter)
    {
        #pragma omp for schedule(static)
        for (int i = 0; i < n; ++i) {
            double sum = 0.0;
            for (int j = 0; j < n; ++j) {
                sum += A[i][j] * x[j];
            }
            x_new[i] = x[i] - tau * (sum - b[i]);
        }
        #pragma omp single
        {
            diff_norm_sq = 0.0;
            x_new_norm_sq = 0.0;
        }
        #pragma omp for reduction(+:diff_norm_sq, x_new_norm_sq)
        for (int i = 0; i < n;++i) {
            double d = x_new[i] - x[i];
            diff_norm_sq += d * d;
            x_new_norm_sq += x_new[i] * x_new[i];
        }
            
        #pragma omp single
        {
            double diff_norm = std::sqrt(diff_norm_sq);
            double x_new_norm = std::sqrt(x_new_norm_sq);
              
            if (diff_norm < tol * x_new_norm) {
                converged = 1;           
            }
            x = x_new;
        }
    }
    }
    
    return x;
}

int main() {
    const int N = 5000;
    std::vector<std::vector<double>> A = generateMatrix(N); 
    std::vector<double> b(N, N+1);
    
    int thread_counts[] = {1,2, 4, 7, 8, 16, 20, 40,80};
    
    std::cout << "Thread Count | Time (seconds)\n";
    std::cout << "-------------|---------------\n";
    
    for (int num_threads : thread_counts) {
        omp_set_num_threads(num_threads);
        
        auto start = std::chrono::high_resolution_clock::now();
        auto x = simple_iteration(A, b);
        auto end = std::chrono::high_resolution_clock::now();
        
        std::chrono::duration<double> elapsed = end - start;
        
        std::cout << std::setw(12) << num_threads << " | " 
                  << std::setw(14) << std::fixed << std::setprecision(4) << elapsed.count() << std::endl;
    }
    
    return 0;
}