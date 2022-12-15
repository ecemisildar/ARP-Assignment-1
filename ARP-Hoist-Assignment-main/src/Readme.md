# PROJECT DESCRIPTION
In this project the aim is to control a Hoist in 2D using the command and inspector windows. In order to do this, 6 different processes created by Master were given below:
1. Command Console (take operator inputs)
2. M1 (to control x direction)
3. M2 (to control z direction)
4. World (assigns random distance with an error of ±5%)
5. Inspector (user interface)
6. Watchdog (checks if the processes work)

# MASTER:
Master process is the father of all remaining six processes. It creates children and wait until their job is finished.

# CHILDREN:
The first child is the command console takes the button inputs from the operator or user. There are 6 buttons in order to increase or decrease the velocties in both directions also there are two more buttons to stop the machine immediately. When one of the buttons is pushed, it takes an arbitrary velocity and goes until the user pushes another button. There is a limitation of pushing the same button to block Hoist crashes to the walls. The dimensions of the environment is written in the inspector's header file as 40 to 10. Also, it can take two signals coming from the inspector or the watchdog to stop or reset the process. Command console sends the data inside using two different pipes to the next processes.

The second and third children are same with a slight difference, while M1 controls the distance in the x direction, M2 controls the distance in the z direction. They take velocity inputs from command process using file desrciptors under specific conditions. So, they check the distance in every iteration not to exceed the maximum dimensions. Both two processes also can take 2 signals that are reset and stop for some emergency conditions. At the end both of the processes sends their updated distance values to the next process.

The fourth process is called world. The aim of its creation is calculating the actual value of Hoist can go according to the desired distances. The error is always in between ±5%. Also, it has to understand which pipe comes first and if both of them comes at the same time it choses randomly between them. To make a selection between processes select() command is used. After that, it calculates the real distance with a random error. When it finishes that part it needs to send this real distance data to the next child using one pipe. Indicating which one is the distance of which direction, it is added letters x and z to distinguish the data.

In the fifth process which is called inspector, the aim is to see the results of all these calculations and moving the Hoist machine in 2D. It updates the real coordinates of the Hoist in every iteration, then it can be seen in the user interface using the strncmp() command. Because the coming data consists of a letter and a number eg. x: 11.20. It takes the number part and assign it to the end effector coordinate. Also, this inspector window has two emergency buttons called stop and reset. Aim of these buttons are interrupt the processes to keep it safe. 

The last process is watchdog and its aim is observing all children process in every 60 seconds and reset the Hoist using the reset signal like inspection console and if there is no action in all the processes. A function called getFileModifiedTime() is written for taking the last modifiaction time of the log files of the processes.

# SIGNALS:
There are three different signals in this project and they are in watchdog and in the inspection processes. Two of them are user defined signals and the other one is interrupt signal which stops running to the code when user pressed ctrl+c. Stop and reset signals are send to 3 different processes which are command, M1 and, M2. When stop signal comes command makes both velocities zero, M1 and M2 calculates the new distance according to zero velocity and it stays at the same position. When reset signal comes command makes again the same thing, but this time M1 and M2 decreases the distance by 10 in every iteration until any other button is pressed or it can make Hoist coordinates equals to (0,0).
In order to send these signals to the right processes is provided by popen(pidof "filename", "r") command.

# LOGFILES:
All children has own logfiles in this project different from their file descriptors. Because, fds cannot be observed they are in the temporary location. But using logfiles gives a chance to write all current times into the files and check all of them regularly using the watchdog.




