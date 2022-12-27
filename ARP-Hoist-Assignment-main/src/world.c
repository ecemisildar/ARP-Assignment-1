#include <stdio.h>
#include <string.h> 
#include <fcntl.h> 
#include <sys/stat.h> 
#include <sys/types.h> 
#include <unistd.h> 
#include <stdlib.h>
#include <sys/select.h>
#include <time.h>

//World process calculates the real distance in the x and z direction
//file descriptors
int fd_vx, fd_vz, fd_dx, fd, fd_dz, fd_real;
//x and z distance, real distance and velocity
float x_vel, z_vel, dist_x, dist_z, dist, real_x, real_z;
float dist = 0;
float x;
//two functions explained below
float RandomFloat(float x);
int calc(int fd, int flag);
//logfile for this process
char * fwrld = "wrld";

//this function will generate random number in range l and r
int generate_random(int l, int r) { 
     int rand_num = (rand() % (r - l + 1)) + l;
    return rand_num;
}

//writes the current time, real distance of x and z into the logfile
//inputs: x and z distance, message type (normal, reset signal or stop signal)
//outputs: function type is void, no output
void write_value (float x, float z) {

    char dt[20]; //space enough for YYYY-MM-DD HH:MM:SS and terminator
    struct tm tm;
    time_t current_time;
    current_time = time(NULL);
    tm = *localtime(&current_time); // convert time_t to struct tm
    strftime(dt, sizeof dt, "%Y-%m-%d %H:%M:%S", &tm); // format
    //Write to log file
    fwrld = fopen ("wrld", "a"); 
    fprintf (fwrld, "time: %s ,real x dist: %.2f, real z dist: %.2f\n", dt, x, z); //prints real x and z distance with time
    fclose (fwrld); //closes logfile

}

//choses the max number between two
int max(int num1, int num2)
{
    return (num1 > num2 ) ? num1 : num2;
}

//World child process
int main(int argc, char const *argv[]){
  	
    char * myfifo_dx = "/tmp/myfifo_dx";//fifo for x distance 
    char * myfifo_dz = "/tmp/myfifo_dz";//fifo for z distance

    char * myfifo_real = "/tmp/myfifo_real";//fifo for real distances
    mkfifo(myfifo_real, 0666);  //makes real distance fd
    
    while (1){
        //opens distance x and z fds 
        fd_dx = open(myfifo_dx,O_RDWR);
        fd_dz = open(myfifo_dz,O_RDWR);
        //opens real distance fd
        fd_real = open(myfifo_real,O_RDWR);
        //error messages if they do not open
        if (fd_dx == 0) {
            perror("Cannot open fifo_dx");
            unlink(myfifo_dx);
            exit(1);
        }
        if (fd_dz == 0) {
            perror("Cannot open fifo_dz");
            unlink(myfifo_dz);
            exit(1);
        }
        if (fd_real == 0) {
            perror("Cannot open fifo_real");
            unlink(myfifo_real);
            exit(1);
        }

        //this part is for chosing the pipe that comes first 
        //or choose them random if they come at the same time
        int rand_number;
        int retval;
        fd_set rfds;
        FD_ZERO(&rfds);
        FD_SET(fd_dx, &rfds);   
        FD_SET(fd_dz, &rfds);
        retval = select(max(fd_dz, fd_dz)+1, &rfds, NULL, NULL, NULL);
        switch (retval){
            case -1:
            perror("select()"); //error
            return -1;

            case 2: //choose randomly
            rand_number = generate_random(0,1);
            if(rand_number == 0){
                calc(fd_dz, 2); //calculate the random z distance
            }
            else{
                calc(fd_dx, 1); //calculate the random x distance
            }
            break;

            case 1: //choose the set one
            if(FD_ISSET(fd_dz, &rfds) != 0){
                calc(fd_dz, 2); //calculate the random z distance
            }
            else if(FD_ISSET(fd_dx, &rfds) != 0){
                calc(fd_dx, 1); //calculate the random x distance
            }
            break; 
        }
       
        
    }
        

	return 0;
} 

//takes any number and    
//returns a random number in the range of +-5%
float RandomFloat(float x) {
	float a = 0.95 * x; //min limit
	float b = 1.05 * x; //max limit
    float random = ((float) rand()) / (float) RAND_MAX;
    float diff = b - a;
    float r = random * diff;
    return a + r;
}

//takes the file descriptor and flag (indicates x or z)
int calc(int fd, int flag){
    //float real_dist[2]; yeni kapattim

    //read the coming distance
    read(fd, &dist, sizeof(float));
    //close that fd
    close(fd);  

    char buffer[1024];
    //if flag is 1 it is x distance
    if(flag == 1){
        //x velocity is zero, then no need to assign random error 
        //because it just the case that when reset or stop buttons are activated 
        if(x_vel == 0) real_x = dist;
        //all the other cases it is assigned a random error
        else real_x = RandomFloat(dist);
        //in order to understand which one is x or z, it is added a letter to indicate
        sprintf(buffer,"x: %f\n",real_x);
        //write the assigned real distance and x into the fd real
        write(fd_real, buffer,strlen(buffer)); 
    }
    //if flag is 2 it is z distance
    if(flag == 2){
        //z velocity is zero, then no need to assign random error 
        //because it just the case that when reset or stop buttons are activated
        if(z_vel == 0) real_z = dist;
        //all the other cases it is assigned a random error
        else real_z = RandomFloat(dist);
        //in order to understand which one is x or z, it is added a letter to indicate
        sprintf(buffer,"z: %f\n",real_z);
        //write the assigned real distance and z into the fd real
        write(fd_real, buffer,strlen(buffer));
    
        
    }
    //write real x and z distances into the logfile world
    write_value (real_x, real_z); 
 
}


