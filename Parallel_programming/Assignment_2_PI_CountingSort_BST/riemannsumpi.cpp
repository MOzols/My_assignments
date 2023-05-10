#include <iostream>
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/sysinfo.h>

constexpr double correctpi = 3.14159265358979323846264338327950288419716939937510;

inline double f(const double x)
{
	return sqrt(1-x*x);
}

double riemann_seq(const int n, double(*f)(double))
{
	const double Dx = 1.0/n;
	double sum = 0.0;
	for(int i=0; i<n; ++i)
		sum += Dx*f(i*Dx);
	return sum;
}

double riemann_parallel(const int n, double(*f)(double)){

    const double Dx = 1.0/n;
    double sum = 0.0;
    #pragma omp parallel
    {
        double partialsum = 0.0;
        #pragma omp for nowait
        for(int i=0; i<n;++i)
            partialsum += Dx*f(i*Dx);
        #pragma omp atomic
        sum += partialsum;
    }
    return sum;
}

int main(int argc, char* argv[]) {

	const int n = argc>1 && atoi(argv[1])>0 ? atoi(argv[1]) : 0;

	printf("n = %d\n", n);
	printf("nproc = %d\n", nproc);

	double pi;

	double ts = omp_get_wtime();
	pi = 4.0*riemann_seq(n, f);
	ts = omp_get_wtime()-ts;
	printf("seq, elapsed time = %.3f seconds, err = %.4e\n", ts, abs(correctpi-pi));

	double tp = omp_get_wtime();
	pi = 4.0*riemann_parallel(n, f);
	tp = omp_get_wtime()-tp;
	printf("par, elapsed time = %.3f seconds, err = %.4e\n", tp, abs(correctpi-pi));

	return EXIT_SUCCESS;
}
