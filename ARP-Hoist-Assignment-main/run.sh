#!/bin/bash
#Script to compile all the processes and run the master
gcc src/master.c -o bin/master 
gcc src/inspection_console.c -lncurses -lm -o bin/inspection
gcc src/command_console.c -lncurses -o bin/command
gcc src/M1.c -o bin/M1
gcc src/M2.c -o bin/M2
gcc src/world.c -o bin/world
gcc src/watchdog.c -o bin/watchdog
./bin/master
