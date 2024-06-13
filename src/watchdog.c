#include <stdio.h>
#include <string.h> 
#include <fcntl.h> 
#include <sys/stat.h> 
#include <sys/types.h> 
#include <unistd.h> 
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#define LEN 10

//logfiles of the each process
char * fcmd = "cmd"; 
char * fm1 = "m1";
char * fm2 = "m2";
char * fwrld = "wrld";
char * fins = "ins";

//to reset the hoist position if all of the processes have no action
void sig_handler_reset(int signo){
    //User defined signal 2 is used for reset purpose
    if(signo == SIGUSR2){ 
        //finds process id of the command, m1, and m2 processes respectively    
        char line[LEN];
        FILE *x4 = popen("pidof -s command", "r");        
        fgets(line, LEN, x4);
        if (x4 == NULL){
            printf("Error1!\n");
        }
        else{
            long pid4 = 0;
        pid4 = strtoul(line, NULL, 10);
        //sends the signal to the process
        kill(pid4, SIGUSR2);
        }
  
        FILE *x5 = popen("pidof -s M1", "r");
         if (x5 == NULL){
            printf("Error2!\n");
        }
        else{
            fgets(line, LEN, x5);
        long pid5 = 0;
        pid5 = strtoul(line, NULL, 10);
        //sends the signal to the process
        kill(pid5, SIGUSR2);
        }
        

        FILE *x6 = popen("pidof -s M2", "r");
         if (x6 == NULL){
            printf("Error3!\n");
        }
        else{
            fgets(line, LEN, x6);
        long pid6 = 0;
        pid6 = strtoul(line, NULL, 10);
        //sends the signal to the process
        kill(pid6, SIGUSR2); 
        }
        

    }
    else{
        //if signal number is different then, writes an error message
        printf("\nerror: coming signal is not SIGUSR2!\n");
    }
}

//function to take last modification time of each process
static time_t getFileModifiedTime(const char *path)
{
    struct stat attr;
    if (stat(path, &attr) == 0)
    {
        printf("last modified time of %s: %s", path, ctime(&attr.st_mtime));
        return attr.st_mtime;
    }
    
    return 0;
}


int main(int argc, char const **argv){
    //signal definition
    signal(SIGUSR2, sig_handler_reset);

    while(1){
    //takes last modification time of the each process in 60 seconds
    time_t t1_cmd = getFileModifiedTime(fcmd);
    time_t t1_m1 = getFileModifiedTime(fm1);
    time_t t1_m2 = getFileModifiedTime(fm2);
    time_t t1_wrld = getFileModifiedTime(fwrld);
    time_t t1_ins = getFileModifiedTime(fins);
    sleep(60);
    time_t t2_cmd = getFileModifiedTime(fcmd);
    time_t t2_m1 = getFileModifiedTime(fm1);
    time_t t2_m2 = getFileModifiedTime(fm2);
    time_t t2_wrld = getFileModifiedTime(fwrld);
    time_t t2_ins = getFileModifiedTime(fins);

    //chechks the time and if they are equal this means there is no action and resets the hoist position
    if(t1_cmd == t2_cmd && 
    t1_m1 == t2_m1 && t1_m2 == t2_m2 && t1_wrld == t2_wrld && t1_ins == t2_ins ){
        printf("Reset\n");
        //sending signal to the command, m1 and m2 processes
        sig_handler_reset(SIGUSR2);
    }
       
    }
    return 0;
        
    
}
