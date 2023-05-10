#include "common.hpp"
#define WORKERS 5
#define PARTITION_SIZE STREAM_ELEMENT_SIZE/WORKERS

using worker_element = streamelement_t<PARTITION_SIZE>;

void emmiter(const int src, const int dst[WORKERS]){
    streamelement x;
    worker_element y;
    int idst=0;
    while(true) {
        x.recv(src);
        if(x.is_terminated()){
            for(int i = 0; i<WORKERS;++i){
                y.set_terminated();
                y.send(dst[i]);
            }
            return;
        }
        for(int i = 0; i<STREAM_ELEMENT_SIZE; i+=PARTITION_SIZE){
            for(int j = 0; j < PARTITION_SIZE; j++)
                y[j] = x[i+j];
            y.send(dst[idst]);
            idst = (idst+1)%WORKERS;
        }
    }
}

void W(const int src, const int dst) {
	worker_element y;
	while(true) {
		y.recv(src);
		if(y.is_terminated()) {
			y.send(dst);
			return;
		}
		for(int j=0; j<PARTITION_SIZE; ++j){
		    	y[j] = h1(g1(f1(y[j])));
		}
		y.send(dst);
	}
}
void collector( const int dst){
    streamelement x;
    worker_element y;
    int w_terminated=0, w_done=0, rank, start, end, idx=0;
    while(true){
        rank = y.recv_any();
        start = (rank-2)*PARTITION_SIZE;
        end = ((rank-2)*PARTITION_SIZE)+PARTITION_SIZE;
        if(y.is_terminated())
            w_terminated++;
        if(w_terminated == WORKERS){
            x.set_terminated();
            x.send(dst);
            return;
        }
        if(!y.is_terminated()){
            for(int i = start; i < end; i++){
                x[i] = y[idx];
                idx++;
            }
            idx=0;
            w_done++;
        }
        if(w_done == WORKERS){
            x.send(dst);
            w_done = 0;
        }
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
