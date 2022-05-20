#ifndef PARENTBST_H
#define PARENTBST_H


#include <stdbool.h>
#include <stdio.h>
#include "bloomFilter.h"
#include "bitops.h"
#include "list.h"

typedef struct parent_virus_node* pVirus_nodeptr;

typedef struct BSTrecord{
    
    int total_requests;             //REQUESTS COUNTERS
    int accepted_requests;
    int rejected_requests;

}BSTrecord;

//BST VIRUS NODES
typedef struct parent_virus_node{
    char* virusName;
    int* BF;                        //WE KEEP A BLOOMFILTER FOR EVERY VIRUS
    BSTrecord* record; 
    RequestList* request_list;      //WE KEEP A LIST OF THE REQUESTS CORRESPONDING TO THE VIRUS
    pVirus_nodeptr left, right;
}parent_virus_node;
pVirus_nodeptr pVirus_root, pVirus_tree;

/*-----------------------------PARENT VIRUSES BINARY SEARCH TREE FUNCTIONS--------------------------------*/
BSTrecord* initialize_BSTRecord();
void pVirus_initialize(pVirus_nodeptr* n, char* key, size_t bloomSize);
pVirus_nodeptr insert_pVirusBST(pVirus_nodeptr* n, char* key, size_t bloomSize);
pVirus_nodeptr search_pVirusBST(pVirus_nodeptr n, char* key);
void get_requests_status(pVirus_nodeptr* n, int* total_requests, int* accepted_requests, int* rejected_requests);
void pVirusBST_printdata(pVirus_nodeptr *n);
void pVirusBST_deallocate(pVirus_nodeptr* n);





#endif