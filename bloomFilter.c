#include "bloomFilter.h"

void initialize_bloom(int** BF, int size){
    //ALLOCATE PROPER MEMORY
    *BF = (int*)malloc(size);                           
    if((*BF) == NULL){
        fprintf(stderr, "Error allocating memory\n");
        exit(1);
    }
    //FILL IT WITH ZEROS
    memset(*BF, 0, size);                                          

}

//DESTROY BF
void free_bloom(int** BF){
    free(*BF);
    *BF = NULL;
}

