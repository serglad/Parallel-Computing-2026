#include <random>
#include <thread>
#include <algorithm>
#include <iostream>
#include <chrono>
using matrix = std::vector<std::vector<double>>;
void fill_rows(matrix::iterator begin,matrix::iterator end,double val,int column_count){
    std::vector<double> row(column_count,val);
    std::fill(begin,end,row);
    return;
}
void multiply_partial(int i_start, int i_end,matrix&in,std::vector<double>& vector,matrix& out,int m,int n){
    for(;i_start<=i_end;++i_start){
        for(int j = 0;j<m;j++){
            for(int k=0;k<n;k++){
                out[i_start][j] += in[i_start][k] * vector[k];
            }
        }
    }
}
void parallel_init(std::vector<std::vector<double>>& matrix, std::vector<double>& vector, int m, int n,int thread_count){
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<double> dist(0.0, 1.0);
    std::vector<std::jthread> threads;
    int rows_per_thread=(m/thread_count)+1;
    for(int i=0;i<m;i+= rows_per_thread){
        threads.emplace_back(fill_rows,matrix.begin()+i,std::min(matrix.begin() + i + rows_per_thread, matrix.end()),dist(gen),n);
    }
    vector.assign(vector.size(),dist(gen));
    return;
}

void run(std::vector<std::vector<double>>& matrix, std::vector<double>& vector,int m, int n,int thread_count){
    std::vector<std::vector<double>>matrix_out(m,std::vector<double>(m,0));
    int rows_per_thread=(m/thread_count)+1;
    std::vector<std::jthread> threads;
    for(int i=0;i<m;i+=rows_per_thread){
        threads.emplace_back(multiply_partial,i,std::min(i+rows_per_thread,m-1),std::ref(matrix),std::ref(vector),std::ref(matrix_out),m,n);
    }
}
int main(void){
    std::vector<int> dimensions = {20000, 40000};
    std::vector<int> thread_counts = {1, 2, 4, 7, 8, 16, 20, 40};
    const int num_runs = 100;
    
    for(int dim : dimensions) {
        int m = dim;
        int n = dim;
        std::cout << m << " X " << n << std::endl;
        
        for(int thread_count : thread_counts) {
            double total_time = 0.0;
            
            for(int i = 0; i < num_runs; ++i) {
                matrix matrix_in(m);
                std::vector<double> vector_in(n);
                parallel_init(matrix_in, vector_in, m, n, thread_count);
                
                auto start = std::chrono::high_resolution_clock::now();
                run(matrix_in, vector_in, m, n, thread_count);
                auto end = std::chrono::high_resolution_clock::now();
                
                std::chrono::duration<double> elapsed = end - start;
                total_time += elapsed.count();
            }
            
            double avg_time = total_time / num_runs;
            std::cout << "Threads: " << thread_count << " - Average time: " << avg_time << " seconds" << std::endl;
        }
    }
    
    return 0;
}