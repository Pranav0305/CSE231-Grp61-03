## CSE231 Operating Systems Monsoon Semester 
### Group-61
### Assignment 3

### MEMBERS: Naman Birla (2022310) and Pranav Bharadwaj(2022363)

## Assignment Details

To implement a SimpleShell that waits for user input, executes commands provided in the user. The shell follows the scheudling policy implemented by the scheduler, a process of the shell itself. The scheduler schedules the jobs in the order of priorities and follows the round robin approach, allowing the processes to run for a certain period of time (quantum) and then switching the processes. The number of processes to be run at a time are specified by the user itself. The ```submit``` command should be used to submit an executable and can be used to specify the priority. (eg, ```submit ./fib 4```)

The highest priority is 1 and the lowest is 4. By default the process enters the queue of highest priority. The queue of highest priority having a process is executed first (again, in a round robin approach) and after every quanta the priority of the process decreases and it is shifted into the next priority queue.


## How to test
A linux based machine is required to run the simple-shell.

The Makefile conatains the appropriate code required for the working of the simpleshell. It also cleans them internally after exitting the program.

1. Open the project in the terminal and run the command ```make```. 

It compiles the ```helloworld.c```, ```fib.c```  and  ```main.c``` files to produce the executables ```hello```, ```fib``` and ```main``` respectively.

2. For running the scheduler + shell implementation, run the command "./main cpus timeslice" replacing cpus with an integer specifying the number of processes to be run at a time and replacing timeslice with quantum of round robin in miliseconds. For example. ``` ./main 2 5000 ``` with 2 cpus and 5000ms time quanta.

3. For exitting, you'll have to use "Ctrl-C" which then displays all the processes submitted to the shell + scheduler and also displays the wait time and execution time of all processes and the average.

The user should use the "submit" command to submit an executable. The priority of the process can be provided alongside as a command line parameter. ```submit ./fib 4``` submits the ```fib``` executable as a priority 4 process.
By default, the process is sent into priority 1 i.e. the highest priority.

The executables created are internally cleaned when the program is exitted.


## Contributions

The members have more-or-less equally contributed to the making of the code. 

The general overview of the work done:

Pranav Bharadwaj:
1.	Implementing the queues.
2.	Handling the priorities of tasks to be scheduled.
3.	Error handling.


Naman Birla:
1.	Modifying the shell to incorporate the scheduler.
2.	Applying the round-robin approach.
3.	Error handling.

