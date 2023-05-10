#include "common.hpp"

void f_1(const int src, const int dst){
    streamelement x;
	while(true) {
		x.recv(src);
		if(x.is_terminated()) {
			x.send(dst);
			return;
		}
		for(int j=0; j<STREAM_ELEMENT_SIZE; ++j)
		    x[j] = f1(x[j]);
		x.send(dst);
	}
}
void g1_h1(const int src, const int dst){
    streamelement x;
    while(true){
        x.recv(src);
        if(x.is_terminated()){
            x.send(dst);
            return;
        }
        for(int j=0; j<STREAM_ELEMENT_SIZE; ++j)
            x[j] = h1(g1(x[j]));
        x.send(dst);
    }
}
// Set this value according to the number of processes required to run your program
const int required_comm_size = 4;

// This function maps each module to a distinct process represented by rank (one-process runs one-module)
void func2rank(const int rank) {
	if(rank==0) in(1);
	if(rank==1) f_1(0,2);
	if(rank==2) g1_h1(1,3);
	if(rank==3) out(2);
}
