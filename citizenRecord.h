#ifndef CITIZENRECORD_H
#define CITIZENRECORD_H

//HOLD A CITIZEN RECORD STRUCT TO READ THE DATA GIVEN FROM FILE
typedef struct citizenRecord{
    char* citizenID;
    char* firstName;
    char* lastName;
    char* country;
    int age;
    char* virusName;
    char* isvaccinated;
    char* dateVaccinated;
}citizenRecord;

#endif