#include "BST.h"


//INITIALIZE A BST NODE
void initialize(nodeptr* n, char* key, size_t bloomSize, size_t sList_size){
    //ALLOCATE PROPER MEMORY FOR DATA AND THE DATA STRUCTURE MEMBERS
    *n = (struct Node*)malloc(sizeof(struct Node));
    (*n)->left = (*n)->right = NULL;
    (*n)->virusName = (char*)malloc(strlen(key) + 1);
    strcpy((*n)->virusName, key);
    initialize_bloom(&((*n)->BF), bloomSize);
    (*n)->vaccinated = initialize_skipList(sList_size);
    (*n)->notVaccinated = initialize_skipList(sList_size);
}

//INSERT A NODE TO BST, RETURN A POINTER TO IT IF ALREADY EXISTS
nodeptr insertBST(nodeptr* n, char* key, size_t bloomSize, size_t sList_size){
    //IF NODE DOES NOT ALREADY EXISTS, INITIALIZE IT
    if(*n == NULL){
        initialize(n, key, bloomSize, sList_size);
        return *n;
    }
    int result = strcmp(key, (*n)->virusName);
    //IF NODE ALREADY EXISTS, RETURN A POINTER TO IT 
    if(result == 0){
        return *n;
    }
    else if(result < 0){
        return insertBST(&((*n)->left), key, bloomSize, sList_size);
    }
    else{
        return insertBST(&((*n)->right), key, bloomSize, sList_size);
    }
    
}

//SEARCH BST FOR GIVEN KEY
nodeptr searchBST(nodeptr n, char* key){
    //IF KEY NOT FOUND, RETURN NULL, ELSE RETURN A POINTER TO THE NODE WITH THAT KEY
    if(n == NULL){
        return NULL;
    }
    int result = strcmp(key,n->virusName);
    if(result == 0){
        return n;
    }
    else if(result < 0){
        return searchBST(n->left,key);
    }
    else{
        return searchBST(n->right, key);
    }
}

void send_viruses_bloomfilters(nodeptr* n, int* fdnum, int bloomSize){
    if(*n == NULL){
        return;
    }
    send_viruses_bloomfilters(&((*n)->left), fdnum, bloomSize);
    send_viruses_bloomfilters(&((*n)->right), fdnum, bloomSize);

    int stop_input = -1;
    int str_size = strlen((*n)->virusName) + 1;
    //WRITE LENGTH OF VIRUSNAME TO PARENT PROCESS
    write(*fdnum, &str_size, sizeof(int));
    //WRITE VIRUSNAME
    write(*fdnum, (*n)->virusName, str_size*sizeof(char));
    //SEND BLOOM FILTER OF VIRUS - WE USE A LOOP AND SEND ALL THE BYTES OF MONITOR VIRUS'S
    //BLOOMFILTER THAT HAVE BEEN SET TO 1 TO PARENT PROCESS, SO THAT THE PARENT PROCESS
    //SETS THE SAME BITS IN IT'S VIRUS'S BLOOMFILTER
    for(int i = 0; i < bloomSize; i++){
        //IF CURRENT BIT IS SET TO 1
        if(testBit((*n)->BF, i) == true){
            //SEND IT TO PARENT PROCESS
            write(*fdnum, &i, sizeof(int));
        }
    }
    //INFORM PARENT PROCESS THAT WE ARE DONE
    write(*fdnum, &stop_input, sizeof(int));

}

//WE USE THIS FUNCTION TO SEND A CITIZEN'S VIRUSES STATUS TO PARENT PROCESS
void send_citizen_status(nodeptr* n, int* writefd, char* id, char* process){
    if(*n == NULL){
        return;
    }
    send_citizen_status(&((*n)->left), writefd, id, process);
    send_citizen_status(&((*n)->right), writefd, id, process);
    int len;

    //SEARCH FOR THE GIVEN CITIZEN ID IN EVERY VIRUS'S VACCINATED AND NOT VACCINATED SKIPLIST
    skipListNode* node = search_skipList((*n)->vaccinated, id);
    if(node != NULL){       //IF FOUND IN VACCINATED LIST, PRINT THE PROPER DATA
        //printf("%s %s %s\n", (*n)->virusName, node->record->isvaccinated, node->record->dateVaccinated);
        len = strlen((*n)->virusName) + 1;
        write_to_socket(*writefd, &len, sizeof(int), process);
        write_to_socket(*writefd, (*n)->virusName, len, process);

        len = strlen(node->record->isvaccinated) + 1;
        write_to_socket(*writefd, &len, sizeof(int), process);
        write_to_socket(*writefd, node->record->isvaccinated, len, process);

        len = strlen(node->record->dateVaccinated) + 1;
        write_to_socket(*writefd, &len, sizeof(int), process);
        write_to_socket(*writefd, node->record->dateVaccinated, len, process);
    }
    node = search_skipList((*n)->notVaccinated, id);
    if(node != NULL){       //SAME IF FOUND IN VIRUS'S NOT VACCINATED SKIPLIST
        //printf("%s %s\n", (*n)->virusName, node->record->isvaccinated);
        len = strlen((*n)->virusName) + 1;
        write_to_socket(*writefd, &len, sizeof(int), process);
        write_to_socket(*writefd, (*n)->virusName, len, process);
        
        len = strlen(node->record->isvaccinated) + 1;
        write_to_socket(*writefd, &len, sizeof(int), process);
        write_to_socket(*writefd, node->record->isvaccinated, len, process);
    }
}


//PRINT BST DATA (ALL VIRUSES NAMES) - ONLY FOR CHECKING PURPOSES
void printdata(nodeptr* n){
    if(*n == NULL){
        return;
    }
    printdata(&((*n)->left));
    printdata(&((*n)->right));

    printf("%s\n", (*n)->virusName);
}

//FREE BST MEMORY
void deallocate(nodeptr* n){
    if((*n) == NULL){
        return;
    }
    deallocate(&((*n)->left));
    deallocate(&((*n)->right));

    free((*n)->virusName);
    (*n)->virusName = NULL;
    free_bloom(&((*n)->BF));
    destroy_skipList((*n)->vaccinated);
    destroy_skipList((*n)->notVaccinated);
    free(*n);
    *n = NULL;
    
}






