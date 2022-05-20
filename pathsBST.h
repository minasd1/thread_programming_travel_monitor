#ifndef PATHSBST_H
#define PATHSBST_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/errno.h>
#include <sys/stat.h>
#include <dirent.h>


typedef struct pathNode* path_nodeptr;

typedef struct pathNode{
    char* name;
    int monitor;                          //THE MONITOR PROCESS THAT HANDLES THE COUNTRY
    path_nodeptr left, right;

}pathNode;
path_nodeptr pathBST_root, pathBST_tree;

/*------------------------------PATHS BINARY SEARCH TREE FUNCTIONS-------------------------------*/
void initialize_pathBST_node(path_nodeptr* n, char* key);
void insert_pathBST(path_nodeptr* n, char* key);
void send_numofRecords(path_nodeptr* key, int* fdnum, DIR* country_dir, struct dirent* dent, FILE* file, ssize_t read, size_t len, struct stat st, char* line, int numofMonitors, int* numofRecords,int *count);
void assign_alphabetic_RR(path_nodeptr* n, char **monitor_paths, int numofMonitors, int *count);
path_nodeptr search_pathBST(path_nodeptr n, char* key);
void write_path(path_nodeptr* n, FILE* file);
void pathBST_printdata(path_nodeptr *n);
void deallocate_pathBST(path_nodeptr* n);


#endif