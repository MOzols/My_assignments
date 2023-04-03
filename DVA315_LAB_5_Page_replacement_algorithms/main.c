#include<stdio.h>
#include<stdlib.h>
#define LRU 1
#define FIFO 2
#define LFU 3
#define OPT 4

int algorithm_type = LRU;
//int algorithm_type = FIFO;
//int algorithm_type = LFU;
//int algorithm_type = OPT;

typedef struct 
{
	int page;	//page stored in this memory frame
	int time;	//Time stamp of page stored in this mempory frame
	int free;	//Indicates if frame is free or not
	//Add own data if needed for FIFO, OPT, LFU
	int FIFO_counter;
	int LFU_counter;
	int OPT_flag;
	int OPT_current;
	int OPT_refSize;
	int* refs;
}frameType;

void initilize(int* no_of_frames, int* no_of_references, int refs[], frameType frames[])
{// Initializes by reading stuff from file and inits all frames as free
	int i;
	FILE* fp;
	char* fileName = "ref.txt";
	errno_t error;

	error = fopen_s(&fp, fileName, "rb");
	if (error == 0 && fp != NULL)
	{
		fscanf_s(fp, "%d", no_of_frames);			//Get the number of frames
		fscanf_s(fp, "%d", no_of_references);		//Get the number of references in the reference string
		for (i = 0; i < *no_of_references; ++i)		//Get the reference string
			fscanf_s(fp, "%d", &refs[i]);
		fclose(fp);
		for (i = 0; i < *no_of_frames; ++i)
		{
			frames[i].free = 1;						//Indicates a free frame in memory
			frames[i].refs = refs;
			frames[i].OPT_flag = -1;
		}
		printf("\nPages in memory:");
		for (i = 0; i < *no_of_frames; ++i)			//Print header with frame numbers
			printf("\t%d", i);
		printf("\n");
	}

}
void printResultOfReference(int no_of_frames, frameType frames[], int pf_flag, int mem_flag, int pos, int mem_frame, int ref)
{//Prints the results of a reference, all frames and their content and some info if page fault
	int j;
	printf("Accessing page %d:", ref);

	for (j = 0; j < no_of_frames; ++j)			//Print out which pages are in memory, i.e. memory frames
	{
		if (frames[j].free == 0)				//Page is in memory
			printf("\t%d", frames[j].page);
		else
			printf("\t");
	}
	if (pf_flag == 0)							//Page fualt
		printf("  Page fault");
	if (mem_flag == 0)							//Did not find a free frame
		printf(", replaced frame: %d", pos);
	else if (mem_frame != -1)					//A free frame was found
		printf(", used free frame %d", mem_frame);
	printf("\n");
}
int findPageToEvict(frameType frames[], int nof)
{//Finds the position in memory to evict in case of page fault and no free memory location
	int pos = 0, i;
	if (algorithm_type == LRU)
	{
		int minimum = frames[0].time;	//LRU eviction strategy -- This is what you are supposted to change in the lab for LFU and OPT

		for (i = 1; i < nof; ++i)
			if (frames[i].time < minimum)			//Find the page position with minimum time stamp among all frames
			{
				minimum = frames[i].time;
				pos = i;
			}
		return pos;
	}
	else if (algorithm_type == FIFO)
	{
		int oldest = frames[0].FIFO_counter;
		for(i = 1; i < nof; ++i)
			if (frames[i].FIFO_counter < oldest)
			{
				oldest = frames[i].FIFO_counter;
				pos = i;
			}
		return pos;
	}
	else if (algorithm_type == LFU)
	{
		int leastUsed = frames[0].LFU_counter;
		for (i = 1; i < nof; ++i)
			if (frames[i].LFU_counter < leastUsed)
			{
				leastUsed = frames[i].LFU_counter;
				pos = i;
			}

		return pos;
	}
	else if (algorithm_type == OPT)
	{
		for (i = 0; i < nof; i++)
		{
			for (int j = frames[0].OPT_current; j < frames[0].OPT_refSize; j++)
			{
				if (frames[i].OPT_flag == -1)
					if (frames[i].page == frames[0].refs[j])
					{
						frames[i].OPT_flag = j;
						break;
					}
			}
			if (frames[i].OPT_flag == -1)
				frames[i].OPT_flag = frames[0].OPT_refSize;
		}
		int farthestNextUsed = frames[0].OPT_flag;
		for (i = 1; i < nof; i++)
		{
			if (frames[i].OPT_flag > farthestNextUsed)
			{
				farthestNextUsed = frames[i].OPT_flag;
				pos = i;
			}
		}
		for (i = 0; i < nof; i++)
			frames[i].OPT_flag = -1;
		return pos;
	}
}
/*Main loops ref string, for each ref 1)check if ref is in memory, 
2)if not, check if there is free frame, 3)if not, find a page to evict.*/ 
int main(void)
{
	int no_of_frames, no_of_references, refs[100], counter = 0, page_fault_flag, no_free_mem_flag, i, j, pos = 0, faults = 0, free = 0;
	frameType frames[20];
	initilize(&no_of_frames, &no_of_references, refs, frames); //Read number of frames, number of refs and ref string from file
	
	for (i = 0; i < no_of_references; i++)
	{
		page_fault_flag = no_free_mem_flag = 0;
		for (j = 0; j < no_of_frames; ++j)
		{
			if (frames[j].page == refs[i])
			{
				counter++;
				frames[j].LFU_counter++;
				frames[j].time = counter;
				page_fault_flag = no_free_mem_flag = 1;
				free = -1;
				break;
			}
		}
		if (page_fault_flag == 0)
		{
			for (j = 0; j < no_of_frames; ++j)
			{
				if (frames[j].free == 1)
				{
					counter++;
					faults++;
					frames[j].FIFO_counter = counter;
					frames[j].LFU_counter = 1;
					frames[j].page = refs[i];
					frames[j].time = counter;
					frames[j].free = 0;
					no_free_mem_flag = 1;
					free = j;
					break;
				}
			}
		}
		if (no_free_mem_flag == 0)
		{
			frames[0].OPT_current = i;
			frames[0].OPT_refSize = no_of_references;
			pos = findPageToEvict(frames, no_of_frames);
			counter++;
			faults++;
			frames[pos].FIFO_counter = counter;
			frames[pos].LFU_counter = 1;
			frames[pos].page = refs[i];
			frames[pos].time = counter;
		}
		printResultOfReference(no_of_frames,frames, page_fault_flag, no_free_mem_flag, pos, free, refs[i]);
	}
	printf("\n\nTotal Page Faults = %d\n\n", faults);
	return 0;
}




