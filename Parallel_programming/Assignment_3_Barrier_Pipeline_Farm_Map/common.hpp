/* DO NOT MODIFY THIS FILE */

#pragma once

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <chrono>
#include <thread>
#include <iostream>

// uncomment to disable assert()
// #define NDEBUG
#include <cassert>

// needed to avoid header issues with C++-supporting MPI implementations
#define OMPI_SKIP_MPICXX 1
#include <mpi.h>

#define DIV(n,m) ((n)/(m)) //truncate the result of n/m
#define CEILDIV(n,m) DIV((n)+(m)-1,m) //roundup the result of n/m

#define TAG 1234
#define STREAM_ELEMENT_SIZE 10
#define STREAM_LENGTH 100

template <int N> struct streamelement_t {
	/*
	Implementation of a stream element of generic size N.
	The entry A[N] is used to test and set the termination guard that is propagated through the modules of the graph when the stream is ended (the initial value is zero which stands for 'not terminated').
	The functions send() and recv() are used to communicate with the process passed as parameter.
	The collecting module of farm and map should be able to receive from any worker in a nondeterministic way. To this end, you can use recv_any() that receives from any process and returns the rank of the sender.
	*/
	streamelement_t() { A[N] = 0; }
	bool is_terminated() { return A[N]==1; };
	void set_terminated() { A[N] = 1; }
	void print(int idx) { printf("[%03d] {", idx); for(int i=0; i<N-1; ++i) printf("%d,", A[i]); printf("%d}\n", A[N-1]); }
	int& operator [] (int i) { assert(i>=0 && i<N); return A[i]; }
	void send(int dst) { MPI_Ssend(A, N+1, MPI_INT, dst, TAG, MPI_COMM_WORLD); }
	void recv(int src) { MPI_Recv(A, N+1, MPI_INT, src, TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE); }
	int recv_any() { MPI_Status status; MPI_Recv(A, N+1, MPI_INT, MPI_ANY_SOURCE, TAG, MPI_COMM_WORLD, &status); return status.MPI_SOURCE; }
private:
	int A[N+1];
};

using streamelement = streamelement_t<STREAM_ELEMENT_SIZE>;

inline void sleep_milliseconds(const int ms) { std::this_thread::sleep_for(std::chrono::milliseconds(ms)); }

// functions' service time
#define T_f0 4
#define T_f1 9
#define T_g1 5
#define T_h1 3

// functions
int f0(int n) { sleep_milliseconds(T_f0); return n*n; }
int f1(int n) { sleep_milliseconds(T_f1); return n*2; }
int g1(int n) { sleep_milliseconds(T_g1); return n/2; }
int h1(int n) { sleep_milliseconds(T_h1); return n+1; }

static double *wtimes = nullptr;

void in(const int dst) {
	wtimes = new double[STREAM_LENGTH];
	streamelement x;
	for(int i=0; i<STREAM_LENGTH; ++i) {
		for(int j=0; j<STREAM_ELEMENT_SIZE; ++j)
			x[j] = f0(j);
		x.send(dst);
		wtimes[i] = -MPI_Wtime();
	}
	x.set_terminated();
	x.send(dst);
}

void out(const int src) {
	wtimes = new double[STREAM_LENGTH];
	streamelement x;
	int i = 0;
	while(true) {
		x.recv(src);
		if(x.is_terminated())
			break;
		if(i<STREAM_LENGTH)
			wtimes[i] = MPI_Wtime();
		for(int j=0; j<STREAM_ELEMENT_SIZE; ++j)
			assert(x[j]==j*j+1);
		x.print(++i);
	}
	printf("\nStream completed: %d elements processed!\n\n", i);
}

static double calc_latency() {
	if(wtimes==nullptr) {
		wtimes = new double[STREAM_LENGTH];
		for(int i=0; i<STREAM_LENGTH; ++i) wtimes[i] = 0.0;
	}
	double sums[STREAM_LENGTH], sum = 0.0;
	MPI_Allreduce(wtimes, sums, STREAM_LENGTH, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
	delete [] wtimes;
	for(int i=0; i<STREAM_LENGTH; ++i) sum += sums[i];
	return sum/STREAM_LENGTH;
}

//to be set in the .cpp file
extern const int required_comm_size;

//to be implemented in the .cpp file
void func2rank(const int);

inline double get_wtime(MPI_Comm comm) {
	MPI_Barrier(comm);
	return MPI_Wtime();
	MPI_Barrier(comm);
}

int main(int argc, char** argv) {
	//return the number of processes required if the command-line option is 'np'
	if(argc==2 && strcmp(argv[1],"np")==0) {
		printf("%d\n", required_comm_size);
		return EXIT_SUCCESS;
	}
	//init MPI env
	MPI_Init(&argc, &argv);
	//get process rank and communicator size
	int rank, size;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	//check that current communicator size equals the required one
	if(size!=required_comm_size)
		return EXIT_FAILURE;
	//start stream computation
	double start = get_wtime(MPI_COMM_WORLD);
	func2rank(rank);
	double end = get_wtime(MPI_COMM_WORLD);
	double lat = calc_latency();
	if(rank==0) {
		sleep_milliseconds(250);
		printf("parallelism degree = %d\ncompletion time = %.2f s\navg service time = %.0f ms\navg latency = %.0f ms\n", required_comm_size-2, end-start, 1000*(end-start)/STREAM_LENGTH, 1000*lat);
	}
	//quit MPI env
	MPI_Finalize();
	//exit
	return EXIT_SUCCESS;
}

#undef STREAM_LENGTH
