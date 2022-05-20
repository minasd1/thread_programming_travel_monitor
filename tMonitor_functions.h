#ifndef TMONITOR_FUNCTIONS_H
#define TMONITOR_FUNCTIONS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <poll.h>
#include "error_handling.h"
#include "parentBST.h"
#include "country_namesBST.h"
#include "socket_handling.h"
#include "hash.h"
#include "list.h"
#include "childProcess.h"



/*-------------------------FUNCTIONS THAT EXECUTE OUR APPLICATION'S OPERATIONS---------------------------------*/
void travelRequest(char* command, char* args, int* writefd, int* readfd, size_t BF_size, int hash_num, char* process);
void travelStats(char* command, char* args);
void searchVaccinationStatus(char* command, char* args, int* writefd, int* readfd, int numMonitors, char* process);
void addVaccinationRecords(char* command, char* args, int* writefd, int* readfd, struct childProcess* monitor, size_t BF_size, char* process);










#endif