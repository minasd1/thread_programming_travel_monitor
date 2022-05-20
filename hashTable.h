#ifndef HASHTABLE_H
#define HASHTABLE_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "hash.h"


//Record with Citizen id data that every hashtable node points to
typedef struct hashRecord{
    char* firstName;
    char* lastName;    
    char* country;
    int age;
}hashRecord;

//Hashtable nodes
typedef struct hashTableList{
    char* id; 
    hashRecord* record;
    struct hashTableList* next;

}hashTableList;

//The hashtable
typedef struct hashTable{
    int size;
    hashTableList **table;
}hashTable;

//Initialize a hashTable object that we are going to use
hashTable htable;

/*------------------------------CITIZEN HASH TABLE FUNCTIONS----------------------------*/
void initialize_hashTable();
hashTableList* initialize_hashList(char* id);
hashRecord* initialize_hashRecord(char* firstName, char* lastName, char* country, int age);
void inserthashTable(char* id, char* firstName, char* lastName, char* country, int age);
hashTableList* search_hashList(char *key);
void destroy_hashList(hashTableList* htableList);
void destroy_hashTable();



#endif