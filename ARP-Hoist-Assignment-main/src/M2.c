#include <stdio.h>
#include <string.h> 
#include <fcntl.h> 
#include <sys/stat.h> 
#include <sys/types.h> 
#include <unistd.h> 
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#define Zmax 10.0
#define Zmin 0.0

//M2 process calculates the distance in the z direction
//file descriptors
int fd_vz, fd_dz; 
//z distance and velocity
float z_vel;
float z_dist;
//time constant
float t = 0.035;
//logfile for this process
char *fm2 = "m2";
//location of the file descriptors
char * myfifo_vz = "/tmp/myfifo_vz"; //fifo for z velocity
char * myfifo_dz = "/tmp/myfifo_dz"; //fifo for z distance 

int i = 0;

//writes the current time, z distance into the logfile
//inputs: z distance, message type (normal, reset signal or stop signal)
//outputs: function type is void, no output
void write_value (float z, int i) {
    //takes the local time of the computer
    char dt[20]; // space enough for YYYY-MM-DD HH:MM:SS and terminator
    struct tm tm;
    time_t current_time;
    current_time = time(NULL);
    tm = *localtime(&current_time); // convert time_t to struct tm
    strftime(dt, sizeof dt, "%Y-%m-%d %H:%M:%S", &tm); // format

    //Write to log file
    fm2 = fopen ("m2", "a");
    if(i == 0) fprintf(fm2, "time: %s z dist: %.2f\n", dt, z); //prints z distance with time
    else if(i == 1) fprintf(fm2, "time: %s STOP SIGNAL  z dist: %.2f\n", dt, z); //indicates that stop signal has arrived
    else if(i == 2) fprintf(fm2, "time: %s RESET SIGNAL z dist: %.2f\n", dt, z);//indicates that reset signal has arrived
    else if(i == 3) fprintf(fm2, "time: %s MAX DISTANCE IS ACHIEVED\n", dt); //indicates max distance
    fclose (fm2); //closes logfile

}

//when stop signal arrives, it makes zero to z velocity and calculates the new distance
//which is as the same as before 
//and prints them to the logfile with the message: "STOP SIGNAL HAS ARRIVED" 
void sig_handler_stop(int signo){

    if(signo == SIGUSR1){
        z_vel = 0;
        z_dist += z_vel*t;
        //prints a message to indicate that the signal has come
        printf("\nsig handler stop m2\n");
        fflush(stdout);
        //opens distance z fifo
        fd_dz = open(myfifo_dz,O_RDWR); 
        //error message 
        if (fd_dz== 0) {
            perror("Cannot open fifo_dz");
            unlink(myfifo_dz);
            exit(1);
        }
        //writes updated distance to the fd
        write(fd_dz, &z_dist, sizeof(float));
        //closes it
        close(fd_dz);
        //writes the updated distance to the logfile of m2 with the message no 1
        write_value(z_dist, 1);
    }
    
}

void sig_handler_reset(int signo){
   
    if(signo == SIGUSR2){
        //prints a message to indicate that the signal has come
        printf("\nsig handler reset m2\n");
        fflush(stdout);
        //opens velocity x fifo
        fd_vz = open(myfifo_vz, O_RDWR);
        //reads the value which is equal to zero
        read(fd_vz, &z_vel, sizeof(float));
        //then, opens the fd of distance z
        fd_dz = open(myfifo_dz,O_RDWR); 
        //error message
        if (fd_dz== 0) {
            perror("Cannot open fifo_dz");
            unlink(myfifo_dz);
            exit(1);
        }
        //checks x distance and velocity if there is no update in x velocity and x distance is not zero
        //it continues to decreasing the distance 
        while(z_dist >= 0 && z_vel == 0){
             //10 is an arbitrary number 
            z_dist -= 0.25*10*t;  
            //writes the updated distance to the fd 
            write(fd_dz, &z_dist, sizeof(float)); 
            //writes the updated distance to the logfile with the necessary signal message       
            write_value(z_dist, 2);
            //read the x velocity again to break the loop if Vx++ or Vx-- is pressed
            read(fd_vz, &z_vel, sizeof(float));
            
        }
        //close both fds
        close(fd_dz);close(fd_vz);
        
    }
    
}



//M2 child process
int main(int argc, char const *argv[]){
  	//makes distance x fd
    mkfifo(myfifo_dz, 0666);

    //signal checks for both signals
    if (signal(SIGUSR1, sig_handler_stop) == SIG_ERR)
        printf("\ncan't catch stop signal\n");
    if (signal(SIGUSR2, sig_handler_reset) == SIG_ERR)
        printf("\ncan't catch reset signal\n");


    while (1) { 
        //opens velocity x and distance x fds
        fd_vz = open(myfifo_vz,O_RDWR);
        fd_dz = open(myfifo_dz,O_RDWR); 
        //error messages if they do not open
        if (fd_vz== 0) {
            perror("Cannot open fifo_vz");
            unlink(myfifo_vz);
            exit(1);
        }
        if (fd_dz== 0) {
            perror("Cannot open fifo_dz");
            unlink(myfifo_dz);
            exit(1);
        }
        //read z velocity
        read(fd_vz, &z_vel, sizeof(float));
        //close fd
        close(fd_vz);

        

        if((Zmin <= z_dist) && (z_dist<= Zmax)){ 

            if(z_vel == 0){
                //write distance to the fd 
                write(fd_dz, &z_dist, sizeof(float));
                //write it to the logfile
                write_value (z_dist, 0);

            }
            else{
                //calculate the new distance
                z_dist += 0.25*z_vel*t;
                //write it to the fd     
                write(fd_dz, &z_dist, sizeof(float));
                //write it to the logfile
                write_value (z_dist, 0);
            }
            
        }
        //error message
        else{
            write_value(z_dist, 3);
            printf("Max distance in z is achieved\n");
            fflush(stdout);
        } 
         //close fd velocity z
        close(fd_dz);       

    } 
    return 0; 
	
}        
