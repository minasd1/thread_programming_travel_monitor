#include "skipList.h"

//INITIALIZE A SKIPLIST
skipList* initialize_skipList(size_t skiplist_size){
    skipList* sList;
    //ALLOCATE THE PROPER MEMORY FOR SKIPLIST
    sList = (skipList*)malloc(sizeof(skipList));
    sList->num_of_layers = skiplist_size;
    
    //ALLOCATE PROPER MEMORY FOR SKIPLIST MEMBERS TO POINT TO
    int layers = sList->num_of_layers;
    sList->header = (skipListNode*)malloc(sizeof(skipListNode));
    sList->header->forward = (skipListNode**)malloc(sizeof(skipListNode*) * layers);
    sList->final = (skipListNode*)malloc(sizeof(skipListNode));
    sList->final->forward = (skipListNode**)malloc(sizeof(skipListNode*) * layers);
    sList->final->id = (char*)malloc(strlen(MAXVALUE) + 1);
    
    strcpy(sList->final->id, MAXVALUE);
    for(int i = 0; i < layers; i++){
        sList->header->forward[i] = sList->final;
        sList->final->forward[i] = NULL;
    }
    
    return sList;
}

//INITIALIZE A SKIPLIST NODE
skipListNode* initialize_node(char *id, int maxLayers){
    //ALLOCATE PROPER MEMORY
    skipListNode* node = (skipListNode*)malloc(sizeof(skipListNode));
    //ASSIGN GIVEN ID VALUE
    node->id = (char*)malloc(strlen(id) + 1);
    strcpy(node->id, id);

    node->forward = (skipListNode**)malloc(sizeof(skipListNode*) * maxLayers);

    for(int i = 0; i < maxLayers; i++){
        node->forward[i] = NULL;          
        
    }

    hashTableList* hashNode = search_hashList(id);
    //IF GIVEN ID ALREADY EXISTS IN DATABASE (IN CITIZEN HASHTABLE)
    if(hashNode != NULL){
        //POINT TO ITS RECORD TO AVOID DUPLICATES
        node->hRecord = hashNode->record;
    }


    return node;
}

//INITIALIZE A SKIPLIST NODE RECORD
skipListRecord* initialize_nodeRecord(char* isvaccinated, char* dateVaccinated){
    skipListRecord* record;
    //ALLOCATE PROPER MEMORY
    record = (skipListRecord*)malloc(sizeof(skipListRecord));
    record->isvaccinated = (char*)malloc(strlen(isvaccinated) + 1);
    strcpy(record->isvaccinated, isvaccinated);
    //IF IT IS A VACCINATED RECORD
    if(strcmp(isvaccinated, "YES") == 0){
        //THEN ASSIGN THE DATE
        record->dateVaccinated = (char*)malloc(strlen(dateVaccinated) + 1);
        strcpy(record->dateVaccinated, dateVaccinated);
    }
    else{
        record->dateVaccinated = NULL;
    }

    return record;
}

//GET NUMBER OF LAYERS THAT A SKIPLIST NODE WILL HAVE
int get_numofLayers(int maxLayers){
    int numofLayers = 1;
    float random = ((float)rand()/RAND_MAX);
    while((random < 0.5) && (numofLayers < maxLayers)){
        numofLayers++;
        random = ((float)rand()/RAND_MAX);
    }
    return numofLayers;
}

//INSERT A NODE IN SKIPLIST
void insertskipList(skipList** sList, char* id, char* isvaccinated, char* dateVaccinated, int maxLayers){
    skipListRecord* record; 
    skipListNode* node = (*sList)->header; 
    
    skipListNode* update[maxLayers];
    int record_id = atoi(id);
    int node_id;
    
    for(int i = maxLayers-1; i >= 0; i--){
        node_id = atoi(node->forward[i]->id);
        //TRAVERSE SKIPLIST FROM UPPER TO LOWER LAYERS
        while(record_id > node_id){
            node = node->forward[i];

            node_id = atoi(node->forward[i]->id);
        }
        update[i] = node;
    }
    node = node->forward[0];
    node_id = atoi(node->id);

    if(node_id == record_id){
        //printf("Record already exists!\n");       //CHECK THAT NODE ID ALREADY EXISTS
        return;                                   //SO WE DON'T INSERT IT
    }
    else{                                               //INSERT NODE
        int numofLayers = get_numofLayers(maxLayers);   //GET NUMBER OF LAYERS THAT THE NEW NODE WILL HAVE
        //INITIALIZE NODE AND ITS' RECORD, ASSIGN THE RECORD TO NODE'S RECORD MEMBER
        record = initialize_nodeRecord(isvaccinated, dateVaccinated);
        node = initialize_node(id, maxLayers);
        node->record = record;

        for(int i = 0; i < numofLayers; i++){
            node->forward[i] = update[i]->forward[i];
            update[i]->forward[i] = node;
        }

    }
}

//SEARCH SKIPLIST FOR GIVEN KEY VALUE
skipListNode* search_skipList(skipList* sList,char* key){
    skipListNode* node = sList->header;
    int num_of_layers = sList->num_of_layers;
    int key_int = atoi(key);
    int id_int;
    //TRAVERSE SKIPLIST FROM UPPER TO LOWER LAYERS
    for(int i = num_of_layers - 1; i >= 0; i--){
        id_int = atoi(node->forward[i]->id);
        while(key_int > id_int){
            node = node->forward[i];

            id_int = atoi(node->forward[i]->id);
        }
    }
    node = node->forward[0];
    id_int = atoi(node->id);
    if(id_int == key_int){
        return node;
    }
    else{
        return NULL;
    }
}

//FREE A NODE'S ALLOCATED MEMORY
void destroy_skipListNode(skipListNode* node){

    free(node->record->isvaccinated);
    if(node->record->dateVaccinated != NULL){
        free(node->record->dateVaccinated);
    }
    
    free(node->record);
    node->hRecord = NULL;
    free(node->id);
    free(node->forward);
    free(node);
}

//DELETE A SKIPLIST NODE
void delete_skipListNode(skipList* sList, char* key, int maxLayers){
    skipListNode* update[maxLayers];
    skipListNode* node = sList->header;
    int key_int = atoi(key);
    int id_int;
    //TRAVERSE SKIPLIST FROM UPPER TO LOWER LAYERS
    for(int i = maxLayers - 1; i >= 0; i--){
        id_int = atoi(node->forward[i]->id);
        while(key_int > id_int){
            node = node->forward[i];

            id_int = atoi(node->forward[i]->id);
        }
        update[i] = node;
    }
    node = node->forward[0];
    id_int = atoi(node->id);
    //IF WE FOUND THE GIVEN KEY
    if(id_int == key_int){
        for(int i = 0; i < maxLayers; i++){
            if(update[i]->forward[i] != node){
                break;
            }
            update[i]->forward[i] = node->forward[i];
        }
        destroy_skipListNode(node);
    }
}

//DESTROY SKIPLIST
void destroy_skipList(skipList* sList){
    skipListNode* current = sList->header->forward[0];
    skipListNode* throwaway;
    //WE ONLY HAVE TO TRAVERSE SKIPLIST'S FIRST LAYER, TO GET ALL THE NODES
    while(current != sList->final){
        throwaway = current;
        current = current->forward[0];
        destroy_skipListNode(throwaway);
    }
    //FREE ALLOCATED MEMORY
    free(sList->header->forward);
    free(sList->header);
    free(sList->final->id);
    free(sList->final->forward);
    free(sList->final);
    free(sList);
}

//PRINT SKIPLIST
void print_skipList(skipList* sList){
    skipListNode* current;
    current = sList->header->forward[0];
    while(current != sList->final){
        printf("%s %s %s %s %d\n", current->id, current->hRecord->firstName, current->hRecord->lastName, current->hRecord->country, current->hRecord->age);   //NEW country->name
        current = current->forward[0];
    }
}





