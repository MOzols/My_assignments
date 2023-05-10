#include "barrier.hpp"

/*
The function has got a master process which will collect messages from all the others
once they have reached the barrier. After sending the message of "finished" to the master,
general processes will lock themselfs in a "receive" waiting for the master to unlock them.
When the master has confirmed that all processes have finished, it sends them a message
so they can continue.

Assuming that a communication completes in one step and a node can participate in at most
to one pairwise (1:1) communication in each step, this function will have a depth of 2*(n-1)
steps, being n the number of threads. The master process consists of two loops of n-1 steps
(one for receiving and the other for sending).
*/

void barrier(MPI_Comm COMM) {

	int world_rank, world_size;
	MPI_Comm_rank(COMM, &world_rank);
	MPI_Comm_size(COMM, &world_size);

	// We don't care about the content of the message.
	// Messages are only used as a way of synchronization
	
	// Root
	if (world_rank == 0) {
		
		int message;
		int counter = world_size-1;
		MPI_Status s;
		
		// All massages are collected in a loop. The order is not important
		// since we need to know that all of them are received to continue
		while (counter > 0) {
			MPI_Recv(&message, 1, MPI_INT, counter, 0, COMM, &s);
			counter--;
		}
		
		// Once we know every process has reaches this point and it is 
		// waiting, we unlock them
		for (int i = 1; i < world_size; i++) {
			MPI_Send(&message, 1, MPI_INT, i, 1, COMM);
		}
		
	}
	
	if (world_rank > 0) {
		
		int complete = 1;
		int message;
		MPI_Status s;
		
		// When the barrier is reached, a message is send to the root
		MPI_Send(&complete, 1, MPI_INT, 0, 0, COMM);
		
		// The Receive forces the program to wait
		MPI_Recv(&message, 1, MPI_INT, 0, 1, COMM, &s);
	}
}
