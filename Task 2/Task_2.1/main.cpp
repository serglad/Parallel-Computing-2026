#include <iostream>
#include <omp.h>
#include <vector>
#include <chrono>
#include <iomanip>
#include <random>


void initialize_parallel(std::vector<std::vector<double>>& matrix, std::vector<double>& vector, int m, int n) {
    
    matrix.assign(m, std::vector<double>(n));
    vector.assign(n, 0.0);
    
    #pragma omp parallel for
    for (int i = 0; i < m; i++) {
        for (int j = 0; j < n; j++) {
            matrix[i][j] = 2.0;
        }
    }
    
    #pragma omp parallel for
    for (int i = 0; i<n; i++) {
        vector[i] = 2.0;
    }

}


std::vector<double> parallel_product(const std::vector<std::vector<double>>& matrix, const std::vector<double>& vector, int m, int n) {
    std::vector<double> output(m, 0.0);
    #pragma omp parallel for
    for (int i = 0; i < m; i++) {
        double sum = 0.0;
        for (int j = 0; j < n; j++) {
            sum += matrix[i][j] * vector[j];
        }
        output[i] = sum;
    }
    return output;
}

int measure_time(auto&& func, const std::vector<std::vector<double>>& matrix, const std::vector<double>& vector, int m, int n) {
    auto start = std::chrono::high_resolution_clock::now();
    auto output = func(matrix, vector, m, n);
    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
}

void run_experiment(int m, int n, const std::vector<int>& threads) {
    std::cout << "\n=== Matrix size: " << m << "x" << n << " ===\n";
    
    std::vector<std::vector<double>> matrix;
    std::vector<double> vector;
    initialize_parallel(matrix,vector,m,n);
    for (int t : threads) {
        omp_set_num_threads(t);
        int measured_time = measure_time(parallel_product, matrix, vector, m, n);
        std::cout << "  " << t << " thread(s): " << measured_time 
                  << "ms"<<std::endl;
    }
}

int main() {
    std::vector<int> thread_counts = {1,2, 4, 7, 8,16,20,40};
    run_experiment(20000, 20000, thread_counts);

    run_experiment(40000, 40000, thread_counts);
    
    return 0;
}