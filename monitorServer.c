#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <math.h>
#include <pthread.h>
#include "citizenRecord.h"
#include "hash.h"
#include "BST.h"
#include "bitops.h"
#include "error_handling.h"
#include "hashTable.h"
#include "skipList.h"
#include "country_namesBST.h"
#include "socket_handling.h"
#include "cyclicBuffer.h"
#include "threadParams.h"
#include "filePathsList.h"


void* child_threads_func(void* args);
pthread_mutex_t database_mutex;



int main(int argc, char* argv[]){

    numOfFiles = 0;                 //TOTAL NUMBER OF FILES IN ALL COUNTRY DIRECTORIES THAT HAVE AT LEAST ONE RECORD               
	files_count = 0;                //WE USE THIS COUNTER WHILE INSERTING FILE PATHS ON CYCLIC BUFFER

    ssize_t rbytes;
    int port = 0;
    int numThreads= 0;
    int socketBufferSize = 0;
    int cyclicBufferSize = 0;
    int sizeOfBloom = 0;
    char* paths_conc;
    int path_num = 0;
    char* process = "child";
    int readfd;
    int writefd;
    DIR* input_dir = NULL;
    DIR* country_dir = NULL;
    struct dirent* dent;
    struct stat st;
    int dir_count = 0;
    FILE* file = NULL;
    char* line = NULL;
    size_t len = 0;
    int numofRecords = 0;
    int stop_input = -1;
    pid_t process_id;
    char slash = '/';
    

    //READ COMMAND LINE ARGUMENTS
    for(int i=0; i < argc; i++){
        if(strcmp(argv[i], "-p") == 0){
            i++;
            port = atoi(argv[i]);
        }
        else if(strcmp(argv[i], "-t") == 0){
            i++;
            numThreads = atoi(argv[i]);
        }
        else if(strcmp(argv[i], "-b") == 0){
            i++;
            socketBufferSize = atoi(argv[i]);
        }
        else if(strcmp(argv[i], "-c") == 0){
            i++;
            cyclicBufferSize = atoi(argv[i]);
        }
        else if(strcmp(argv[i], "-s") == 0){
            i++;
            sizeOfBloom = atoi(argv[i]);
        }
        else if(i == 11){
            paths_conc = argv[i];

        }

    }

    printf("Child process: process id is %d, parent id is %d\n"
    "socketBufferSize is %d\n"
    "cyclicBufferSize is %d\n"
    "sizeOfBloom is %d\n", 
    getpid(), getppid(), socketBufferSize, cyclicBufferSize, sizeOfBloom);
    
    int sock;

    struct sockaddr_in server;
    struct sockaddr* serverptr = (struct sockaddr*) &server;
    struct hostent* rem;

    sock = create_socket(AF_INET, SOCK_STREAM, 0);

    if ((rem = gethostbyname("localhost")) == NULL) {  
        exit(1);
    }

    server.sin_family = AF_INET;
    memcpy(&server.sin_addr, rem->h_addr, rem->h_length);
    server.sin_port = htons(port);

    connect_on_socket(sock, serverptr, sizeof(server));

    readfd = sock;
    writefd = sock;


    //COUNT NUMBER OF PATHS GIVEN TO THE MONITOR
    for(int i = 0; i < strlen(paths_conc); i++){
        char ch = paths_conc[i];
        if(ch == '-'){
            path_num++;
        }
    }

    //STRING ARRAY THAT HOLDS ALL THE PATHS
    char* paths[path_num];
    
    //TOKENIZE THE PATHS AND HOLD THEM TO THE STRING ARRAY
    for(int i = 0; i < path_num; i++){
        if(i == 0){
            paths[i] = strtok(paths_conc, "-");
        }
        else{
            paths[i] = strtok(NULL, "-");
        }

    }


    char* path_cp = (char*)malloc(strlen(paths[0]) + 1);
    strcpy(path_cp, paths[0]);

    char* input_dir_name = strtok(path_cp, "/");
    
    //OPEN INPUT DIRECTORY
    input_dir = opendir(input_dir_name);
    if (input_dir == NULL) {
        printf ("Cannot open directory '%s'\n", input_dir_name);
        exit(1);
    }

    //READ CONTENTS OF INPUT DIRECTORY - INSERT SUBDIRECTORIES' COUNTRY NAMES TO COUNTRY NAMES BST
    while((dent = readdir(input_dir)) != NULL){

        struct stat st;

        if(strcmp(dent->d_name, ".") == 0 || strcmp(dent->d_name, "..") == 0){
            continue;
        }
        if (fstatat(dirfd(input_dir), dent->d_name, &st, 0) < 0)
        {
            perror(dent->d_name);
            continue;
        }

        if (S_ISDIR(st.st_mode)) dir_count++;

        //INSERT COUNTRY NAME TO MONITOR'S COUNTRY NAMES BINARY SEARCH TREE
        insert_country_namesBST(&cNamesBST_root, dent->d_name);

    }
    rewinddir(input_dir);

    
    //INITIALIZE A LIST TO HOLD ALL FILES PATHS
    FilePathsList* fPathsList = initialize_filePathsList();
    

    for(int i = 0; i < path_num; i++){
        //OPEN DIRECTORY OF COUNTRY 
        country_dir = opendir(paths[i]);
        if (country_dir == NULL) {
            printf("Cannot open directory '%s'\n", paths[i]);
            exit(1);
        }
        chdir(paths[i]);


        //READ THE DIRECTORY'S CONTENTS - START READING IT'S FILES
        while((dent = readdir(country_dir)) != NULL){

            if(strcmp(dent->d_name, ".") == 0 || strcmp(dent->d_name, "..") == 0){
                continue;
            }

            if(stat(dent->d_name, &st) != 0){                   //FILE DOES NOT EXIST
                printf("%s", strerror(errno));
            }
            else if(st.st_size <= 1){                           //FILE IS EMPTY
                //printf("File %s is empty!\n", dent->d_name);
            }
            else{                                               //FILE EXISTS
                //OPEN FILE TO GET ITS RECORDS
                file = fopen(dent->d_name, "r");
                if(file == NULL){
                    printf("Could not open file!\n");
                    exit(1);
                }

                //GET CURRENT FILE'S FULL PATH
                char* file_full_path = (char*)malloc(strlen(paths[i]) + strlen(dent->d_name) + 2); //+2
                strcpy(file_full_path, paths[i]);
                strncat(file_full_path, &slash, 1);
                strcat(file_full_path, dent->d_name);

                //AND INSERT IT ON FILE PATHS LIST
                insert_filePaths_list(&fPathsList, file_full_path);
                //KEEP TRACK OF THE TOTAL NUMBER OF FILES
                numOfFiles++;

                //COUNT TOTAL NUMBER OF RECORDS TO INITIALIZE DATA STRUCTURES ACCORDINGLY
                while((rbytes = getline(&line, &len, file)) != -1){
                    numofRecords++;

                }

                free(line);
                line = NULL;

                free(file_full_path);
                file_full_path = NULL;

                fclose(file);

            }

        }


        closedir(country_dir);
        chdir("..");
        chdir("..");
    }
    

    //INITIALIZE THE CYCLIC BUFFER
    cyclicBuffer.size = cyclicBufferSize;
    initialize_cyclicBuffer();

    float fsize = (float)numofRecords*1.2;
    htable.size = (int)fsize;                           //GET CITIZEN'S HASHTABLE SIZE
    //int skiplistSize = 6;
    int skiplistSize = (log(numofRecords)/log(2));      //AND VIRUSES SKIPLIST SIZE ACCORDING TO NUMBER OF
    len = 0;
    initialize_hashTable();                             //INITIALIZE CITIZENS HASHTABLE

    //INITIALIZE MUTEXES - CONDITION VARIABLES USED IN PLACE AND OBTAIN FUNCTIONS 
    pthread_mutex_init(&buffer_mutex, 0);
    pthread_cond_init(&cond_nonempty, 0);
	pthread_cond_init(&cond_nonfull, 0);
    pthread_mutex_init(&database_mutex, 0);
    
    //THREADS FUNCTION PARAMETERS
    struct threadParams params;
    params.skipListSize = skiplistSize;
    params.bloomSize = sizeOfBloom;
    pthread_t threads[numThreads];

    //CREATE NUMTHREADS THREADS
    for(int i = 0; i < numThreads; i++){
        int error = pthread_create(&threads[i], NULL, &child_threads_func, &params);
        if(error == -1){
            exit(1);
        }
    }

    //PLACE FILE PATHS TO CYCLIC BUFFER
    place_to_cyclic_buffer(fPathsList);
    files_count++;
    pthread_cond_broadcast(&cond_nonempty);

    //WAIT FOR THE THREADS TO TERMINATE
    for(int i = 0; i < numThreads; i++){
        int error = pthread_join(threads[i], 0);
        if(error == -1){
            exit(1);
        }
    }

    destroy_cyclicBuffer();

    //SEND VIRUSES BLOOMFILTERS TO PARENT PROCESS
    send_viruses_bloomfilters(&root, &writefd, sizeOfBloom);
    write_to_socket(writefd, &stop_input, sizeof(int), process);


    //START RECEIVING COMMANDS FROM TRAVELMONITOR PROCESS
    char* received_command = NULL;
    char* operation = NULL;
    char* arguments;
    char* id;                           //ALL POSSIBLE ARGUMENTS TO BE TOKENIZED BY PARENT RECEIVED COMMAND
    char* date;
    char* countryFrom;
    char* countryTo;
    char* virusName;
    char* yes = "YES";                  //INFORM PARENT PROCESS WHETHER GIVEN TRAVELREQUEST IS ACCEPTED OR NOT
    int yes_len = strlen(yes) + 1;
    char* no = "NO";
    int no_len = strlen(no) + 1;
    int command_len;
    bool found;
    int total_requests = 0;             //KEEP TRACK OF THE MONITOR'S REQUEST COUNTERS
    int accepted_requests = 0;
    int rejected_requests = 0;
    char* logfile_init = "log_file.";   //PREFIX OF MONITOR PROCESS LOGFILE NAME
    char* logfile_name;
    FILE* logfile = NULL;
    int processId_digits;
    bool request_accepted;
    ssize_t readbytes;
    

    //READ LENGTH OF COMMAND TO BE RECEIVED
    while(((readbytes = read_from_socket(readfd, &command_len, sizeof(int), process)) >= 0)){
        received_command = (char*)malloc(command_len*sizeof(char));
            

        //READ THE COMMAND
        read_from_socket(readfd, received_command, command_len*sizeof(char), process);

        if(strcmp(received_command, "exit") != 0){
            operation = strtok(received_command, " ");
            arguments = strtok(NULL, "\n");
        }
        else{
            operation = strtok(received_command, "\n");
        }
            

        //IF PARENT PROCESS HAS SENT A TRAVELREQUEST COMMAND RECEIVED BY USER
        if(strcmp(operation, "/travelRequest") == 0){

            id = strtok(arguments, " ");
            hashTableList* htable_node = search_hashList(id);
            //CHECK IF CITIZEN EXISTS IN CITIZEN HASHTABLE
            if(htable_node != NULL){
                //IF EXISTS - TOKENIZE CITIZEN'S DATA
                date = strtok(NULL, " ");
                if(date != NULL){
                    countryFrom = strtok(NULL, " ");
                    if(countryFrom != NULL){
                        countryTo = strtok(NULL, " ");
                        if(countryTo != NULL){
                            virusName = strtok(NULL, "\n");
                            //GET ACCESS TO GIVEN VIRUS'S VIRUS BST NODE
                            nodeptr virus_node = searchBST(root, virusName);
                            //AND SEARCH IN IT'S SKIPLIST WHETHER CITIZEN IS VACCINATED TO IT OR NOT
                            skipListNode* sNode = search_skipList(virus_node->vaccinated, id);
                            if(sNode == NULL){
                                //NOT VACCINATED - INFORM PARENT PROCESS
                                write_to_socket(writefd, &no_len, sizeof(int), process);
                                write_to_socket(writefd, no, no_len, process);
                                rejected_requests++;
                            }
                            else{
                                //VACCINATED - INFORM PARENT PROCESS
                                write_to_socket(writefd, &yes_len, sizeof(int), process);
                                write_to_socket(writefd, yes, yes_len, process);
                                //AND SEND DATE OF VACCINATION TO PARENT
                                int date_len = strlen(sNode->record->dateVaccinated) + 1;
                                write_to_socket(writefd, &date_len, sizeof(int), process);
                                write_to_socket(writefd, sNode->record->dateVaccinated, date_len, process);

                                //RECEIVE FROM PARENT IF REQUEST IS ACCEPTED OR NOT, DEPENDING ON VACCINATION DATE
                                read_from_socket(readfd, &request_accepted, sizeof(bool), process);

                                //KEEP TRACK OF ACCEPTED, REJECTED AND TOTAL REQUESTS OF MONITOR PROCESS
                                if(request_accepted == true){
                                    accepted_requests++;
                                }
                                else{
                                    rejected_requests++;
                                }

                            }
                        }
                    }
                }
            }
            else{
                //CITIZEN DOES NOT EXIST IN DATABASE - HAS NOT BEEN VACCINATED FOR ANYTHING WE KNOW
                //INFORM PARENT PROCESS THAT CITIZEN IS NOT VACCINATED (piazza)
                write_to_socket(writefd, &no_len, sizeof(int), process);
                write_to_socket(writefd, no, no_len, process);
                rejected_requests++;
            }
            total_requests++;

        }
        else if(strcmp(operation, "/searchVaccinationStatus") == 0){

            id = strtok(arguments, "\n");
            hashTableList* citizen_node;
            citizen_node = search_hashList(id);
            //IF GIVEN CITIZEN ID FROM PARENT EXISTS
            if(citizen_node != NULL){
                //INFORM PARENT PROCESS THAT CITIZEN IS FOUND
                found = true;
                write_to_socket(writefd, &found, sizeof(bool), process);

                //SEND CITIZEN'S INFORMATION
                int name_len = strlen(citizen_node->record->firstName) + 1;
                write_to_socket(writefd, &name_len, sizeof(int), process);
                write_to_socket(writefd, citizen_node->record->firstName, name_len, process);

                int lastName_len = strlen(citizen_node->record->lastName) + 1;
                write_to_socket(writefd, &lastName_len, sizeof(int), process);
                write_to_socket(writefd, citizen_node->record->lastName, lastName_len, process);

                int country_len = strlen(citizen_node->record->country) + 1;
                write_to_socket(writefd, &country_len, sizeof(int), process);
                write_to_socket(writefd, citizen_node->record->country, country_len, process);

                write_to_socket(writefd, &(citizen_node->record->age), sizeof(int), process);

                //SEND CITIZEN'S VIRUSES STATUS - WE USE A FUNCTION FOR THAT
                send_citizen_status(&root, &writefd, id, process);
                write_to_socket(writefd, &stop_input, sizeof(int), process);
            }
            else{               //CITIZEN NOT FOUND
                found = false;
                write_to_socket(writefd, &found, sizeof(bool), process);
            }

        }else if(strcmp(operation, "/addVaccinationRecords") == 0){
            //NEW RECORDS FILES WITH NEW RECORDS HAVE BEEN INSERTED ON GIVEN COUNTRY'S DIRECTORY
            DIR* dir = NULL;
            char* dirname = (char*)malloc(strlen(input_dir_name) + strlen(arguments) + 3);
            strcpy(dirname, input_dir_name);
            strncat(dirname, &slash, 1);
            strcat(dirname, arguments);

            //INITIALIZE A FILE PATHS LIST   
            FilePathsList* localfplist = initialize_filePathsList();
            numOfFiles = 0;
            files_count = 0;

            //OPEN GIVEN COUNTRY'S DIRECTORY
            dir = opendir(dirname);
            if (dir == NULL) {
                printf("Cannot open directory '%s'\n", dirname);
                exit(1);
            }
            chdir(dirname);

            //READ THE DIRECTORY - START READING IT'S FILES
            while((dent = readdir(dir)) != NULL){

                if(strcmp(dent->d_name, ".") == 0 || strcmp(dent->d_name, "..") == 0){
                    continue;
                }

                if(stat(dent->d_name, &st) != 0){                   //FILE DOES NOT EXIST
                    printf("%s", strerror(errno));
                }
                else if(st.st_size <= 1){                           //FILE IS EMPTY
                    //printf("File %s is empty!\n", dent->d_name);
                }
                else{                                               //FILE EXISTS

                    //GET CURRENT FILE'S FULL PATH
                    char* file_full_path1 = (char*)malloc(strlen(dirname) + strlen(dent->d_name) + 3); //+2
                    strcpy(file_full_path1, dirname);
                    strncat(file_full_path1, &slash, 1);
                    strcat(file_full_path1, dent->d_name);

                    //AND INSERT IT ON FILE PATHS LIST
                    insert_filePaths_list(&localfplist, file_full_path1);

                    //KEEP TRACK OF THE TOTAL NUMBER OF FILES
                    numOfFiles++;


                    free(file_full_path1);
                    file_full_path1 = NULL;

                }

            }
                
            free(dirname);
            closedir(dir);
            chdir("..");
            chdir("..");

            initialize_cyclicBuffer();
                
            //CREATE NUMTHREADS THREADS  
            for(int i = 0; i < numThreads; i++){
                int error = pthread_create(&threads[i], NULL, &child_threads_func, &params);
                if(error == -1){
                    exit(1);
                }
            }
                
            //PLACE FILE PATHS TO CYCLIC BUFFER
            place_to_cyclic_buffer(localfplist);
            files_count++;
            pthread_cond_broadcast(&cond_nonempty);

            //WAIT FOR THE THREADS TO TERMINATE
            for(int i = 0; i < numThreads; i++){
                int error = pthread_join(threads[i], 0);
                if(error == -1){
                    exit(1);
                }
            }
                
            //SEND VIRUSES BLOOMFILTERS TO PARENT PROCESS
            send_viruses_bloomfilters(&root, &writefd, sizeOfBloom);
            write_to_socket(writefd, &stop_input, sizeof(int), process);

            destroy_filePathslist(localfplist);
            destroy_cyclicBuffer();

        }else if(strcmp(operation, "exit") == 0){
            printf("received command is %s\n", received_command);
            //GET PROCESSES ID - WE NEED IT FOR IT'S LOFILE NAME
            process_id = getpid();
            processId_digits = floor(log10(abs(process_id))) + 1;
            logfile_name = (char*)malloc(strlen(logfile_init)+processId_digits+1);
            sprintf(logfile_name, "%s%d", logfile_init, process_id);

            //OPEN LOGFILE
            logfile = fopen(logfile_name, "w");

            //WRITE COUNTRIES TO LOGFILE
            write_countries_names(&cNamesBST_root, logfile);

            //WRITE REQUESTS STATUS TO LOGFILE
            fprintf(logfile, "\nTOTAL TRAVEL REQUESTS %d\n", total_requests);
            fprintf(logfile, "ACCEPTED %d\n", accepted_requests);
            fprintf(logfile, "REJECTED %d\n", rejected_requests);

            fclose(logfile);
            free(logfile_name);
            free(received_command);
            received_command = NULL;

            break;
        }

        free(received_command);
        received_command = NULL;

    }


    
    //FREE ALL THE ALLOCATED MEMORY
    if(line)
        free(line);

    free(path_cp);
    closedir(input_dir);

    pthread_cond_destroy(&cond_nonempty);
    pthread_cond_destroy(&cond_nonfull);
    pthread_mutex_destroy(&buffer_mutex);
    pthread_mutex_destroy(&database_mutex);

    destroy_filePathslist(fPathsList);
    deallocate_country_namesBST(&cNamesBST_root);
    deallocate(&root);
    destroy_hashTable();

    printf("child process terminating: process id is %d, parent id is %d\n", getpid(), getppid());

    
    

    return 0;

}


//CHILD THREADS FUNCTION - OBTAIN PATHS OF FILES FROM CYCLIC BUFFER
//INSERT FILE RECORDS TO DATABASE

void* child_threads_func(void* args){
    
    FILE* file = NULL;
    ssize_t rbytes;
    struct citizenRecord record;
    hashTableList* citizen_node;
    unsigned long BF_position;
    char* line = NULL;
    size_t len = 0;
    struct threadParams* tParams = args;
    
    int skiplistSize = tParams->skipListSize;
    int sizeOfBloom = tParams->bloomSize;
    //printf("thread %lu: skiplistsize is %d\n", pthread_self(), skiplistSize);
    //printf("thread %lu: bloomsize is %d\n", pthread_self(), sizeOfBloom);
    char* data;

    pthread_mutex_lock(&database_mutex);
    

    while(files_count <= numOfFiles || cyclicBuffer.count > 0){
        
        //TAKE PATH OF A FILE FROM CYCLIC BUFFER
        data = obtain(&cyclicBuffer);
        
        if(data == NULL){
            break;
        }

        //AND OPEN IT TO ACCESS ITS' RECORDS
        file = fopen(data, "r");
        if(file == NULL){
            printf("Could not open file!\n");
            exit(1);
        }
        
        while((rbytes = getline(&line, &len, file)) != -1){
            //TOKENIZE LINE STRINGS AND ASSIGN THEM TO PROPER RECORD MEMBERS
            record.citizenID = strtok(line, " ");
            record.firstName = strtok(NULL, " ");
            record.lastName = strtok(NULL, " ");
            record.country = strtok(NULL, " ");
            record.age = atoi(strtok(NULL, " "));
            record.virusName = strtok(NULL, " ");
            record.isvaccinated = strtok(NULL, " ");
            record.dateVaccinated = strtok(NULL, "\n");
            
            //ERROR, NOT VACCINATED CITIZEN MUST NOT HAVE A VACCINATION DATE
            if((strcmp(record.isvaccinated, "NO") == 0) && (record.dateVaccinated != NULL)){
                //printf("ERROR IN RECORD %s %s %s %s %d %s %s %s\n", record.citizenID ,record.firstName, record.lastName, record.country, record.age, record.virusName, record.isvaccinated, record.dateVaccinated);
            }
            else{

                citizen_node = search_hashList(record.citizenID);
                if(citizen_node != NULL){
                    //printf("citizen id is %s\n", citizen_node->id);
                }
                if ((citizen_node != NULL) && ((strcmp(record.firstName, citizen_node->record->firstName) != 0) || (strcmp(record.lastName, citizen_node->record->lastName) != 0) || (strcmp(record.country, citizen_node->record->country) !=0) || (record.age != citizen_node->record->age))){
                    //INCONSISTENCY IN GIVEN CITIZEN DATA
                    //printf("Inconsistent record!\n");
                }
                else{   //INSERT PROPER RECORD MEMBERS TO HASHTABLE

                    inserthashTable(record.citizenID, record.firstName, record.lastName, record.country, record.age);
                    //INSERT THE VIRUS TO BST OR GET A POINTER TO ITS NODE
                    tree = insertBST(&root, record.virusName, sizeOfBloom, skiplistSize);
                    if(strcmp(record.isvaccinated, "YES") == 0){

                        if(search_skipList(tree->notVaccinated, record.citizenID) == NULL){
                            //UPDATE BLOOM FILTER
                            for(int i = 0; i < 16; i++){
                                BF_position = hash_i(record.citizenID, i) % sizeOfBloom;
                                setBit(tree->BF, BF_position);
                            }
                            //INSERT IN VACCINATED SKIPLIST
                            insertskipList(&(tree->vaccinated), record.citizenID, record.isvaccinated, record.dateVaccinated, skiplistSize);
                        }
                        else{
                            //CITIZEN ALREADY IN NON-VACCINATED LIST
                            //printf("Inconsistent record!\n");
                        }
                    }
                    else{
                        if(search_skipList(tree->vaccinated, record.citizenID) == NULL){
                            insertskipList(&(tree->notVaccinated), record.citizenID, record.isvaccinated, record.dateVaccinated, skiplistSize);
                        }
                        else{
                            //CITIZEN ALREADY IN VACCINATED LIST
                            //printf("Inconsistent record!\n");
                        }
                    }
                }

            }

            free(line);
            line = NULL;


        }

        free(line);
        line = NULL;
        fclose(file);
    
    }

    pthread_mutex_unlock(&database_mutex);
    
    
    return NULL;
}

