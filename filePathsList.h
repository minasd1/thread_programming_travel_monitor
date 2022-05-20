#ifndef FILEPATHSLIST_H
#define FILEPATHSLIST_H


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cyclicBuffer.h"


typedef struct fPathsListNode{
    
    char* data;
    struct fPathsListNode* next;

}fPathsListNode;


typedef struct FilePathsList{
    fPathsListNode* head;

}FilePathsList;


/*------------------FILE PATHS LIST FUNCTIONS-------------------*/
FilePathsList* initialize_filePathsList();
fPathsListNode* initialize_filePathsListNode(char *data);              
void insert_filePaths_list(FilePathsList** fList, char* data);
void place_to_cyclic_buffer(FilePathsList* list); 
void print_filePathslist(FilePathsList* list);    
void destroy_filePathsListNode(fPathsListNode* lNode);
void destroy_filePathslist(FilePathsList* list);













#endif