#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>
#include <sys/stat.h>


//This function creates a new process from the specified process image
int spawn(const char ** program) {

  pid_t child_pid = fork();

  if(child_pid < 0) {
    perror("Error while forking...");
    return 1;
  }

  else if(child_pid != 0) {
    return child_pid;
  }

  else {
    if(execvp (program[0], program) == 0);
    perror("Exec failed");
    return 1;
  }
}




int main(){
  //argument list for each processes
  char * arg_list_command[] = { "/usr/bin/konsole", "-e", "./bin/command", NULL };
  char * arg_list_inspection[] = { "/usr/bin/konsole", "-e", "./bin/inspection", NULL };
  char * arg_list_M1[] = {"./bin/M1", NULL };
  char * arg_list_M2[] = {"./bin/M2", NULL };
  char * arg_list_world[] = {"./bin/world", NULL };
  char * arg_list_watchdog[] = {"/usr/bin/konsole", "-e","./bin/watchdog", NULL };

  pid_t pid_cmd = spawn(arg_list_command);
  pid_t pid_insp = spawn(arg_list_inspection);
  pid_t pid_m1 = spawn(arg_list_M1);
  pid_t pid_m2 = spawn(arg_list_M2);	
  pid_t pid_world = spawn(arg_list_world);
  pid_t pid_watchdog = spawn(arg_list_watchdog);

  //wait processes to change their states
  int status;
  waitpid(pid_cmd, &status, 0);
  waitpid(pid_insp, &status, 0);
  waitpid(pid_m1, &status, 0);
  waitpid(pid_m2, &status, 0);
  waitpid(pid_world, &status, 0); 
  waitpid(pid_watchdog, &status, 0); 

  

  printf ("Main program exiting with status %d\n", status);
  return 0;
}


