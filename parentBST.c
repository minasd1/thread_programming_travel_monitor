#include "parentBST.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>

//INITIALIZE A BST RECORD
BSTrecord* initialize_BSTRecord(){        
    BSTrecord* bRecord = (BSTrecord*)malloc(sizeof(BSTrecord));
    
    bRecord->total_requests = 0;
    bRecord->accepted_requests = 0;
    bRecord->rejected_requests = 0;

    return bRecord;
}


//INITIALIZE A BST NODE
void pVirus_initialize(pVirus_nodeptr* n, char* key, size_t bloomSize){
    //ALLOCATE PROPER MEMORY FOR DATA AND THE DATA STRUCTURE MEMBERS
    *n = (struct parent_virus_node*)malloc(sizeof(struct parent_virus_node));
    (*n)->left = (*n)->right = NULL;
    (*n)->virusName = (char*)malloc(strlen(key) + 1);
    strcpy((*n)->virusName, key);
    initialize_bloom(&((*n)->BF), bloomSize);
    (*n)->record = initialize_BSTRecord();
    (*n)->request_list = initialize_list();
}

//INSERT A NODE TO BST, RETURN A POINTER TO IT IF ALREADY EXISTS
pVirus_nodeptr insert_pVirusBST(pVirus_nodeptr* n, char* key, size_t bloomSize){
    //IF NODE DOES NOT ALREADY EXISTS, INITIALIZE IT
    if(*n == NULL){
        pVirus_initialize(n, key, bloomSize);
        return *n;
    }
    int result = strcmp(key, (*n)->virusName);
    //IF NODE ALREADY EXISTS, RETURN A POINTER TO IT 
    if(result == 0){
        return *n;
    }
    else if(result < 0){
        return insert_pVirusBST(&((*n)->left), key, bloomSize);
    }
    else{
        return insert_pVirusBST(&((*n)->right), key, bloomSize);
    }
    
}

//SEARCH BST FOR GIVEN KEY
pVirus_nodeptr search_pVirusBST(pVirus_nodeptr n, char* key){
    //IF KEY NOT FOUND, RETURN NULL, ELSE RETURN A POINTER TO THE NODE WITH THAT KEY
    if(n == NULL){
        return NULL;
    }
    int result = strcmp(key,n->virusName);
    if(result == 0){
        return n;
    }
    else if(result < 0){
        return search_pVirusBST(n->left,key);
    }
    else{
        return search_pVirusBST(n->right, key);
    }
}

//SEND NUMBER OF REQUESTS TO LOGFILE OF PARENT PROCESS
void get_requests_status(pVirus_nodeptr* n, int* total_requests, int* accepted_requests, int* rejected_requests){
    if(*n == NULL){
        return;
    }
    get_requests_status(&((*n)->left), total_requests, accepted_requests, rejected_requests);
    get_requests_status(&((*n)->right), total_requests, accepted_requests, rejected_requests);

    listNode* current = (*n)->request_list->head;
    while(current != NULL){
        if(current->record->accepted == true){
            (*accepted_requests)++;
        }
        else{
            (*rejected_requests)++;
        }
        (*total_requests)++;
        
        current = current->next;
    }    

}

//PRINT BST DATA (ALL VIRUSES NAMES) - CHECKING PURPOSES ONLY
void pVirusBST_printdata(pVirus_nodeptr* n){
    if(*n == NULL){
        return;
    }
    pVirusBST_printdata(&((*n)->left));
    pVirusBST_printdata(&((*n)->right));

    printf("%s\n", (*n)->virusName);
}

//FREE BST MEMORY
void pVirusBST_deallocate(pVirus_nodeptr* n){
    if((*n) == NULL){
        return;
    }
    pVirusBST_deallocate(&((*n)->left));
    pVirusBST_deallocate(&((*n)->right));

    free((*n)->virusName);
    (*n)->virusName = NULL;
    free_bloom(&((*n)->BF));
    free((*n)->record);
    destroy_list((*n)->request_list);
    free(*n);
    *n = NULL;
    
}