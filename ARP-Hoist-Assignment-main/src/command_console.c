#include "./../include/command_utilities.h"
#include <stdio.h>
#include <string.h> 
#include <fcntl.h> 
#include <sys/stat.h> 
#include <sys/types.h> 
#include <unistd.h> 
#include <stdlib.h>
#include <signal.h>
#include <time.h>
//Command console process takes the inputs from the buttons/user and updates the velocities for x and z directions

float x_vel = 0;
float z_vel = 0;
int i = 0; 
//file descriptors of the x and z velocities
int fd_vx, fd_vz;
//location of the file descriptors
char * myfifo_vx = "/tmp/myfifo_vx"; 
char * myfifo_vz = "/tmp/myfifo_vz"; 
//logfile of command 
char * fcmd = "cmd";

//writes the current time, x and z velocities into the logfiles
//inputs: x velocity, z velocity, message type (normal, reset signal or stop signal)
//outputs: function type is void, no output
void write_value (float x_vel, float z_vel, int i) {
    //takes the local time of the computer
    char dt[20]; // space enough for YYYY-MM-DD HH:MM:SS and terminator
    struct tm tm;
    time_t current_time;
    current_time = time(NULL);
    tm = *localtime(&current_time); // convert time_t to struct tm
    strftime(dt, sizeof dt, "%Y-%m-%d %H:%M:%S", &tm); // format

    //Write to log file
    fcmd = fopen ("cmd", "a"); //opens logfile of the command 
    if(i == 0) fprintf(fcmd, "time: %s x vel: %.2f, z_vel: %.2f\n", dt, x_vel, z_vel); //prints x and z velocity with time
    else if(i == 1) fprintf(fcmd, "time: %s RESET SIGNAL HAS ARRIVED x vel: %.2f, z vel: %.2f\n", dt, x_vel, z_vel); //indicates that reset signal has arrived
    else if(i == 2) fprintf(fcmd, "time: %s STOP SIGNAL HAS ARRIVED x vel: %.2f, z vel: %.2f\n", dt, x_vel, z_vel); ////indicates that stop signal has arrived
    fclose (fcmd); //closes logfile

}

//when stop signal arrives, it makes zero to both velocities 
//and prints them to the logfile with the message: "STOP SIGNAL HAS ARRIVED" 
void sig_handler_stop(int signo){
    //checks the signal number
    if(signo == SIGUSR1){ 
        x_vel = 0;
        z_vel = 0;
        //writes the new values to the logfile 
        write_value(x_vel, z_vel,2);
    }   
}
//when reset signal arrives, it makes zero to both velocities 
//and prints them to the logfile with the message: "RESET SIGNAL HAS ARRIVED" 
void sig_handler_reset(int signo){
    //checks the signal number
    if(signo == SIGUSR2){
        //makes zero to both velocities
        x_vel = 0;
        z_vel = 0;
        //writes the new velocities to the fd in order to send them to next processes
        write(fd_vx, &x_vel, sizeof(float));
        write(fd_vz, &z_vel, sizeof(float));
        close(fd_vz);
        close(fd_vx);
        //writes the new values to the logfile                
        write_value(x_vel, z_vel, 1);         
        }
         
}

int main(int argc, char const *argv[]){	
    //makes the file descriptors
    mkfifo(myfifo_vx, 0666);
    mkfifo(myfifo_vz, 0666); 	
    // Utility variable to avoid trigger resize event on launch
    int first_resize = TRUE;
    // Initialize User Interface 
    init_console_ui();
    //this counter is for counting the pushing numbers of the buttons
    int counter[4] = {0,0,0,0};

    //signal checks for both signals
    if (signal(SIGUSR1, sig_handler_stop) == SIG_ERR){
        fcmd = fopen ("cmd", "a");
        fprintf(fcmd, "\ncan't catch stop signal\n");
        fclose (fcmd);
    }       
    if (signal(SIGUSR2, sig_handler_reset) == SIG_ERR){
        fcmd = fopen ("cmd", "a");
        fprintf(fcmd, "\ncan't catch reset signal\n");
        fclose (fcmd);
    }




    // Infinite loop
    while(TRUE)
	{	
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

                // Vx-- button pressed
                if(check_button_pressed(vx_decr_btn, &event)) { 
                    counter[1] = 0; //Vx++
                    counter[2] = 0; //Vz--  
                    counter[3] = 0; //Vz++            	
                    counter[0]++;   //Vx-- (it counts how many times it is pressed to Vx--)
                    //it is forbidden to push a button more than 3 times
                    if(counter[0] < 3){
                        //decreases the velocity
                        x_vel -= 20;
                        mvprintw(LINES - 1, 1, "Horizontal Speed Decreased");
                        refresh();
                        sleep(1);
                        for(int j = 0; j < COLS; j++) {
                            mvaddch(LINES - 1, j, ' ');
                        }
                        //writes the new value of the velocities to the logfile of the command console
                        write_value (x_vel, z_vel, 0);                
                    }
                    
                                                
                }

                // Vx++ button pressed
                else if(check_button_pressed(vx_incr_btn, &event)) {
                    counter[0] = 0;
                    counter[2] = 0;  
                    counter[3] = 0;             	
                    counter[1]++; //counts Vx++
                    
                    if(counter[1]< 3){
                        x_vel += 20;
                        mvprintw(LINES - 1, 1, "Horizontal Speed Increased");
                        refresh();
                        sleep(1);
                        for(int j = 0; j < COLS; j++) {
                            mvaddch(LINES - 1, j, ' ');
                        }
                        write_value (x_vel, z_vel, 0); 
                    }
                    
                }

                // Vx stop button pressed
                else if(check_button_pressed(vx_stp_button, &event)) {
                    //no counting because pushing stop button infinite times changes nothing
                    counter[0] = 0;
                    counter[1] = 0;  
                    counter[2] = 0;
                    counter[3] = 0;
                    x_vel = 0;                	
                    mvprintw(LINES - 1, 1, "Horizontal Motor Stopped");
                    refresh();
                        sleep(1);
                        for(int j = 0; j < COLS; j++) {
                            mvaddch(LINES - 1, j, ' ');
                        }
                        write_value (x_vel, z_vel, 0); 
                }

                // Vz-- button pressed
                else if(check_button_pressed(vz_decr_btn, &event)) {
                    counter[0] = 0;
                    counter[1] = 0;  
                    counter[3] = 0;                                    	
                    counter[2]++; //counts Vz--

                    if(counter[2] < 3){
                        z_vel -= 20;
                        mvprintw(LINES - 1, 1, "Vertical Speed Decreased");
                        refresh();
                        sleep(1);
                        for(int j = 0; j < COLS; j++) {
                            mvaddch(LINES - 1, j, ' ');
                        }
                        write_value (x_vel, z_vel, 0); 
                    }
                    
                }

                // Vz++ button pressed
                else if(check_button_pressed(vz_incr_btn, &event)) {
                    counter[0] = 0;
                    counter[1] = 0;  
                    counter[2] = 0;                                   	
                    counter[3]++; //counts Vz++
                    
                    if(counter[3] < 3){
                        z_vel += 20;
                        mvprintw(LINES - 1, 1, "Vertical Speed Increased");
                        refresh();
                        sleep(1);
                        for(int j = 0; j < COLS; j++) {
                            mvaddch(LINES - 1, j, ' ');
                        }
                        write_value (x_vel, z_vel, 0); 
                    }                    
                }

                // Vz stop button pressed
                else if(check_button_pressed(vz_stp_button, &event)) {
                    counter[0] = 0;
                    counter[1] = 0;  
                    counter[2] = 0;
                    counter[3] = 0;
                    z_vel = 0;
                    mvprintw(LINES - 1, 1, "Vertical Motor Stopped");
                    refresh();
                    sleep(1);
                    for(int j = 0; j < COLS; j++) {
                        mvaddch(LINES - 1, j, ' ');
                    }
                    write_value (x_vel, z_vel, 0); 
                }
            }
            
        }
        //at the end of the statement
        //fd of x vel and z vel opens and writes their new values to their file descriptors
        fd_vx = open(myfifo_vx, O_RDWR);
        //error message
        if (fd_vx== 0) {
            perror("Cannot open fifo_vx");
            unlink(myfifo_vx);
            exit(1);
        }
        write(fd_vx, &x_vel, sizeof(float));
        close(fd_vx);
        
        fd_vz = open(myfifo_vz, O_RDWR);
        //error message
         if (fd_vz == 0) {
            perror("Cannot open fifo_vz");
            unlink(myfifo_vz);
            exit(1);
        }
        write(fd_vz, &z_vel, sizeof(float));
        close(fd_vz);
        
        sleep(1);
        refresh();
        
	}
    // Terminate
    endwin();
    return 0;
}
