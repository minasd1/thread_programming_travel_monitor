#include "hashTable.h"
#include "hash.h"

//INITIALIZE OUR HASH TABLE
void initialize_hashTable(){
    int size = htable.size;
    htable.table = (hashTableList**)malloc(sizeof(hashTableList*) * size);
    for(int i = 0; i < size; i++){
        htable.table[i] = NULL;
    }
    
}

//INITIALIZE HASHTABLE NODE
hashTableList* initialize_hashList(char* id){

    hashTableList* htableList;
    htableList = (hashTableList*)malloc(sizeof(hashTableList));
    htableList->id = (char*)malloc(strlen(id)+1);
    strcpy(htableList->id, id);
    htableList->next = NULL;
    htableList->record = NULL;

    return htableList;
}

//INITIALIZE HASHTABLE NODE RECORD
hashRecord* initialize_hashRecord(char* firstName, char* lastName, char* country, int age){
    
    hashRecord* record;
    //ALLOCATE PROPER MEMORY
    record = (hashRecord*)malloc(sizeof(hashRecord));
    record->firstName = (char*)malloc(sizeof(char)*strlen(firstName)+1);
    strcpy(record->firstName, firstName);
    record->lastName = (char*)malloc(sizeof(char)*strlen(lastName) + 1);
    strcpy(record->lastName, lastName);
    record->country = (char*)malloc(sizeof(char)*strlen(country)+1);
    strcpy(record->country, country);                                           
    
    record->age = age;

    return record;
}

void inserthashTable(char* id, char* firstName, char* lastName, char* country, int age){
    hashRecord* record = initialize_hashRecord(firstName, lastName, country, age);
    hashTableList* htableList = initialize_hashList(id);
    htableList->record = record;
    unsigned long hash = sdbm(id)%htable.size;  //HASH TO GET THE PROPER HASHTABLE LIST TO INSERT NODE TO
    if(htable.table[hash] == NULL){             //IF IT IS THE FIRST NODE IN THE LIST
        htableList->next = NULL;
        htable.table[hash] = htableList;
        return;
    }
    htableList->next = htable.table[hash]; //INSERT NODE TO THE BEGINNING OF THE LIST
    htable.table[hash] = htableList;       //ALSO, BEGINNING OF THE LIST POINTS TO THE NODE 

}

hashTableList* search_hashList(char *key){
   
    unsigned long hash = sdbm(key)%htable.size; //HASH GIVEN KEY TO GET PROPER HASHTABLE LIST
    hashTableList* current = htable.table[hash];
    while(current != NULL){                     //TRAVERSE THE LIST TO FIND IF GIVEN KEY EXISTS
        if(strcmp(current->id, key) == 0){
            return current;
        }
        else{
            current = current->next;
        }
    }
    return current;

}

void destroy_hashList(hashTableList* htableList){   //DESTROY A HASHTABLE LIST
    hashTableList* current = htableList;
    hashTableList* throwaway;
    while(current != NULL){                         //TRAVERSE THE LIST
        throwaway = current;
        current = current->next;
        free(throwaway->record->firstName);         //FREE NODE'S ALLOCATED MEMORY
        free(throwaway->record->lastName);
        free(throwaway->record->country);
        free(throwaway->record);
        free(throwaway->id);
        free(throwaway);
    }
}

void destroy_hashTable(){                   //FREE HASHTABLE MEMORY
    int size = htable.size;
    for(int i = 0; i < size; i++){
        destroy_hashList(htable.table[i]);
    }
    free(htable.table);
}
