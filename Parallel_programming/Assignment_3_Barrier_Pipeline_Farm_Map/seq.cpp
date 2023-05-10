#include "common.hpp"

void M(const int src, const int dst) {
	streamelement x;
	while(true) {
		x.recv(src);
		if(x.is_terminated()) {
			x.send(dst);
			return;
		}
		for(int j=0; j<STREAM_ELEMENT_SIZE; ++j)
			x[j] = h1(g1(f1(x[j])));
		x.send(dst);
	}
}

// Set this value according to the number of processes required to run your program
const int required_comm_size = 3; 

// This function maps each module to a distinct process represented by rank (one-process runs one-module)
void func2rank(const int rank) {
	if(rank==0) in(1);
	if(rank==1) M(0,2);
	if(rank==2) out(1);
}
