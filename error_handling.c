#include "error_handling.h"


//GET NUMBER OF SPACES OF COMMAND GIVEN BY USER, SO AS TO KNOW ITS' SPECIFIC ARGUMENT USE
//ALSO DECIDE IF GIVEN COMMAND FROM USER IS VALID OR NOT
int get_num_of_spaces(char *str){
    int num_of_spaces = 0;
    char ch;

    for(int i = 0; i < strlen(str); i++){
        ch = str[i];
        if(ch == ' '){
            num_of_spaces++;
        }
    }
    return num_of_spaces;
}

//CHECK IF GIVEN ARGUMENT IS A NUMBER
bool is_number(char* str){
    for(int i = 0; i < strlen(str); i++){
        if(str[i] < '0' || str[i] > '9'){
            return false;
        }
    }
    return true;
}

//CHECK IF GIVEN ARGUMENT IS A STRING
bool is_string(char *str){
    for(int i = 0; i < strlen(str); i++){
        if(isalpha(str[i]) == 0){
            return false;
        }
    }
    return true;
}

//CHECK IF A GIVEN VIRUSNAME HAS VALID FORM
//A VIRUSNAME MUST BE CONSISTED OF LETTERS, NUMBERS AND MAYBE A '-' CHARACTER
bool virus_valid_name(char* str){
    bool flag = true;
    for(int i = 0; i < strlen(str) && flag == true; i++){
        if((isalpha(str[i])) || ((str[i] >= '0') && (str[i] <= '9')) || (str[i] == '-')){
            flag = true;
        }
        else{
            flag = false;
        }
    }
    return flag;

}

//CHECK IF A GIVEN DATE ARGUMENT HAS A VALID FORM
//A VALID DATE HASH THE FORM dd-mm-yyyy
bool valid_date(char* date){
    
    char* date_cp;
    date_cp = strdup(date);
    //DATE MUST HAVE A LENGTH OF 10 CHARACTERS
    if(strlen(date_cp) == 10){
        for(int i = 0; i < strlen(date_cp); i++){
            if(i == 2 || i == 5)
            {
                if(date[i] != '-'){
                    free(date_cp);
                    return false;
                }
            }
            else{
                if(date[i] == '-'){
                    free(date_cp);
                    return false;
                }
            }
            
        }
        //TOKENIZE THE DATE
        char* day = strtok(date_cp, "-");
        char* month = strtok(NULL, "-");
        char* year = strtok(NULL, "-");


        if(is_number(day) && is_number(month) && is_number(year)){
            unsigned short int day_i, month_i, year_i;
            day_i = atoi(day);
            month_i = atoi(month);
            year_i = atoi(year);
            if(day_i < 01 || day_i > 30 || month_i < 01 || month_i > 12 || year_i < 1990 || year_i > 2050){
                free(date_cp);
                return false;
            }
            
        }
    }
    else{
        free(date_cp);
        return false;
    }
    free(date_cp);
    return true;
    
}

//USE IT WHEN USER GIVES 2 DATE ARGUMENTS, WE MUST KNOW THAT SECOND DATE ARGUMENT IS GREATER THAN THE FIRST
bool date2_greater_date1(char* date1, char* date2){
    struct tm tm = {0};

    char* date1_cp = (char*)malloc(strlen(date1) + 1);
    strcpy(date1_cp, date1);
    
    strptime(date1_cp, "%d-%m-%Y", &tm);
    time_t t1 = mktime(&tm);
    free(date1_cp);


    char *date2_cp = (char*)malloc(strlen(date2) + 1);
    strcpy(date2_cp, date2);
    
    strptime(date2_cp, "%d-%m-%Y", &tm);
    time_t t2 = mktime(&tm);
    free(date2_cp);
    
    
    double seconds = difftime(t1, t2);
    
    if (seconds > 0) {
        return false;
    }
    
    return true;
}

//CHECK IF DIFFERENCE BETWEEN GIVEN DATES IS WITHIN SIX MONTHS
bool within_six_months(char* date1, char* date2){
    char* date1_cp;
    char* date2_cp;
    date1_cp = strdup(date1);
    date2_cp = strdup(date2);
    
    //TOKENIZE THE DATE
    char* day1 = strtok(date1_cp, "-");
    char* month1 = strtok(NULL, "-");
    char* year1 = strtok(NULL, "-");

    char* day2 = strtok(date2_cp, "-");
    char* month2 = strtok(NULL, "-");
    char* year2 = strtok(NULL, "-");


    if(is_number(day1) && is_number(month1) && is_number(year1)){
        unsigned short int day_i_1, month_i_1, year_i_1;
        unsigned short int day_i_2, month_i_2, year_i_2;
        day_i_1 = atoi(day1);
        month_i_1 = atoi(month1);
        year_i_1 = atoi(year1);

        day_i_2 = atoi(day2);
        month_i_2 = atoi(month2);
        year_i_2 = atoi(year2);
            
        if(((year_i_2 > year_i_1) && ((12-month_i_1+month_i_2) > 6)) || ((year_i_2 > year_i_1) && ((12-month_i_1+month_i_2) == 6) && (day_i_2 >= day_i_1)) || ((year_i_2 == year_i_1) && ((month_i_2 - month_i_1) > 6)) || ((year_i_2 == year_i_1) && ((month_i_2 - month_i_1) == 6) && (day_i_2 >= day_i_1))){
            
            free(date1_cp);
            free(date2_cp);
            return false;
        }
        
        free(date1_cp);
        free(date2_cp);
        return true;
        
            
    }
    else{
        printf("Within_six_months: Invalid function arguments!\n");
        exit(1);
    }
}