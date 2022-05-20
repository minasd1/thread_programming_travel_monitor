#ifndef HASH_H
#define HASH_H

/*HASH FUNCTIONS-USE THEM IN CITIZEN HASHTABLE AND VIRUSES BLOOMFILTERS*/
unsigned long djb2(char *str);
unsigned long sdbm(char *str);
unsigned long hash_i(char *str, unsigned int i);

    
#endif