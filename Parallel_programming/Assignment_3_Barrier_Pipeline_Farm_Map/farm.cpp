#include "common.hpp"
#define WORKERS 5

void emmiter(const int src, const int dst[WORKERS]){
    streamelement x;
    int idst=0;
    while(true) {
        x.recv(src);
        if(x.is_terminated()){
            for(int i = 0; i<WORKERS;++i)
                x.send(dst[i]);
            return;
        }
        x.send(dst[idst]);
        idst = (idst+1)%5;
    }
}

void W(const int src, const int dst) {
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
void collector( const int dst){
    streamelement x;
    int w_count=0;
    while(true){
        x.recv_any();
        if(x.is_terminated())
            w_count++;
        if(w_count == WORKERS){
            x.send(dst);
            return;
        }
        if(!x.is_terminated())
            x.send(dst);
    }
}
// Set this value according to the number of processes required to run your program
const int required_comm_size = 9;

// This function maps each module to a distinct process represented by rank (one-process runs one-module)
void func2rank(const int rank) {
    int dst[]={2,3,4,5,6};
	if(rank==0) in(1);
	if(rank==1) emmiter(0,dst);
	if(rank==2) W(1,7);
	if(rank==3) W(1,7);
	if(rank==4) W(1,7);
	if(rank==5) W(1,7);
	if(rank==6) W(1,7);
	if(rank==7) collector(8);
	if(rank==8) out(7);
}
