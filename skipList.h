#ifndef SKIPLIST_H
#define SKIPLIST_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hashTable.h"

#define MAXVALUE "99999"

//RECORDS THAT THE SKIPLIST NODES POINT TO
typedef struct skipListRecord{
    char* isvaccinated;
    char* dateVaccinated;

}skipListRecord;

//NODES OF THE SKIPLIST
typedef struct skipListNode{
    char* id;
    skipListRecord* record;
    struct skipListNode** forward;
    hashRecord* hRecord;            //POINT TO A CITIZEN HASHTABLE RECORD TO AVOID DUPLICATES 

    
}skipListNode;

//SKIPLIST STRUCT
typedef struct skipList{
    int num_of_layers;
    skipListNode *header;
    skipListNode *final;


}skipList;


/*------------------------------VIRUSES SKIPLISTS FUNCTIONS-------------------------------------------*/
skipList* initialize_skipList(size_t skiplist_size);
skipListNode* initialize_node(char *id, int layers);
skipListRecord* initialize_nodeRecord(char* isvaccinated, char* dateVaccinated);
int get_numofLayers(int layers);
void insertskipList(skipList** sList, char* id, char* isvaccinated, char* dateVaccinated, int maxLayers);
skipListNode* search_skipList(skipList* sList, char* key);
void destroy_skipListNode(skipListNode* node);
void delete_skipListNode(skipList* sList, char* key, int maxLayers);
void destroy_skipList(skipList* sList);
void print_skipList(skipList* sList);


#endif