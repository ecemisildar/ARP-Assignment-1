#include <stdio.h>
#include <string.h> 
#include <fcntl.h> 
#include <sys/stat.h> 
#include <sys/types.h> 
#include <unistd.h> 
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#define Xmax 40
#define Xmin 0
//M1 process calculates the distance in the x direction

//file descriptors
int fd_vx, fd_dx;
//x distance and velocity
float x_dist;
float x_vel; 
//time constant
float t = 0.035;
//logfile for this process
char * fm1 = "m1";
//location of the file descriptors
char * myfifo_vx = "/tmp/myfifo_vx";//fifo for x velocity
char * myfifo_dx = "/tmp/myfifo_dx";//fifo for x distance 

int i = 0;

//writes the current time, x distance into the logfile
//inputs: x distance, message type (normal, reset signal or stop signal)
//outputs: function type is void, no output
void write_value (float x, int i) {
    //takes the local time of the computer
    char dt[20]; // space enough for YYYY-MM-DD HH:MM:SS and terminator
    struct tm tm;
    time_t current_time;
    current_time = time(NULL);
    tm = *localtime(&current_time); // convert time_t to struct tm
    strftime(dt, sizeof dt, "%Y-%m-%d %H:%M:%S", &tm); // format

    //Write to log file
    fm1 = fopen ("m1", "a"); //opens logfile of the m1 
    if(i == 0) fprintf(fm1, "time: %s x dist: %.2f\n", dt, x); //prints x distance with time
    else if(i == 1) fprintf(fm1, "time: %s STOP SIGNAL  x dist: %.2f\n", dt, x); //indicates that stop signal has arrived
    else if(i == 2) fprintf(fm1, "time: %s RESET SIGNAL x dist: %.2f\n", dt, x); //indicates that reset signal has arrived
    else if(i == 3) fprintf(fm1, "time: %s MAX DISTANCE IS ACHIEVED\n", dt); //indicates max distance
    fclose (fm1); //closes logfile

}

//when stop signal arrives, it makes zero to x velocity and calculates the new distance
//which is as the same as before 
//and prints them to the logfile with the message: "STOP SIGNAL HAS ARRIVED" 
void sig_handler_stop(int signo){

    if(signo == SIGUSR1){
        x_vel = 0;
        x_dist += x_vel*t;
        //prints a message to indicate that the signal has come
        printf("\nsig handler stop m1\n"); 
        fflush(stdout);
        //opens distance x fifo
        fd_dx = open(myfifo_dx,O_RDWR);
        //error message 
        if (fd_dx == 0) {
            perror("Cannot open fifo_dx");
            unlink(myfifo_dx);
            exit(1);
        }
        //writes updated distance to the fd
        write(fd_dx, &x_dist, sizeof(float));
        //closes it
        close(fd_dx);
        //writes the updated distance to the logfile of m1 with the message no 1
        write_value(x_dist, 1);
    }       
}

void sig_handler_reset(int signo){

    if(signo == SIGUSR2){
        //prints a message to indicate that the signal has come
        printf("\nsig handler reset m1\n");
        fflush(stdout);
        //opens velocity x fifo
        fd_vx = open(myfifo_vx, O_RDWR);
        //reads the value which is equal to zero
        read(fd_vx, &x_vel, sizeof(float));
        //then, opens the fd of distance x
        fd_dx = open(myfifo_dx,O_RDWR); 
        //error message
        if (fd_dx == 0) {
            perror("Cannot open fifo_dx");
            unlink(myfifo_dx);
            exit(1);
        }
        //checks x distance and velocity if there is no update in x velocity and x distance is not zero
        //it continues to decreasing the distance
        while(x_dist >= 0 && x_vel == 0){
            //10 is an arbitrary number 
            x_dist -= 10*t;
            //writes the updated distance to the fd 
            write(fd_dx, &x_dist, sizeof(float));
            //writes the updated distance to the logfile with the necessary signal message
            write_value(x_dist, 2);
            //read the x velocity again to break the loop if Vx++ or Vx-- is pressed
            read(fd_vx, &x_vel, sizeof(float));

        }
        //close both fds
        close(fd_dx);close(fd_vx);

    }
    
}



//M1 child process
int main(int argc, char const *argv[]){
    //makes distance x fd
    mkfifo(myfifo_dx, 0666);
    
    //signal checks for both signals
    if (signal(SIGUSR1, sig_handler_stop) == SIG_ERR)
        printf("\ncan't catch stop signal\n");
    if (signal(SIGUSR2, sig_handler_reset) == SIG_ERR)
        printf("\ncan't catch reset signal\n");


    while (1){ 
        //opens velocity x and distance x fds
        fd_vx = open(myfifo_vx,O_RDWR);
        fd_dx = open(myfifo_dx,O_RDWR);
        
        //error messages if they do not open
        if (fd_vx== 0) {
            perror("Cannot open fifo_vx");
            unlink(myfifo_vx);
            exit(1);
        }
        if (fd_dx == 0) {
            perror("Cannot open fifo_dx");
            unlink(myfifo_dx);
            exit(1);
        }
        
        //read x velocity
        read(fd_vx, &x_vel, sizeof(float));
        //close fd
        close(fd_vx);
       
        //if x distance is between max and min 
       if((Xmin <= x_dist) && (x_dist <= Xmax)){

            if(x_vel == 0){
                //write it to the fd
                write(fd_dx, &x_dist, sizeof(float));
                //write it to the logfile
                write_value (x_dist, 0);
            } 
            else{
                //calculate the new distance
                x_dist += x_vel*t;
                //write it to the fd
                write(fd_dx, &x_dist, sizeof(float));
                //write it to the logfile
                write_value (x_dist, 0);
            }
                           
        }
        //error message
        else{
            write_value(x_dist, 3);
            printf("Max distance ib x is achieved\n");
            fflush(stdout);
        } 
        //close fd velocity x
        close(fd_dx); 
    } 
    return 0; 
	
}
