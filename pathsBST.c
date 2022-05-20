#include "pathsBST.h"

//INITIALIZE A PATH BINARY SEARCH TREE NODE
void initialize_pathBST_node(path_nodeptr* n, char* key){
    *n = malloc(sizeof(struct pathNode));
    (*n)->left = (*n)->right = NULL;
    (*n)->name = (char*)malloc(strlen(key)+1);
    strcpy((*n)->name, key);
    
}

//INSERT A PATH TO PATHS BST
void insert_pathBST(path_nodeptr* n, char* key){
    //IF NOT ALREADY EXISTS - THEN INITIALIZE IT
    if(*n == NULL){
        initialize_pathBST_node(n, key);
    }
    else{
        int result = strcmp(key, (*n)->name);
        if(result < 0){
            insert_pathBST(&((*n)->left), key);
        }
        else if(result > 0){
            insert_pathBST(&((*n)->right), key);
        }
    }

}


//ASSIGN PATHS TO MONITOR PROCESSES EVENLY USING ALPHABETIC ROUND-ROBIN
void assign_alphabetic_RR(path_nodeptr* n, char **monitor_paths, int numofMonitors, int *count){
    if(*n == NULL){
        return;
    }
    assign_alphabetic_RR(&((*n)->left), monitor_paths, numofMonitors, count);
    //ROUND-ROBIN, START FROM THE FIRST MONITOR PROCESS AGAIN
    if(*count >= numofMonitors){
        *count = 0;
    }
    char dash = '-';
    
    if(monitor_paths[*count] == NULL){
        monitor_paths[*count] = (char*)malloc(strlen((*n)->name) + 2);
        strcpy(monitor_paths[*count], (*n)->name);
        strcat(monitor_paths[*count], &dash);
    }
    else{
        monitor_paths[*count] = (char*)realloc(monitor_paths[*count], strlen(monitor_paths[*count]) + strlen((*n)->name) + 2);
        strcat(monitor_paths[*count], (*n)->name);
        strcat(monitor_paths[*count], &dash);
    }
                         
    (*count)++;
    assign_alphabetic_RR(&((*n)->right), monitor_paths, numofMonitors, count);
}

path_nodeptr search_pathBST(path_nodeptr n, char* key){
    //IF KEY NOT FOUND, RETURN NULL, ELSE RETURN A POINTER TO THE NODE WITH THAT KEY
    if(n == NULL){
        return NULL;
    }
    int result = strcmp(key,n->name);
    if(result == 0){
        return n;
    }
    else if(result < 0){
        return search_pathBST(n->left,key);
    }
    else{
        return search_pathBST(n->right, key);
    }
}

//PRINT PATH NAMES - ONLY FOR CHECKING PURPOSE
void pathBST_printdata(path_nodeptr* n){
    if(*n == NULL){
        return;
    }
    pathBST_printdata(&((*n)->left));
    printf("%s\n", (*n)->name);
    pathBST_printdata(&((*n)->right));
}

//FREE MEMORY ALLOCATED BY PATH BST
void deallocate_pathBST(path_nodeptr* n){
    if((*n) == NULL){
        return;
    }
    deallocate_pathBST(&((*n)->left));
    deallocate_pathBST(&((*n)->right));

    free((*n)->name);
    (*n)->name = NULL;
    
    free(*n);
    *n = NULL;
}