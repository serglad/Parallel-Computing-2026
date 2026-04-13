#include <iostream>
#include <omp.h>
#include "math.h"
#include <vector>
#include <chrono>

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
    
    for(int threadCount : threadCounts){
        auto start = std::chrono::high_resolution_clock::now();
        double result = integrateOMP(lb, ub, segmentCount, &sin, threadCount);
        auto end = std::chrono::high_resolution_clock::now();
        
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        std::cout << "Threads: " << threadCount << ", Result: " << result 
                  << ", Time: " << duration.count() << " ms" << std::endl;
    }
    return 0;
}