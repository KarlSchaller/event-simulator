/*
============================================================================
Name        : Karl Schaller
Date        : 02/12/2020
Course      : CIS3207
Homework    : Assignment 1 Event Simulator
 ============================================================================
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum event_type {JOB_ARRIVED, CPU_FINISHED, DISK1_FINISHED, DISK2_FINISHED, NETWORK_FINISHED, SIMULATION_FINISHED};

struct linked_list {
	struct node {
		union node_data {
			int job_num; // for cpu, etc. queue
			struct event { // for priority queue
				int time;
				enum event_type type;
			} event_info;
		} data;
		struct node *next;
	} *head, *tail;
	int length;
} event_pq, cpu_q, disk1_q, disk2_q, network_q;

void enqueue(struct linked_list *q, int job_num); // for fifo queue
void insert(struct linked_list *pq, int time, enum event_type type); // for event min pq
union node_data pop(struct linked_list *q);

int main(int argc, char *argv[]) {
	
	
	// read txt config
	FILE *fp = fopen(argv[1], "r");
	if (fp == NULL) {
		puts("eventsimulator: cannot open file");
		exit(1);
	}
	char buffer[32];
	const int SEED = atoi(fgets(buffer, 32, fp) + 5);
	const int INIT_TIME = atoi(fgets(buffer, 32, fp) + 10);
	const int FIN_TIME = atoi(fgets(buffer, 32, fp) + 9);
	const int ARRIVE_MIN = atoi(fgets(buffer, 32, fp) + 11);
	const int ARRIVE_MAX = atoi(fgets(buffer, 32, fp) + 11);
	const float QUIT_PROB = atof(fgets(buffer, 32, fp) + 10);
	const float NETWORK_PROB = atof(fgets(buffer, 32, fp) + 13);
	const int CPU_MIN = atoi(fgets(buffer, 32, fp) + 8);
	const int CPU_MAX = atoi(fgets(buffer, 32, fp) + 8);
	const int DISK1_MIN = atoi(fgets(buffer, 32, fp) + 10);
	const int DISK1_MAX = atoi(fgets(buffer, 32, fp) + 10);
	const int DISK2_MIN = atoi(fgets(buffer, 32, fp) + 10);
	const int DISK2_MAX = atoi(fgets(buffer, 32, fp) + 10);
	const int NETWORK_MIN = atoi(fgets(buffer, 32, fp) + 12);
	const int NETWORK_MAX = atoi(fgets(buffer, 32, fp) + 12);
	fclose(fp);
	
	srand(SEED);
	
	// add arrival of first event
	int job_num = 1;
	insert(&event_pq, INIT_TIME, JOB_ARRIVED);
	insert(&event_pq, FIN_TIME, SIMULATION_FINISHED);
	
	while (event_pq.length > 0) { // queue not empty
		// read event
		struct event data = pop(&event_pq).event_info;
		
		// handle event
		int job_id;
		switch (data.type) {
			
			case JOB_ARRIVED :
				printf("at time %d job %d arrived\n", data.time, job_num);
				
				// send job to cpu
				enqueue(&cpu_q, job_num++);
				if (cpu_q.length == 1)
					insert(&event_pq, rand() % (CPU_MAX-CPU_MIN+1) + CPU_MIN + data.time, CPU_FINISHED);
				
				// determine next arrival
				insert(&event_pq, rand() % (ARRIVE_MAX-ARRIVE_MIN+1) + ARRIVE_MIN + data.time, JOB_ARRIVED);
				break;
				
			case CPU_FINISHED : ;
				job_id = pop(&cpu_q).job_num;
				printf("at time %d job %d finished at the cpu\n", data.time, job_id);
				
				// job exits system
				int prob = rand() % 100;
				if (prob < 100*QUIT_PROB) {
				    if (cpu_q.length >= 1)
					    insert(&event_pq, rand() % (CPU_MAX-CPU_MIN+1) + CPU_MIN + data.time, CPU_FINISHED);
					break;
				}
				
				// job uses network
				prob = rand() % 100;
				if (prob < 100*NETWORK_PROB) {
					enqueue(&network_q, job_id);
					if (network_q.length == 1)
						insert(&event_pq, rand() % (NETWORK_MAX-NETWORK_MIN+1) + NETWORK_MIN + data.time, NETWORK_FINISHED);
				}
				
				// job does disk read
				else if (disk1_q.length < disk2_q.length) {
					enqueue(&disk1_q, job_id);
					if (disk1_q.length == 1)
						insert(&event_pq, rand() % (DISK1_MAX-DISK1_MIN+1) + DISK1_MIN + data.time, DISK1_FINISHED);
				}
				else if (disk1_q.length > disk2_q.length) {
					enqueue(&disk2_q, job_id);
					if (disk2_q.length == 1)
						insert(&event_pq, rand() % (DISK2_MAX-DISK2_MIN+1) + DISK2_MIN + data.time, DISK2_FINISHED);
				}
				else {
					prob = rand() % 2;
					if (prob) {
						enqueue(&disk1_q, job_id);
						if (disk1_q.length == 1)
							insert(&event_pq, rand() % (DISK1_MAX-DISK1_MIN+1) + DISK1_MIN + data.time, DISK1_FINISHED);
					}
					else {
						enqueue(&disk2_q, job_id);
						if (disk2_q.length == 1)
							insert(&event_pq, rand() % (DISK2_MAX-DISK2_MIN+1) + DISK2_MIN + data.time, DISK2_FINISHED);
					}
				}
				
				// check for next job
				if (cpu_q.length >= 1)
					insert(&event_pq, rand() % (CPU_MAX-CPU_MIN+1) + CPU_MIN + data.time, CPU_FINISHED);
				break;
				
			case DISK1_FINISHED : ;
				job_id = pop(&disk1_q).job_num;
				printf("at time %d job %d finished at disk1\n", data.time, job_id);
				
				// job goes to cpu
				enqueue(&cpu_q, job_id);
				if (cpu_q.length == 1)
					insert(&event_pq, rand() % (CPU_MAX-CPU_MIN+1) + CPU_MIN + data.time, CPU_FINISHED);
				
				// check for next job
				if (disk1_q.length >= 1)
					insert(&event_pq, rand() % (DISK1_MAX-DISK1_MIN+1) + DISK1_MIN + data.time, DISK1_FINISHED);
				break;
				
			case DISK2_FINISHED : ;
				job_id = pop(&disk2_q).job_num;
				printf("at time %d job %d finished at disk2\n", data.time, job_id);
				
				// job goes to cpu
				enqueue(&cpu_q, job_id);
				if (cpu_q.length == 1)
					insert(&event_pq, rand() % (CPU_MAX-CPU_MIN+1) + CPU_MIN + data.time, CPU_FINISHED);
				
				// check for next job
				if (disk2_q.length >= 1)
					insert(&event_pq, rand() % (DISK2_MAX-DISK2_MIN+1) + DISK2_MIN + data.time, DISK2_FINISHED);
				break;
				
			case NETWORK_FINISHED : ;
				job_id = pop(&network_q).job_num;
				printf("at time %d job %d finished at the network\n", data.time, job_id);
				
				// job goes to cpu
				enqueue(&cpu_q, job_id);
				if (cpu_q.length == 1)
					insert(&event_pq, rand() % (CPU_MAX-CPU_MIN+1) + CPU_MIN + data.time, CPU_FINISHED);
				
				// check for next job
				if (network_q.length >= 1)
					insert(&event_pq, rand() % (NETWORK_MAX-NETWORK_MIN+1) + NETWORK_MIN + data.time, NETWORK_FINISHED);
				break;
				
			case SIMULATION_FINISHED :
				printf("at time %d simulation finished\n", data.time);
				exit(0);
		}
	}
	exit(0);
}

void enqueue(struct linked_list *q, int job_num) {
	struct node *new_node = (struct node *) malloc(sizeof(struct node));
	new_node->data.job_num = job_num;
	new_node->next = NULL;
	if (q->head == NULL) {
		q->head = new_node;
		q->tail = new_node;
		q->length = 1;
	}
	else {
		q->tail->next =  new_node;
		q->tail = q->tail->next;
		q->length++;
	}
}

void insert(struct linked_list *pq, int time, enum event_type type) {
	struct node *new_node = (struct node *) malloc(sizeof(struct node));
	new_node->data.event_info.time = time;
	new_node->data.event_info.type = type;
	new_node->next = NULL;
	if (pq->head == NULL) { // no elements in list
		pq->head = new_node;
		pq->tail = new_node;
		pq->length = 1;
	}
	else if (time < pq->head->data.event_info.time) { // new element in head of pq
		new_node->next = pq->head;
		pq->head = new_node;
		pq->length++;
	}
	else { // new element is somewhere else
		struct node *current_node = pq->head;
		while (current_node->next != NULL && time > current_node->next->data.event_info.time) {
			current_node = current_node->next;
		}
		// current node is now the node before where the new node should go
		new_node->next = current_node->next;
		current_node->next = new_node;
		pq->length++;
		if (new_node->next == NULL)
			pq->tail = new_node;
	}
}

union node_data pop(struct linked_list *q) {
	union node_data data = q->head->data;
	if (q->head == q->tail) {
		free(q->head);
		q->head = q->tail = NULL;
	}
	else {
		struct node *next = q->head->next;
		free(q->head);
		q->head = next;
	}
	q->length--;
	return data;
}