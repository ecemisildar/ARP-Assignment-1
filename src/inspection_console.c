#include "./../include/inspection_utilities.h"
#include <stdio.h>
#include <fcntl.h> 
#include <sys/stat.h> 
#include <sys/types.h> 
#include <signal.h>
#include <string.h>
#include <time.h>
#define LEN 10

//Inspection process takes the real distance of x and z, 
//then moves the Hoist to the desired points

//logfile for this process
char *fins = "ins";
//end effector x and z
float ee_x = 0;
float ee_z = 0; 

int i = 0;

//writes the current time, real x and z distance into the logfile
//inputs: real x, and z distance, message type (normal, reset signal or stop signal)
//outputs: function type is void, no output
void write_value (float ee_x, float ee_z, int i) {

    char dt[20]; // space enough for YYYY-MM-DD HH:MM:SS and terminator
    struct tm tm;
    time_t current_time;
    current_time = time(NULL);
    tm = *localtime(&current_time); // convert time_t to struct tm
    strftime(dt, sizeof dt, "%Y-%m-%d %H:%M:%S", &tm); // format

    //Write to log file
    fins = fopen ("ins", "a");
    //prints time and ee coordinates
    if(i == 0) fprintf (fins, "time: %s end-effector x: %.2f, end effector z: %.2f\n", dt, ee_x, ee_z);
    if(i == 1) fprintf (fins, "time: %s NOT STOPPED\n", dt); //indicates error 
    if(i == 2) fprintf (fins, "time: %s NOT RESETED\n", dt); //indicates error 
    fclose (fins); //closes logfile

}

//in order to close all processes with crtl+c
void sig_handler(int signo)
{
    if (signo == SIGINT){
        printf("received SIGINT\n");
        exit(EXIT_SUCCESS);
    }
}

//to stop the hoist
void sig_handler_stop(int signo){
    if(signo == SIGUSR1){     

        char line[LEN];
        FILE *x1 = popen("pidof -s command", "r");        
        fgets(line, LEN, x1);
        if (x1 == NULL){
            printf("Error1!\n");
        }
        else{
            long pid1 = 0;
        pid1 = strtoul(line, NULL, 10);
        //sends the signal to the process
        kill(pid1, SIGUSR1);
        }
  
        FILE *x2 = popen("pidof -s M1", "r");
         if (x2 == NULL){
            printf("Error2!\n");
        }
        else{
            fgets(line, LEN, x2);
        long pid2 = 0;
        pid2 = strtoul(line, NULL, 10);
        //sends the signal to the process
        kill(pid2, SIGUSR1);
        }
        

        FILE *x3 = popen("pidof -s M2", "r");
         if (x3 == NULL){
            printf("Error3!\n");
        }
        else{
            fgets(line, LEN, x3);
        long pid3 = 0;
        pid3 = strtoul(line, NULL, 10);
        //sends the signal to the process
        kill(pid3, SIGUSR1); 
        }
        

    }
    else{
        write_value(ee_x, ee_z, 1);//error message
    }
        
}
//to reset the hoist position  
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
        write_value(ee_x, ee_z, 2);//error message
    }

}


int main(int argc, char const *argv[]){

    char * myfifo_real = "/tmp/myfifo_real";//fifo for real x, z distance
    int fd_real;
    float real_dist = 0;
    //signal definitions
    signal(SIGUSR1, sig_handler_stop);
    signal(SIGUSR2, sig_handler_reset);

    // Utility variable to avoid trigger resize event on launch
    int first_resize = TRUE;

    // Initialize User Interface 
    init_console_ui();

    //error message for sigint
    if (signal(SIGINT, sig_handler) == SIG_ERR)
        printf("\ncan't catch SIGINT\n");
   

    // Infinite loop
    while(TRUE){	
        //opens world fd
        fd_real = open(myfifo_real,O_RDWR);
        //error if it does not open
        if (fd_real== 0) {
            perror("Cannot open fifo_real");
            unlink(myfifo_real);
            exit(1);
        }

        // Get mouse/resize commands in non-blocking mode...
        int cmd = getch();

        // If user resizes screen, re-draw UI
        if(cmd == KEY_RESIZE) {
            if(first_resize) {
                first_resize = FALSE;
            }
            else {
                reset_console_ui();
            }
        }
        
        // Else if mouse has been pressed
        else if(cmd == KEY_MOUSE) {

            // Check which button has been pressed...
            if(getmouse(&event) == OK) {
                // STOP button pressed
                if(check_button_pressed(stp_button, &event)) {
                	//communicate both processes and inform that it is pressed to the stop button
                    mvprintw(LINES - 1, 1, "STP button pressed");  
                    //stop signal sent
                    sig_handler_stop(SIGUSR1);
                    refresh();
                    sleep(1);
                    for(int j = 0; j < COLS; j++) {
                        mvaddch(LINES - 1, j, ' ');
                    }

                }
                             

                // RESET button pressed
                else if(check_button_pressed(rst_button, &event)) {
                    
                    mvprintw(LINES - 1, 1, "RST button pressed");
                    //reset signal sent
                    sig_handler_reset(SIGUSR2);
                    refresh();
                    sleep(1);
                    for(int j = 0; j < COLS; j++) {
                        mvaddch(LINES - 1, j, ' ');
                    }
                    
                    
                }
            }
        }

        char line[1024];
        int bytes_read;
        //reads world process
        bytes_read = read(fd_real, line, sizeof(line));
        //if it has x with the number
        //it is assigned to ee_x
        if(strncmp(line,"x:",2) == 0) {
            float x_prime;
            sscanf(line, "x: %f\n", &x_prime);
            ee_x = x_prime;
        }
        else if(strncmp(line,"z:",2) == 0) {
            float z_prime;
            sscanf(line, "z: %f\n", &z_prime);
            ee_z = z_prime;
        }
        //write the fnal values into the logfile of the inspector
        write_value(ee_x, ee_z, 0);
        //Update UI
        update_console_ui(&ee_x, &ee_z);
        
	}

    // Terminate
    endwin();
    return 0;
}
