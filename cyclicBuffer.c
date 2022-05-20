#include "cyclicBuffer.h"

//INITIALIZE CYCLIC BUFFER
void initialize_cyclicBuffer() {
	int size = cyclicBuffer.size;
	cyclicBuffer.data = (char**)malloc(sizeof(char*) * size);
	for(int i = 0; i < size; i++){
        cyclicBuffer.data[i] = NULL;
    }

	cyclicBuffer.start = 0;
	cyclicBuffer.end = -1;
	cyclicBuffer.count = 0;
}

//PLACE A PATH IN THE BUFFER
void place(pool_t * pool, char* data) {
	pthread_mutex_lock(&buffer_mutex);
	while (pool->count >= cyclicBuffer.size) {
		//printf(">> Found Buffer Full \n");
		pthread_cond_wait(&cond_nonfull, &buffer_mutex);
	}
	pool->end = (pool->end + 1) % cyclicBuffer.size;
	pool->data[pool->end] = data;								
	pool->count++;
	files_count++;
	pthread_cond_broadcast(&cond_nonempty);
	pthread_mutex_unlock(&buffer_mutex);
	
}

//RETRIEVE A PATH FROM THE BUFFER
char* obtain(pool_t * pool) {
	char* data;
	pthread_mutex_lock(&buffer_mutex);
	while (pool->count <= 0 && files_count <= numOfFiles) {
		//printf(">> Found Buffer Empty \n");
		pthread_cond_wait(&cond_nonempty, &buffer_mutex);
	}
	
	data = pool->data[pool->start];
	pool->start = (pool->start + 1) % cyclicBuffer.size;
	pool->count--;
		
	pthread_cond_signal(&cond_nonfull);
	pthread_mutex_unlock(&buffer_mutex);
	return data;
	
}

//DESTROY CYCLIC BUFFER
void destroy_cyclicBuffer(){
	
    free(cyclicBuffer.data);
}
