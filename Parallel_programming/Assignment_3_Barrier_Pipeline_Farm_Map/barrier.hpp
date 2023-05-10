#pragma once

#include <mpi.h>
#include <cstdio>
#include <cstdlib>

void barrier(MPI_Comm COMM);

#define BARRIER_TAG 1234

int main(int argc, char **argv) {
	/* DON'T CHANGE THIS FUNCTION */
	MPI_Init(&argc, &argv);

	int rank, size;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	const int barrier_size = size-1;
	MPI_Comm barrier_comm;

	if(rank==0) {
		//master control thread
		MPI_Comm_split(MPI_COMM_WORLD, MPI_UNDEFINED, 0, &barrier_comm);
		int n=2*barrier_size, acc=0, rdy=0, msg;
		while(n--) {
			MPI_Recv(&msg, 1, MPI_INT, MPI_ANY_SOURCE, BARRIER_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			if(rdy==0 && msg<0) continue; //negative values received when rdy is still zero are discarded
			acc += msg; //accumulate message values
			rdy += acc==barrier_size; //becomes 1 as soon as all positive values have been received
		}
		printf("acc=%d\n",acc);
		//if no message has been discarded (i.e., the barrier works) acc is zero
		printf("%d-process barrier test %sed.\n", barrier_size, acc==0?"pass":"fail");
	} else {
		//synchronized threads
		//printf("Enter else in .hpp\n");
		MPI_Comm_split(MPI_COMM_WORLD, 0, rank, &barrier_comm);
		const int plus_one = +1, minus_one = -1;
		MPI_Send(&plus_one, 1, MPI_INT, 0, BARRIER_TAG, MPI_COMM_WORLD);
		//printf("Process with num_%d in .hpp else 1\n", rank);
		barrier(barrier_comm);
		//printf("Process with num_%d in .hpp else 2\n", rank);
		MPI_Send(&minus_one, 1, MPI_INT, 0, BARRIER_TAG, MPI_COMM_WORLD);
		//printf("Exit else in .hpp\n");
	}
	MPI_Finalize();
	return EXIT_SUCCESS;
}
