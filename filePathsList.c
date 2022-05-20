#include "filePathsList.h"

//INTIALIZE FILE PATHS LIST
FilePathsList* initialize_filePathsList(){
    FilePathsList* list;
    list = (FilePathsList*)malloc(sizeof(FilePathsList));
    list->head = NULL;

    return list;
}

//INITIALIZE A FILE PATHS LIST NODE
fPathsListNode* initialize_filePathsListNode(char *data){
    //ALLOCATE PROPER MEMORY
    fPathsListNode* lNode = (fPathsListNode*)malloc(sizeof(fPathsListNode));
    
    lNode->data = (char*)malloc(strlen(data) + 1);
    strcpy(lNode->data, data);

    return lNode;
}

//INSERT A FILE PATH TO FILE PATHS LIST
void insert_filePaths_list(FilePathsList** fList, char* data){
    //INITIALIZE THE NODE        
    fPathsListNode* lNode = initialize_filePathsListNode(data);

    //INSERT THE NODE TO THE BEGINNING OF THE LIST
    lNode->next = (*fList)->head;
    (*fList)->head = lNode;

}

//INSERT ALL PATHS OF FILE PATHS LIST TO THE CYCLIC BUFFER
void place_to_cyclic_buffer(FilePathsList* list){
    fPathsListNode* current;
    current = list->head;

    while(current != NULL){
        place(&cyclicBuffer, current->data);
        current = current->next;
    }
}

//PRINT FILE PATHS LIST CONTENTS - ONLY FOR CHECKING PURPOSES
void print_filePathslist(FilePathsList* list){
    fPathsListNode* current;
    current = list->head;

    while(current != NULL){
        printf("%s\n", current->data);
        current = current->next;
    }
}

//DESTROY A FILE PATHS LIST NODE
void destroy_filePathsListNode(fPathsListNode* lNode){
    free(lNode->data);
    free(lNode);
}

//DESTROY FILE PATHS LIST
void destroy_filePathslist(FilePathsList* list){
    fPathsListNode* current;
    current = list->head;
    fPathsListNode* throwaway;
    //TRAVERSE THE LIST AND FREE EVERY NODE'S MEMORY
    while(current != NULL){
        throwaway = current;
        current = current->next;
        destroy_filePathsListNode(throwaway);

    }
    
    free(list);
}