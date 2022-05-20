#ifndef CHILDPROCESS_H
#define CHILDPROCESS_H

#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

/* MONITOR PROCESSES STRUCT */
typedef struct childProcess{
    pid_t pid;
    char* read_pipe;
    char* write_pipe;
    int status;

}childProcess;


#endif