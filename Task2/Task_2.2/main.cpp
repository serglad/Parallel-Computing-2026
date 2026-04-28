#include <iostream>
#include <omp.h>
#include "math.h"
#include <vector>
#include <chrono>
#include <fstream>

double integrate(double lb, double ub, double step, double (*func)(double)){
    double sum = 0;
    for(double x = lb; x <= ub; x += step){
        sum += func(x) * step;
    }
    return sum;
}

double integrateOMP(double lb, double ub, double segmentCount, double (*func)(double), int nJobs){
    double step = (ub-lb)/segmentCount;
    double sum=0;
    #pragma omp parallel num_threads(nJobs)
    {
        int totalThreads = omp_get_num_threads();
        int threadId = omp_get_thread_num();
        int segmentsPerThread = segmentCount/totalThreads;
        double threadLb = threadId * segmentsPerThread * step;
        double threadUb = (threadId == totalThreads-1) ? ub : threadLb + segmentsPerThread * step;
        #pragma omp atomic
            sum += integrate(threadLb, threadUb, step, func);
    }
    return sum;
}

int main(void){
    double lb = 0;
    double ub = 2 * M_PI;
    int threadCounts[] = {1,2, 4, 7, 8, 16, 20, 40};
    int segmentCount = 40'000'000;
    std::ofstream output_stream("ver1_output.txt");
    for(int threadCount : threadCounts){
        int n=100;
        double sum = 0;
        for(int i=0;i<n;++i){
            auto start = std::chrono::high_resolution_clock::now();
            double result = integrateOMP(lb, ub, segmentCount, &sin, threadCount);
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
            sum += duration.count();
        }
        output_stream << "Threads: " << threadCount
                  << ", Time: " << sum/n << " ms" << std::endl;
    }
    output_stream.close();
    return 0;
}