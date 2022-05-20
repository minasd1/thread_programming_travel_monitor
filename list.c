//#define _GNU_SOURCE
#include "list.h"

//INITIALIZE LIST
RequestList* initialize_list(){
    RequestList* list;
    list = (RequestList*)malloc(sizeof(RequestList));
    list->head = NULL;

    return list;
}

//INITIALIZE A LIST RECORD
listRecord* initialize_listRecord(char* country, char* date){        
    listRecord* lRecord = (listRecord*)malloc(sizeof(listRecord));
    
    lRecord->country = (char*)malloc(strlen(country)+1);
    strcpy(lRecord->country, country);
    lRecord->date = (char*)malloc(strlen(date) + 1);
    strcpy(lRecord->date, date);
    lRecord->accepted = false;

    return lRecord;
}

//INITIALIZE A LIST NODE
listNode* initialize_listNode(char *country, char* date){
    //ALLOCATE PROPER MEMORY
    listNode* lNode = (listNode*)malloc(sizeof(listNode));
    //INITIALIZE NODE'S RECORD
    lNode->record = initialize_listRecord(country, date);

    return lNode;
}

//INSERT A NODE TO LIST
listNode* insert_request_list(RequestList** list, char* country, char* date, bool status){  
    //INITIALIZE THE NODE        
    listNode* lNode = initialize_listNode(country, date);
    lNode->record->accepted = status;

    //INSERT THE NODE TO THE BEGINNING OF THE LIST
    lNode->next = (*list)->head;
    (*list)->head = lNode;

    return lNode;
}

//SEARCH IF GIVEN KEY EXISTS IN LIST
listNode* search_list(RequestList* list, char* key){
    listNode* current;
    current = list->head;
    //TRAVERSE THE LIST TO FIND IF GIVEN KEY EXISTS
    while(current != NULL){
        if(strcmp(current->record->country, key) == 0){            //anti gia name vazoume country
            return current;
        }
        else{
            current = current->next;
        }
    }
    return current;
}

//FREE A LIST NODE MEMORY
void destroy_listNode(listNode* lNode){
    free(lNode->record->date);
    free(lNode->record->country);
    free(lNode->record);
    free(lNode);
}

//FREE MEMORY ALLOCATED FROM LIST
void destroy_list(RequestList* list){
    listNode* current;
    current = list->head;
    listNode* throwaway;
    //TRAVERSE THE LIST AND FREE EVERY NODE'S MEMORY
    while(current != NULL){
        throwaway = current;
        current = current->next;
        destroy_listNode(throwaway);

    }
    
    free(list);
}
