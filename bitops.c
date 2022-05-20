#include "bitops.h"

void setBit(int* A, int k){
    int i = k/32;           //FIND INDEX OF ARRAY THAT K CORRESPONDS TO
    int position = k%32;    //FIND SPECIFIC POSITION 

    unsigned int flag = 1;
    flag = flag << position;

    A[i] = A[i] | flag;     //SET BIT TO 1
}

bool testBit(int *A, int k){
    int i = k/32;           //FIND INDEX OF ARRAY THAT K CORRESPONDS TO    
    int position = k%32;    //FIND SPECIFIC POSITION 

    unsigned int flag = 1;
    flag = flag << position;

    if((A[i] & flag) != 0){ //CHECK IF BIT IS SET TO 1
        return true;
    }
    else{
        return false;
    }
}