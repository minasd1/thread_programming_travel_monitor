#ifndef LIST_H
#define LIST_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>


typedef struct listRecord{
    
    char* country;
    char* date;
    bool accepted;

}listRecord;

typedef struct listNode{
    
    listRecord* record;
    struct listNode* next;

}listNode;

typedef struct RequestList{
    listNode* head;

}RequestList;

/*-----------------------------REQUESTS LIST FUNCTIONS----------------------------------*/
RequestList* initialize_list();
listNode* initialize_listNode(char *country, char* date);
listRecord* initialize_listRecord(char* country, char* date);                
listNode* insert_request_list(RequestList** cList, char* country, char* date, bool status);     
listNode* search_list(RequestList* list, char* key);
void destroy_listNode(listNode* lNode);
void destroy_list(RequestList* list);


#endif