#ifndef COUNTRY_NAMESBST_H
#define COUNTRY_NAMESBST_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/errno.h>
#include <sys/stat.h>
#include <dirent.h>


typedef struct cNamesNode* cNames_nodeptr;

typedef struct cNamesNode{
    char* name;
    int monitor;                          //THE MONITOR PROCESS THAT HANDLES THE COUNTRY
    cNames_nodeptr left, right;

}cNamesNode;
cNames_nodeptr cNamesBST_root, cNamesBST_tree;

/*------------------------------COUNTRY NAMES BINARY SEARCH TREE FUNCTIONS-------------------------------*/
void initialize_cNamesBST_node(cNames_nodeptr* n, char* key);
void insert_country_namesBST(cNames_nodeptr* n, char* key);
void get_country_monitor(cNames_nodeptr* n, int numofMonitors, int *count);
cNames_nodeptr search_cNamesBST(cNames_nodeptr n, char* key);
void write_countries_names(cNames_nodeptr* n, FILE* file);
void cNames_printdata(cNames_nodeptr *n);
void deallocate_country_namesBST(cNames_nodeptr* n);


#endif