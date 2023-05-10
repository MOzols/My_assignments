#include <iostream>
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>

using namespace std;

//A cache line is the unit of data transfer between the cache and main memory (needed for solving the task). Typically the cache line is 64 bytes.
#ifndef cachelinesize
#define cachelinesize 64 //bytes
#endif

#define p cachelinesize/sizeof(int) //To calculate padding
#define s k*p //To calculate size for array with padding

template <const int k> void seq_countingsort(int *out, int const *in, const int n) {
	int counters[k] = {}; // all zeros
	for (int i = 0; i < n; ++i)
		++counters[in[i]];
	int tmp, sum = 0;
	for (int i = 0; i < k; ++i) {
		tmp = counters[i];
		counters[i] = sum;
		sum += tmp;
	}
	for (int i = 0; i < n; ++i)
		out[counters[in[i]]++] = in[i];
}

template <const int k> void par_countingsort(int *out, int const *in, const int n) {
	int counters[nproc][k] = {}; // all zeros
	#pragma omp parallel
	{
		int *thcounters = counters[omp_get_thread_num()];
		#pragma omp for
		for (int i = 0; i < n; ++i)
			++thcounters[in[i]];
		#pragma omp single
		{
			int tmp, sum = 0;
			for (int j = 0; j < k; ++j){
			    for (int i = 0; i < nproc; ++i) {
					tmp = counters[i][j];
					counters[i][j] = sum;
					sum += tmp;
				}
			}
		}
		#pragma omp for
		for (int i = 0; i < n; ++i)
			out[thcounters[in[i]]++] = in[i];
	}
}
template <const int k> void par2_countingsort(int *out, int const *in, const int n){

    alignas(cachelinesize) int counters[nproc][s] = {};
	#pragma omp parallel
	{
		int *thcounters =  counters[omp_get_thread_num()];
		#pragma omp for
		//Using for loop to save values from input array in appropriate index in counters
		//to put in appropriate index is used padding to guarantee 64 byte gap between variables
		for (int i = 0; i < n; ++i)
			++thcounters[(in[i]*p)];
		#pragma omp single
		{
			int tmp, sum = 0;
			for (unsigned int j = 0; j < s; j+=p){
			    for (int i = 0; i < nproc; ++i) {
					tmp = counters[i][j];
					counters[i][j] = sum;
					sum += tmp;
				}
			}
		}
		#pragma omp for
		for (int i = 0; i < n; ++i)
		    //using padding to access appropriate values in counters
			out[thcounters[(in[i]*p)]++] = in[i];
	}
}

bool checkreset(int *out, int const *in, const int n) {
	int insum = 0, outsum = 0, notsorted = 0;
	#pragma omp parallel for reduction(+:insum)
	for (int i = 0; i < n; ++i) insum += in[i];
	#pragma omp parallel for reduction(+:outsum)
	for (int i = 0; i < n; ++i) outsum += out[i];
	#pragma omp parallel for reduction(+:notsorted)
	for (int i = 1; i < n; ++i) notsorted += out[i-1]>out[i];
	if(insum!=outsum || notsorted) return false;
	#pragma omp parallel for
	for (int i = 0; i < n; ++i) out[i] = 0;
	return true;
}

#define k 90

int main(int argc, char *argv[]) {

	//init input
	int n = argc>1 && atoi(argv[1])>0 ? atoi(argv[1]) : 0;
	int* in = (int*)malloc(sizeof(int)*n);
	int* out = (int*)malloc(sizeof(int)*n);;
	for (int i = 0; i < n; ++i)
		in[i] = rand()%k;
	printf("n = %d\n", n);

	//print some parameters
	printf("nproc = %d\n", nproc);
	printf("cachelinesize = %d byte\n", cachelinesize);
	printf("k = %d\n", k);

	//tests
	double ts = omp_get_wtime();
	seq_countingsort<k>(out, in, n);
	ts = omp_get_wtime() - ts;
	printf("seq, elapsed time = %.3f seconds, check passed = %c\n", ts, checkreset(out,in,n)?'y':'n');

	double tp = omp_get_wtime();
	par_countingsort<k>(out, in, n);
	tp = omp_get_wtime() - tp;
	printf("par, elapsed time = %.3f seconds (%.1fx speedup), check passed = %c\n", tp, ts/tp, checkreset(out,in,n)?'y':'n');

    double tp2 = omp_get_wtime();
	par2_countingsort<k>(out, in, n);
	tp2 = omp_get_wtime() - tp2;
	printf("par, elapsed time = %.3f seconds (%.1fx speedup), check passed = %c\n", tp2, ts/tp2, checkreset(out,in,n)?'y':'n');

	//free mem
	free(in);
	free(out);

	return EXIT_SUCCESS;
}
