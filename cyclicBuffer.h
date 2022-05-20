#ifndef CYCLIC_BUFFER_H
#define CYCLIC_BUFFER_H

#include <stdio.h> 	
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <pthread.h>
#include <unistd.h>



typedef struct pool_t{
	int size;
	char** data;    
	int start;
	int end;
	int count;
} pool_t;

pthread_mutex_t buffer_mutex;
pthread_cond_t cond_nonempty;
pthread_cond_t cond_nonfull;
pool_t cyclicBuffer;
int numOfFiles;
int files_count;

/*------CYCLIC BUFFER FUNCTIONS------*/
void initialize_cyclicBuffer();
void place(pool_t * pool, char* data);
char* obtain(pool_t * pool);
void destroy_cyclicBuffer();



#endif