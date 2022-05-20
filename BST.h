#ifndef BST_H
#define BST_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include "bloomFilter.h"
#include "skipList.h"
#include "bitops.h"
#include "socket_handling.h"

typedef struct Node* nodeptr;

//BST VIRUS NODES
typedef struct Node{
    char* virusName;
    int* BF;                //WE KEEP A BLOOMFILTER FOR EVERY VIRUS 
    skipList* vaccinated;   //WE ALSO KEEP A VACCINATED AND A NOT VACCINATED SKIPLIST FOR IT
    skipList* notVaccinated;
    nodeptr left, right;
}node;
nodeptr root, tree;

/*------------------MONITORS VIRUSES BINARY SEARCH TREE FUNCTIONS-------------------*/
void initialize(nodeptr* n, char* key, size_t bloomSize, size_t sList_size);
nodeptr searchBST(nodeptr n, char* key);
nodeptr insertBST(nodeptr* n, char* key, size_t bloomSize, size_t sList_size);
void send_viruses_bloomfilters(nodeptr* n, int* fdnum, int bloomSize);
void send_citizen_status(nodeptr* n, int* writefd, char* id, char* process);
void printdata(nodeptr *n);
void deallocate(nodeptr* n);

#endif