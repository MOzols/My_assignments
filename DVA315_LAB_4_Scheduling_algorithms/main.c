#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<time.h>
#include<windows.h>

#define sched_RR 1
#define sched_SJF 2
#define sched_MQ 3
#define QUEUE_SIZE 10

//int sched_type = sched_RR;
//int sched_type = sched_SJF;
int sched_type = sched_MQ;

int finished = 0;
int context_switch_program_exit = 0;
int context_switch = 0;
int OS_cycles = 0;
int context_switches = 0;
int exec_task = 0;

typedef struct taskprop
{
	int deadline;
	int period;
	int release_time;
	int priority;
	int ID;
	int quantum;
	int queue_size;
	struct taskprop* next;
}task;

task* ready_queue = NULL;
task* high_queue = NULL;
task* medium_queue = NULL;
task* low_queue = NULL;
task* waiting_queue = NULL;
task* shortest_job;
task* idle_task;
task tasks[QUEUE_SIZE];
int activeTasks = 0;
int idleTasks = 0;

//---------Linked list functions----------
void printList(task* head)
{
	task* temp = head;
	if (head != NULL)
		while (temp != NULL)
		{
			printf("\nTask in queue: %d quanta left: %d", temp->ID, temp->quantum);
			temp = temp->next;
		}
	printf("\n\n");
}
void copy_task(task** dest, task* src)
{
	task* temp = *dest;
	temp->ID = src->ID;
	temp->deadline = src->deadline;
	temp->release_time = src->release_time;
	temp->period = src->period;
	temp->priority = src->priority;
	temp->quantum = src->quantum;
	temp->queue_size = 0;
	*dest = temp;
}
void SelectionSort(task** head)
{
	task* temp;
	task* temp2;
	task* temp3 = (task*)malloc(sizeof(task));
	for (temp = *head; temp != NULL; temp = temp->next)
		for (temp2 = temp->next; temp2 != NULL; temp2 = temp2->next)
			if (temp->quantum > temp2->quantum)
			{
				copy_task(&(temp3), temp);
				copy_task(&temp, temp2);
				copy_task(&temp2, temp3);
			}
	free(temp3);
}
task* create(int deadline, int period, int release_time, int priority, int ID, int quantum, task* next)
{
	task* new_node = (task*)malloc(sizeof(task));
	if (new_node == NULL)
	{
		printf("Memory Error creating a new ndoe.\n");
		exit(0);
	}
	new_node->deadline = deadline;
	new_node->period = period;
	new_node->release_time = release_time;
	new_node->priority = priority;
	new_node->ID = ID;
	new_node->quantum = quantum;
	new_node->next = next;
	return new_node;
}
task* push(task* head, task data)
{
	task* cursor = head;
	if (cursor == NULL)
		head = create(data.deadline, data.period, data.release_time, data.priority, data.ID, data.quantum, NULL);
	else
	{
		if (cursor->next != NULL)
			while (cursor->next != NULL)
				cursor = cursor->next;
		task* new_node = create(data.deadline, data.period, data.release_time, data.priority, data.ID, data.quantum, NULL);
		cursor->next = new_node;
	}
	return head;
}
task* pop(task* head)
{
	if (head == NULL)
		return NULL;
	task* front = head;
	head = head->next;
	front->next = NULL;
	free(front);
	return head;
}
task* putFirst(task* head, task data)
{
	if(head == NULL)
		head = create(data.deadline, data.period, data.release_time, data.priority, data.ID, data.quantum, NULL);
	else
	{
		task* front = create(data.deadline, data.period, data.release_time, data.priority, data.ID, data.quantum, NULL);
		front->next = head;
		head = front;
	}
	return head;
}
task* remove_back(task* head)
{
	if (head == NULL)
		return NULL;
	task* cursor = head;
	if (head->next == NULL)
	{
		head = NULL;
		free(cursor);
		return head;
	}
	else
	{
		while (cursor->next->next != NULL)
			cursor = cursor->next;
		free(cursor->next);
		cursor->next = NULL;
	}
	return head;

}
task* remove_front(task* head)
{
	if (head == NULL)
		return NULL;
	task* front = head;
	head = head->next;
	front->next = NULL;
	free(front);
	return head;
}
task* remove_node(task* head, task* nd)
{
	if (nd == head)
	{
		head = pop(head);
		return head;
	}
	if (nd->next == NULL)
	{
		head = remove_back(head);
		return head;
	}
	task* cursor = head;
	while (cursor != NULL && cursor->next != nd)
		cursor = cursor->next;
	if (cursor != NULL)
	{
		task* tmp = cursor->next;
		cursor->next = tmp->next;
		tmp->next = NULL;
		free(tmp);
	}
	return head;
}
task* first_to_last(task* head)
{
	if (head == NULL)
		return NULL;
	if (head->next == NULL)
		return head;
	task* new_front = head->next;
	task* cursor = new_front;
	while (cursor->next != NULL)
		cursor = cursor->next;
	cursor->next = head;
	head->next = NULL;
	return new_front;
}
void readTaskset_n(char* filepath)
{
	FILE* reads;
	errno_t error;
	char* fileName = "taskset.txt";
	error = fopen_s(&reads, fileName, "rb");
	if (error == 0 && reads != NULL)
	{
		task* data_struct = (task*)malloc(sizeof(task));
		if (data_struct != NULL)
			while (!feof(reads))
			{
				fscanf_s(reads, "%d %d %d %d %d %d\n", &data_struct->deadline, &data_struct->period, &data_struct->release_time, &data_struct->priority, &data_struct->ID, &data_struct->quantum);
				waiting_queue = push(waiting_queue, *data_struct);
			}
		else
			printf("Memeory Error to create data struct in readTaskset_n");
		free(data_struct);
	}
	else
	{
		perror("Error");
		return;
	}
}
void OS_wakeup_n()
{
	if (waiting_queue == NULL)
		return;
	task* temp = waiting_queue;
	while (temp != NULL)
	{
		if (OS_cycles >= temp->release_time)
		{
			ready_queue = push(ready_queue, *temp);
			waiting_queue = remove_node(waiting_queue, temp);
			temp = waiting_queue;
		}
		else
			temp = temp->next;
	}
}
task* scheduler_n()
{
	if (ready_queue != NULL||high_queue != NULL || medium_queue != NULL || low_queue != NULL)
	{
		task* temporary = ready_queue;
		if (sched_type == sched_RR)
		{
			printf("\nReady queue:");
			printList(ready_queue);
			return ready_queue;
		}
		if (sched_type == sched_SJF)
		{
			//SelectionSort(&ready_queue);
			///*
			if (ready_queue != shortest_job)
				shortest_job = ready_queue;
			while (temporary != NULL)
			{
				if (temporary->quantum < shortest_job->quantum)
					shortest_job = temporary;
				temporary = temporary->next;
			}
			if (ready_queue != shortest_job)
			{
				temporary = ready_queue;
				while (temporary->next != shortest_job && temporary->next != NULL)
					temporary = temporary->next;
				temporary->next = shortest_job->next;
				shortest_job->next = ready_queue;
				ready_queue = shortest_job;
			}
			//*/
			printf("\nReady queue after sorting:");
			printList(ready_queue);
			return ready_queue;
		}
		if (sched_type == sched_MQ)
		{
			while (temporary != NULL)
			{
				if (temporary->priority == 1)
					high_queue = push(high_queue, *temporary);
				else if (temporary->priority == 2)
					medium_queue = push(medium_queue, *temporary);
				else if (temporary->priority == 3)
					medium_queue = putFirst(medium_queue, *temporary);
				else if (temporary->priority % 4 == 0)
					low_queue = push(low_queue, *temporary);
				else
					low_queue = putFirst(low_queue, *temporary);
				ready_queue = remove_node(ready_queue, temporary);
				temporary = ready_queue;
			}
			if (high_queue != NULL)
			{
				printf("\nHigh priority queue:");
				printList(high_queue);
				ready_queue = push(ready_queue, *high_queue);
				high_queue = pop(high_queue);
			}
			else if (medium_queue != NULL)
			{
				printf("\nMedium priority queue:");
				printList(medium_queue);
				ready_queue = push(ready_queue, *medium_queue);
				medium_queue = pop(medium_queue);
			}
			else if (low_queue != NULL)
			{
				printf("\nLow priority queue:");
				printList(low_queue);
				ready_queue = push(ready_queue, *low_queue);
				low_queue = pop(low_queue);
			}
			return ready_queue;
		}
	}
	else
	{
		idle_task->quantum++;
		printf("RETURNED IDLE TASK");
		return idle_task;
	}
	return NULL;
}
void dispatch_n(task* exec)
{
	if (exec->ID != exec_task)
	{
		context_switches++;
		exec_task = exec->ID;
	}
	exec->quantum--;
	if (exec->quantum > 0)
	{
		if (exec == idle_task)
			printf("OS_cycle: %d; Idle task is executing - Total context switches: %d\n", OS_cycles, context_switches);
		else
			printf("OS_cycle: %d; Task %d is executing with %d quanta left - Total context switches: %d\n", OS_cycles, exec->ID, exec->quantum, context_switches);
		if (sched_type == sched_RR)
			ready_queue = first_to_last(ready_queue);
		else if (sched_type == sched_MQ)
			exec->priority++;
	}
	else
	{
		printf("OS_cycle: %d; Task %d has executed and finished its quanta - Total context swithces: %d\n", OS_cycles, exec->ID, context_switches);
		ready_queue->release_time = ready_queue->release_time + ready_queue->period;
		waiting_queue = push(waiting_queue, *ready_queue);
		ready_queue = pop(ready_queue);
	}
}

int main(int argc, char** argv)
{
	char* fp = "hej";
	readTaskset_n(fp);
	task* task_to_be_run;
	idle_task = create(1337, 0, 0, 0, 0, 200000000, NULL);
	while (1)
	{
		OS_wakeup_n();
		task_to_be_run = scheduler_n();
		dispatch_n(task_to_be_run);
		OS_cycles++;
		Sleep(250);
	}
}