#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>      
#include <netdb.h>          
#include <sys/stat.h>
#include <sys/errno.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <math.h>
#include <sys/ioctl.h>
#include <poll.h>
#include "country_namesBST.h"
#include "error_handling.h"
#include "skipList.h"
#include "bitops.h"
#include "hash.h"
#include "list.h"
#include "tMonitor_functions.h"
#include "socket_handling.h"
#include "pathsBST.h"
#include "parentBST.h"
#include "childProcess.h"

extern int errno;

#define PERMS 0666  //PERMISSIONS OF DIRECTORIES 
#define K 16        //NUMBER OF HASH FUNCTIONS


int main(int argc, char* argv[]){
    srand(time(NULL));

    int numMonitors = 0;
    size_t socketBufferSize = 0;
    size_t cyclicBufferSize = 0;
    size_t sizeOfBloom = 0;
    DIR *input_dir = NULL;
    char *input_dir_name = NULL;
    int numThreads = 0;
    struct dirent *dent;
    int dir_count = 0;
    char* process = "parent";
    size_t len = 0;
    pid_t parent_id;
    pid_t wait_pid;
    int status = 0;
    int min_port = 8000;
    int max_port = 60000;
    int port = rand() % (max_port - min_port + 1) + min_port;  //PORTT BETWEEN THESE VALUES
    int max_clients = 195; //COUNTRIES IN THE WORLD - WE NEVER CREATE MORE MONITORS


    //READ COMMAND LINE ARGUMENTS
    for(int i=0; i < argc; i++){
        if(strcmp(argv[i], "-m") == 0){
            i++;
            numMonitors = atoi(argv[i]);
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
        else if(strcmp(argv[i], "-i") == 0){
            i++;
            input_dir = opendir(argv[i]);
            if (input_dir == NULL) {
                printf ("Cannot open directory '%s'\n", argv[i]);
                exit(1);
            }
            input_dir_name = argv[i];
        }
        else if(strcmp(argv[i], "-t") == 0){
            i++;
            numThreads = atoi(argv[i]);
        }
    }

    //KEEP TRACK OF THE NUMBER OF COUNTRY SUBDIRECTORIES IN INPUT DIRECTORY
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
    
        if (S_ISDIR(st.st_mode)){
            dir_count++;
        } 

    }
    rewinddir(input_dir);


    //IF MONITORS ARE MORE THAN COUNTRIES
    if(numMonitors > dir_count){
        //SET THEM TO BE EQUAL TO COUNTRIES - EACH MONITOR WILL HANDLE ONE COUNTRY
        numMonitors = dir_count;
    }

    struct childProcess monitor[numMonitors];
    int readfd[numMonitors];                //READ FILE DESCRIPTORS OF NAMED PIPES
    int writefd[numMonitors];               //WRITE FILE DESCRIPTORS
    int mainfd = 0;                         
    char* path[dir_count];                  //COUNTRY DIRECTORIES PATHS
    char* monitor_paths[numMonitors];       //PATHS ASSIGNED TO EACH MONITOR
    int exec_return_value = 0;
    char* monitor_process = "./monitorServer";

    for(int i = 0; i < numMonitors; i++){
        monitor_paths[i] = NULL;
    }
    
    int count = 0;

    for(int i = 0; i < dir_count; i++){
        path[i] = (char*)malloc(strlen(input_dir_name) + 1);
        strcpy(path[i], input_dir_name);
        
    }

    //READ INPUT DIRECTORY'S CONTENTS - INSERT COUNTRIES' SUBDIRECTORIES PATHS TO PATH BST
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
    
        if (S_ISDIR(st.st_mode)){
            
            path[count] = (char*)realloc(path[count], strlen(path[count]) + strlen(dent->d_name) + 1);
            strcat(path[count], dent->d_name);
            //INSERTION
            insert_pathBST(&pathBST_root, path[count]);

            free(path[count]);
            path[count] = NULL;
            count++;
            
        } 
        

    }
    rewinddir(input_dir);
    
    count = 0;
    //ASSIGN PATHS TO MONITOR PROCESSES EVENLY USING ALPHABETIC ROUND-ROBIN
    assign_alphabetic_RR(&pathBST_root, monitor_paths, numMonitors, &count);
    
    //INITIALIZE SERVER - SOCKETS
    struct sockaddr_in server;
    struct sockaddr *serverptr = (struct sockaddr *) &server;
    mainfd = create_socket(AF_INET, SOCK_STREAM, 0);
    server.sin_family = AF_INET; 
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(port);

    bind_socket(mainfd, serverptr, sizeof (server));
    listen_for_connections(mainfd, max_clients);        

    //PASS ARGUMENTS TO MONITOR PROCESSES AS STRINGS
    int port_digits = floor(log10(abs(port))) + 1;
    char port_str[port_digits+1];
    snprintf(port_str, sizeof(port_str), "%d", port);

    int numThreads_digits = floor(log10(abs(numThreads))) + 1;
    char numThreads_str[numThreads_digits+1];
    snprintf(numThreads_str, sizeof(numThreads_str), "%d", numThreads);

    int socketBufferSize_digits = floor(log10(abs(socketBufferSize))) + 1;
    char socketBufferSize_str[socketBufferSize_digits+1];
    snprintf(socketBufferSize_str, sizeof(socketBufferSize_str), "%ld", socketBufferSize);

    int cyclicBufferSize_digits = floor(log10(abs(cyclicBufferSize))) + 1;
    char cyclicBufferSize_str[cyclicBufferSize_digits+1];
    snprintf(cyclicBufferSize_str, sizeof(cyclicBufferSize_str), "%ld", cyclicBufferSize);

    int sizeOfBloom_digits = floor(log10(abs(sizeOfBloom))) + 1;
    char sizeOfBloom_str[sizeOfBloom_digits+1];
    snprintf(sizeOfBloom_str, sizeof(sizeOfBloom_str), "%ld", sizeOfBloom);
    
    //CREATE NUMOFMONITORS MONITORS
    for(int i = 0; i < numMonitors; i++){           
        monitor[i].pid = fork();                                                         
        if(monitor[i].pid < 0){                 //FORK FAILED                                                                                   
            perror("fork failed");
            return 2;
        }
        else if(monitor[i].pid == 0){           //CHILD PROCESS                            
            
            //CONTINUE EXECUTION FROM MONITOR PROGRAM
            exec_return_value = execl(monitor_process, monitor_process, "-p", port_str, "-t", numThreads_str, "-b", socketBufferSize_str, "-c", cyclicBufferSize_str, "-s", sizeOfBloom_str, monitor_paths[i], NULL);

            if(exec_return_value == -1){
                perror("execl ");
                exit(1);
            }
            break;
        }
        int newsocket = 0;
        struct sockaddr_in client;
        socklen_t client_len = sizeof(client);
        newsocket = accept_connection(mainfd, (struct sockaddr*)&client, &client_len);
        readfd[i] = newsocket;
        writefd[i] = newsocket;
        
    }


    //PARENT PROCESS
    count = 0;
    dir_count = 0;
    
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
        
        if (S_ISDIR(st.st_mode)){
            //INSERTION
            insert_country_namesBST(&cNamesBST_root, dent->d_name);
        }

    }
    rewinddir(input_dir);

    count = 0;
    //GET TO KNOW WHICH MONITOR HANDLES WHICH COUNTRY/COUNTRIES
    get_country_monitor(&cNamesBST_root, numMonitors, &count);

    char* virusName = NULL;
    int virusName_len = 0;
    int BF_position = 0;
    int numMonitors_digits = floor(log10(abs(numMonitors))) + 1;
    count = 0;

    struct pollfd* pollarray = malloc(sizeof(struct pollfd)*numMonitors);
    for(int i = 0; i < numMonitors; i++){
        pollarray[i].fd = readfd[i];
        pollarray[i].events = POLLIN;
        pollarray[i].revents = 0;

    }
    int gather_data = 0;

    //READ VIRUSES BLOOMFILTERS DATA FROM MONITOR PROCESSES
    while(gather_data < numMonitors){
        int poll_retval = poll(pollarray, numMonitors, -1);
        if(poll_retval == -1){
            perror("poll: ");
            exit(1);
        }
        for(int i = 0; i < numMonitors; i++){
            if((pollarray[i].revents & POLLIN) != 0){
                int fd = pollarray[i].fd;
                gather_data++;
                while(1){
                    //READ LENGTH OF VIRUS NAME STRING
                    read_from_socket(fd, &virusName_len, sizeof(int), process);
                    if(virusName_len == -1){
                        break;
                    }
                    else{
                        //READ VIRUSNAME
                        virusName = (char*)malloc(virusName_len*sizeof(char) + numMonitors_digits + 1);                  
                        read_from_socket(fd, virusName, virusName_len*sizeof(char), process);

                        //INSERT VIRUS TO PARENT'S VIRUS BST - GET A POINTER TO ITS' NODE IF ALREADY INSERTED
                        pVirus_tree = insert_pVirusBST(&pVirus_root, virusName, sizeOfBloom);

                        //SET BST ACCORDING TO CHILD PROCESSES GIVEN DATA 
                        while(1){

                            read_from_socket(fd, &BF_position, sizeof(int), process);
                            if(BF_position == -1){  //END OF INPUT
                                break;
                            }
                            else{        //SET GIVEN BST POSITION TO 1 IN CORRESPONDING VIRUS'S BLOOMFILTER OF PARENT
                                setBit(pVirus_tree->BF, BF_position);
                            }
                        }
                        free(virusName);
                        virusName = NULL;
                    }
                }
            }
        }

    }
    free(pollarray);

    //INTERACT WITH USER
    char* command = NULL;   //USER COMMAND TO INTERACT WITH DATABASE
    char* operation;        
    char* arguments;
    ssize_t rbytes;
    char* logfile_init = "log_file.";   //LOGFILE PREFIX
    char* logfile_name;
    int parentId_digits;
    FILE* logfile = NULL;
    int total_requests = 0;             //REQUESTS COUNTERS
    int accepted_requests = 0;
    int rejected_requests = 0;
    char exit_msg[5] = "exit";

    //RECEIVE REQUESTS FROM USER
    while(1){

        printf("Please insert command: ");
        rbytes = getline(&command, &len, stdin);
        if(rbytes == -1){
            fprintf(stderr, "can't read line, errno:%d\n", errno);
            exit(EXIT_FAILURE);
        }

        int command_len = strlen(command) + 1;
        char command_cp[command_len];
        strcpy(command_cp, command);
        operation = strtok(command, " ");
        arguments = strtok(NULL, "\n");


        if(strcmp(operation, "/travelRequest") == 0){
            travelRequest(command_cp, arguments, &writefd[0], &readfd[0], sizeOfBloom, K, process);
        }
        else if(strcmp(operation, "/travelStats") == 0){
            travelStats(command_cp, arguments);
        }
        else if(strcmp(operation, "/addVaccinationRecords") == 0){
            addVaccinationRecords(command_cp, arguments, &writefd[0], &readfd[0], &monitor[0], sizeOfBloom, process);
        }
        else if(strcmp(operation, "/searchVaccinationStatus") == 0){
            searchVaccinationStatus(command_cp, arguments, &writefd[0], &readfd[0], numMonitors, process);
        }
        else if(strcmp(operation, "exit\n") != 0){

            printf("Not a proper operation! Operation should be one of the following:\n"
            "/travelRequest\n"
            "/travelStats\n"
            "/addVaccinationRecords\n"
            "/searchVaccinationStatus\n"
            "/exit\n\n");

        }
        if(strcmp(operation, "exit\n") == 0){
            int exit_len = strlen(exit_msg) + 1;

            for(int i = 0; i < numMonitors; i++){
                //SENT EXIT MESSAGE TO ALL THE MONITOR PROCESSES
                write_to_socket(writefd[i], &exit_len, sizeof(int), process);
                write_to_socket(writefd[i], exit_msg, exit_len, process);
            }
            
            //GET PROCESS ID - USE IT FOR LOGFILE NAME
            parent_id = getpid();
            parentId_digits = floor(log10(abs(parent_id))) + 1;                              
            logfile_name = (char*)malloc(strlen(logfile_init)+parentId_digits+1);

            sprintf(logfile_name, "%s%d", logfile_init, parent_id);

            //OPEN LOGFILE
            logfile = fopen(logfile_name, "w");
            //WRITE COUNTRIES TO LOGFILE
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
                
                fprintf(logfile, "%s\n", dent->d_name); 
            }
            //CALL THIS PARENT BST FUNCTION TO RECEIVE ALL REQUESTS STATUS 
            get_requests_status(&pVirus_root, &total_requests, &accepted_requests, &rejected_requests);
            
            //WRITE REQUESTS STATUS TO LOGFILE
            fprintf(logfile, "TOTAL TRAVEL REQUESTS %d\n", total_requests);
            fprintf(logfile, "ACCEPTED %d\n", accepted_requests);
            fprintf(logfile, "REJECTED %d\n", rejected_requests);

            fclose(logfile);
            free(logfile_name);

            free(command);
            command = NULL;
            
            printf("Exiting...\n");
            break;
        }

        
    }

    //SHUTDOWN AND CLOSE ALL SOCKETS PIPES THAT HAVE BEEN CREATED
    for(int i=0; i < numMonitors; i++){
        shutdown(writefd[i], SHUT_RDWR);
        close(writefd[i]);

        shutdown(readfd[i], SHUT_RDWR);
        close(readfd[i]);        
    }
    

    //WAIT CHILD PROCESSES(MONITORS) TO FINISH
    while ((wait_pid = wait(&status)) > 0);

    //FREE ALL THE ALLOCATED MEMORY
    for(int i = 0; i < numMonitors; i++){
        free(monitor_paths[i]);
    }

    closedir(input_dir);
    deallocate_country_namesBST(&cNamesBST_root);
    deallocate_pathBST(&pathBST_root);
    pVirusBST_deallocate(&pVirus_root);

    printf("parent process terminating: process id is %d\n", getpid());
    
    return 0;  
}



