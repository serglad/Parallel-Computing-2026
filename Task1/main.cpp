#include <math.h>
#include <iostream>
#include <memory>
#ifdef VAL_DOUBLE
#define SIN_TYPE double
#else
#define SIN_TYPE float
#endif

#define OUTPUT_LENGTH 10000000

int main(void){
    std::unique_ptr<SIN_TYPE[]> sins=std::make_unique<SIN_TYPE[]>(OUTPUT_LENGTH);
    const double period = 2*M_PI/OUTPUT_LENGTH;
    SIN_TYPE sum = 0;
    for(int i=0;i<OUTPUT_LENGTH;++i){
        sins[i]=sin(period*i);
        sum += sins[i];
    }
    std::cout << sum << std::endl;
}