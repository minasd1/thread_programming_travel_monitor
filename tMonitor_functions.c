#include "tMonitor_functions.h"

void travelRequest(char* command, char* args, int* writefd, int* readfd, size_t BF_size, int hash_num, char* process){
    char* citizenID = NULL;
    char* date;
    char* countryFrom;
    char* countryTo;
    char* virusName;
    char* isVaccinated = NULL;
    int len;
    int BF_position;
    int numMonitor;
    char* vacdate = NULL;
    bool request_accepted;

    int num_of_spaces = get_num_of_spaces(command);
    
    if(num_of_spaces == 5){
        citizenID = strtok(args, " ");
        //CHECK IF GIVEN ID HAS THE PROPER FORM
        if((is_number(citizenID)) && (strlen(citizenID) <= 5)){
            date = strtok(NULL, " ");
            countryFrom = strtok(NULL, " ");
            
            //CHECK IF GIVEN COUNTRY FROM ARGUMENT EXIST IN DATABASE
            cNames_nodeptr country_names_node = search_cNamesBST(cNamesBST_root, countryFrom);      
            if(country_names_node != NULL){
                countryTo = strtok(NULL, " ");
                if(is_string(countryTo)){
                    virusName = strtok(NULL, "\n");
                    if(virusName != NULL){
                        //GET THE MONITOR PROCESS THAT HANDLES GIVEN COUNTRY
                        numMonitor = country_names_node->monitor;
                        pVirus_nodeptr virus_node = search_pVirusBST(pVirus_root, virusName);
                        //WE HAVE TO CHECK THIS TO PREVENT USING A NULL POINTER LATER
                        if(virus_node != NULL){
                            
                            unsigned short int count = 0;
                            //CHECK THE BLOOMFILTER TO DECIDE WHETHER GIVEN ID IS OF A NOT VACCINATED OR MAYBE
                            //VACCINATED PERSON
                            for(int i = 0; i < hash_num; i++){
                                BF_position = hash_i(citizenID, i)% BF_size;
                                if(testBit(virus_node->BF, BF_position) == false){
                                    //REQUEST REJECTED - CAUGHT BY PARENT BLOOMFILTER
                                    printf("REQUEST REJECTED - YOU ARE NOT VACCINATED\n");
                                    insert_request_list(&(virus_node->request_list), countryTo, date, false);
                                    
                                    break;
                                }
                                else{
                                    count++;
                                } 
                            }
                            //GIVEN RECORD FROM USER IS MAYBE VACCINATED - ASK MONITOR PROCESS    
                            if(count == hash_num){
                                len = strlen(command) + 1;
                                //SEND COMMAND TO CHILD
                                write_to_socket(*(writefd+numMonitor), &len, sizeof(int), process);
                                write_to_socket(*(writefd+numMonitor), command, len, process);

                                //RECEIVE IF CITIZEN IS VACCINATED FROM CHILD PROCESS
                                read_from_socket(*(readfd+numMonitor), &len, sizeof(int), process);
                                isVaccinated = (char*)malloc(len*sizeof(char));
                                read_from_socket(*(readfd+numMonitor), isVaccinated, len, process);

                                //IF CITIZEN IS VACCINATED, GET VACCINATION DATE
                                if(strcmp(isVaccinated, "YES") == 0){
                                    //READ VACCINATION DATE FROM CHILD PROCESS
                                    read_from_socket(*(readfd+numMonitor), &len, sizeof(int), process);
                                    vacdate = (char*)malloc(len*sizeof(char));
                                    read_from_socket(*(readfd+numMonitor), vacdate, len, process);

                                    if(date2_greater_date1(vacdate, date)){
                                        
                                        //IF CITIZEN IS VACCINATED WITHIN SIX MONTHS OF TRAVEL DATE
                                        if(within_six_months(vacdate, date)){
                                            printf("REQUEST ACCEPTED – HAPPY TRAVELS\n");
                                            //INFORM THE CHILD PROCESS THAT REQUEST IS ACCEPTED
                                            request_accepted = true;
                                            write_to_socket(*(writefd+numMonitor), &request_accepted, sizeof(bool), process);

                                            //INSERT GIVEN REQUEST TO VIRUS'S REQUEST LIST
                                            insert_request_list(&(virus_node->request_list), countryTo, date, true);
            
                                        }
                                        else{
                                            printf("REQUEST REJECTED – YOU WILL NEED ANOTHER VACCINATION BEFORE TRAVEL DATE\n");
                                            //INFORM THE CHILD PROCESS THAT REQUEST IS REJECTED
                                            request_accepted = false;
                                            write_to_socket(*(writefd+numMonitor), &request_accepted, sizeof(bool), process);
                                            
                                            //INSERT GIVEN REQUEST TO VIRUS'S REQUEST LIST
                                            insert_request_list(&(virus_node->request_list), countryTo, date, false);
                            
                                        }
                                    }
                                    else{           //ERROR INPUT MANIPULATION
                                        printf("Error: Travel date must be after vaccination date!\n");
                                    }
                                }
                                else{
                                    printf("REQUEST REJECTED - YOU ARE NOT VACCINATED\n");
                                }
                            }
                        }
                        else{
                            printf("There is no such virus record in given country!\n");
                        }
                    }
                    else{
                        printf("Error: Command must be like following:\n"
                        "/travelRequest citizenID date countryFrom countryTo virusName\n");
                    }
                }
                else{
                    printf("Error: countryTo argument must be a string!\n");
                }
            }
            else{
                printf("Error: Wrong country input!\n");
            }
            
        }
        else{
            printf("Error: Given id should be a number with 5 digits maximum!\n");
        }
    }
    else{
        printf("Error: Command must be like following:\n"
        "/travelRequest citizenID date countryFrom countryTo virusName\n");
    }
    //if(countryFrom_cp != NULL){free(countryFrom_cp);}
    if(isVaccinated != NULL){free(isVaccinated);}
    if(vacdate != NULL){free(vacdate);}
    
}


void travelStats(char* command, char* args){
    char* virusName;
    char* date1;
    char* date2;
    char* country;
    int total_requests = 0;
    int accepted_requests = 0;
    int rejected_requests = 0;

    int num_of_spaces = get_num_of_spaces(command);

    if(num_of_spaces == 3 || num_of_spaces == 4){
        virusName = strtok(args, " ");
        pVirus_nodeptr virus_node = search_pVirusBST(pVirus_root, virusName);
        if(virus_node != NULL){                 //CHECK FOR A PROPER ARGUMENTS INPUT
            date1 = strtok(NULL, " ");
            if(num_of_spaces == 3){                         // /travelStats virusName date1 date2
                date2 = strtok(NULL, "\n");
                if(date2 != NULL){
                    if(valid_date(date1) && valid_date(date2)){ //CHECK THAT DATES HAVE VALID FORM
                        if(date2_greater_date1(date1, date2)){  //CHECK IF DATE2 IS GREATER THAN DATE1
                            listNode* current = virus_node->request_list->head;
                                
                            //TRAVERSE VIRUS'S REQUEST LIST
                            while(current != NULL){
                                //CHECK IF CURRENT REQUEST DATE IS BETWEEN GIVEN DATES
                                if(date2_greater_date1(date1, current->record->date) && date2_greater_date1(current->record->date, date2)){
                                    if(current->record->accepted == true){
                                        accepted_requests++;
                                    }
                                    else{
                                        rejected_requests++;
                                    }
                                    total_requests++;
                                }
                                current = current->next;
                            }
                            printf("TOTAL REQUESTS %d\n", total_requests);
                            printf("ACCEPTED %d\n", accepted_requests);
                            printf("REJECTED %d\n", rejected_requests);
                        }
                        else{       //ERROR INPUT MANIPULATION
                            printf("Error: Date2 must be greater than date1!\n");
                        }
                    }
                    else{
                        printf("Invalid date!\n");
                    }
                    
                }
                else{
                    printf("Error: Command must be like following:\n"
                    "/travelStats virusName date1 date2\n");
                }
            }
            else if(num_of_spaces == 4){       // /travelStats virusName date1 date2 country
                date2 = strtok(NULL, " ");
                if(date2_greater_date1(date1, date2)){  //CHECK IF DATE2 IS GREATER THAN DATE1
                    country = strtok(NULL, "\n");
                    if(country != NULL){
                        listNode* current = virus_node->request_list->head;
        
                        //TRAVERSE VIRUS'S REQUEST LIST
                        while(current != NULL){
                            //CHECK IF CURRENT REQUEST CONCERNS GIVEN COUNTRY
                            if(strcmp(current->record->country, country) == 0){
                                //CHECK IF REQUEST DATE IS BETWEEN GIVEN DATES
                                if(date2_greater_date1(date1, current->record->date) && date2_greater_date1(current->record->date, date2)){
                                    if(current->record->accepted == true){
                                        accepted_requests++;
                                    }
                                    else{
                                        rejected_requests++;
                                    }
                                    total_requests++;
                                }
                                
                            }
                            current = current->next;
                        }
                        printf("%s\n", country);
                        printf("TOTAL REQUESTS %d\n", total_requests);
                        printf("ACCEPTED %d\n", accepted_requests);
                        printf("REJECTED %d\n", rejected_requests);
                    }
                    else{       //ERROR INPUT MANIPULATION
                        printf("Error: Command must be like following:\n"
                        "/travelStats virusName date1 date2 country\n");
                    }
                }
                else{
                    printf("Error: Date2 must be greater than date1!\n");
                }
            }
            else{
                printf("Error: Command must be like following:\n"
                "/travelStats virusName date1 date2 [country]\n");
            }
            

        }
        else{
            printf("Given virus does not exist in database!\n");
        }
        
    }
    else{
        printf("Error: Command must be like following:\n"
        "/travelStats virusName date1 date2 [country]\n");
    }
}

void searchVaccinationStatus(char* command, char* args, int* writefd, int* readfd, int numMonitors, char* process){
    char* citizenID;
    char* firstName;
    char* lastName;
    char* country;
    int age;
    char* virusName;
    char* isVaccinated;
    char* dateVaccinated;
    int len;
    bool found;

    struct pollfd* pollarray = malloc(sizeof(struct pollfd)*numMonitors);
    for(int i = 0; i < numMonitors; i++){
        pollarray[i].fd = *(readfd+i);
        pollarray[i].events = POLLIN;
        pollarray[i].revents = 0;
    }

    int num_of_spaces = get_num_of_spaces(command);
    
    if(num_of_spaces == 1){
        citizenID = strtok(args, "\n");
        if(citizenID != NULL){
            len = strlen(command) + 1;
            
            for(int i = 0; i < numMonitors; i++){
                //SEND COMMAND TO CHILD PROCESSES
                write_to_socket(*(writefd+i), &len, sizeof(int), process);
                write_to_socket(*(writefd+i), command, len, process);

            }
            int gather_data = 0;
            
            while(gather_data < numMonitors){
                int poll_retval = poll(pollarray, numMonitors, -1);
                if(poll_retval == -1){
                    perror("poll: ");
                    exit(1);
                }

                for(int i = 0; i < numMonitors; i++){
                    if((pollarray[i].revents & POLLIN) != 0){
                        int fd = pollarray[i].fd;
                        read_from_socket(fd, &found, sizeof(bool), process);
                        gather_data++;
                        //IF GIVEN CITIZEN ID IS FOUND IN A MONITOR PROCESS RECORDS
                        if(found == true){
                            //READ CITIZEN'S INFORMATION BY THE MONITOR
                            read_from_socket(fd, &len, sizeof(int), process);
                            firstName = (char*)malloc(len*sizeof(char));
                            read_from_socket(fd, firstName, len, process);

                            read_from_socket(fd, &len, sizeof(int), process);
                            lastName = (char*)malloc(len*sizeof(char));
                            read_from_socket(fd, lastName, len, process);

                            read_from_socket(fd, &len, sizeof(int), process);
                            country = (char*)malloc(len*sizeof(char));
                            read_from_socket(fd, country, len, process);

                            read_from_socket(fd, &age, sizeof(int), process);
                            printf("%s %s %s %s\n", citizenID, firstName, lastName, country);
                            printf("AGE %d\n", age);

                            free(firstName);
                            firstName = NULL;
                            free(lastName);
                            lastName = NULL;
                            free(country);
                            country = NULL;

                            //READ VACCINATION STATUS BY THE MONITOR FOR EVERY VIRUS THAT CITIZENID HAS A RECORD
                            while(1){
                                read_from_socket(fd, &len, sizeof(int), process);
                                if(len == -1){
                                    break;
                                }
                                else{
                                    virusName = (char*)malloc(len*sizeof(char));
                                    read_from_socket(fd, virusName, len, process);

                                    read_from_socket(fd, &len, sizeof(int), process);
                                    isVaccinated = (char*)malloc(len*sizeof(char));
                                    read_from_socket(fd, isVaccinated, len, process);

                                    //IF CITIZEN IS VACCINATED
                                    if(strcmp(isVaccinated, "YES") == 0){
                                        read_from_socket(fd, &len, sizeof(int), process);
                                        dateVaccinated = (char*)malloc(len*sizeof(char));
                                        read_from_socket(fd, dateVaccinated, len, process);
                                        printf("%s VACCINATED ON %s\n", virusName, dateVaccinated);
                                        free(dateVaccinated);
                                        dateVaccinated = NULL;
                                    }
                                    else{
                                        printf("%s NOT YET VACCINATED\n", virusName);
                                    }

                                    free(virusName);
                                    virusName = NULL;
                                    free(isVaccinated);
                                    isVaccinated = NULL;
                                }
                            }
                        }
                    }
                }

            }
    
        }
        else{       //ERROR INPUT MANIPULATION
            printf("Error: Command must be like following:\n"
            "searchVaccinationStatus citizenID\n");
        }
    }
    else{
        printf("Error: Command must be like following:\n"
        "searchVaccinationStatus citizenID\n");
    }
    free(pollarray);

}


void addVaccinationRecords(char* command, char* args, int* writefd, int* readfd, struct childProcess* monitor, size_t BF_size, char* process){
    char* country;
    int numMonitor;
    
    char* virusName;
    int virusName_len = 0;
    int BF_position = 0;
    int len;
    
    int num_of_spaces = get_num_of_spaces(command);
    
    if(num_of_spaces == 1){
        country = strtok(args, "\n");
        
        if(country != NULL){
            if(is_string(country)){                         
                //strncat(country, &ch, sizeof(char));
                //SEARCH IF GIVEN COUNTRY ARGUMENT EXISTS IN DATABASE
                cNames_nodeptr country_node = search_cNamesBST(cNamesBST_root, country);
                if(country_node != NULL){
                    //GET MONITOR PROCESS THAT HANDLES THE COUNTRY
                    numMonitor = country_node->monitor;
                    
                    //SEND COMMAND TO THAT MONITOR
                    len = strlen(command);
                    write_to_socket(*(writefd+numMonitor), &len, sizeof(int), process);
                    write_to_socket(*(writefd+numMonitor), command, len, process);
                    
                    while(1){
                        //READ LENGTH OF VIRUS NAME STRING
                        read_from_socket(*(readfd+numMonitor), &virusName_len, sizeof(int), process);
                        if(virusName_len == -1){
                            break;
                        }
                        else{
                            //READ VIRUSNAME
                            virusName = (char*)malloc(virusName_len*sizeof(char));
                            read_from_socket(*(readfd+numMonitor), virusName, virusName_len*sizeof(char), process);

                            //INSERT VIRUS TO PARENT'S VIRUS BST - GET A POINTER TO ITS' NODE IF ALREADY INSERTED
                            pVirus_tree = insert_pVirusBST(&pVirus_root, virusName, BF_size);

                            //SET VIRUS'S BLOOMFILTER ACCORDING TO CHILD PROCESS GIVEN DATA 
                            while(1){
                                //RECEIVE THE POSITIONS OF BLOOMFILTER THAT ARE SET TO 1 BY THE MONITOR BLOOMFILTER
                                read_from_socket(*(readfd+numMonitor), &BF_position, sizeof(int), process);
                                if(BF_position == -1){  //END OF INPUT
                                    break;
                                }
                                else{                   
                                    //END SET THE SAME POSITIONS OF PARENT VIRUS'S BLOOMFILTER TO 1
                                    setBit(pVirus_tree->BF, BF_position);
                                }
                            }
                            free(virusName);
                            virusName = NULL;
                        }
                    }
                }
                else{       //ERROR INPUT MANIPULATION
                    printf("Error: Given country does not exist in database!\n");
                }
            }
            else{
                printf("Error: Country argument must be a string\n");
            }
        }
        else{
            printf("Error: Command must be like following:\n"
            "addVaccinationRecords country\n");
        }
    }
    else{
        printf("Error: Command must be like following:\n"
        "addVaccinationRecords country\n");
    }
}