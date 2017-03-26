Documentation for Warmup Assignment 2
=====================================

Name: Suraj Shankar
USC email: surajhes@usc.edu

+-------+
| BUILD |
+-------+

Comments: Type make warmup2 in the nunki shell.
make: Compiles the C files and creates an executable called warmup2
make warmup2:  Compiles the C files and creates an executable called warmup2
make clean: Removes all .o and executable files 

+-----------------+
| SKIP (Optional) |
+-----------------+

This section is left blank intentionally.

+---------+
| GRADING |
+---------+

Basic running of the code : 100 out of 100 pts

Missing required section(s) in README file : None
Cannot compile : No, can compile
Compiler warnings : No warnings displayed
"make clean" : Removes all .o and executable files
Segmentation faults : None observed
Separate compilation : The individual c files are compiled when make command is used.
Using busy-wait : No
Handling of commandline arguments:
    1) -n : Successful
    2) -lambda : Successful
    3) -mu : Successful
    4) -r : Successful
    5) -B : Successful
    6) -P : Successful
Trace output :
    1) regular packets: 7 lines of trace as per spec
    2) dropped packets: 1 line of trace indicating drop as per spec
    3) removed packets: 1 line of trace when packet is removed as per spec
    4) token arrival (dropped or not dropped): 1 line of trace as per spec
Statistics output :
    1) inter-arrival time : Successful
    2) service time : Successful
    3) number of customers in Q1 : Successful
    4) number of customers in Q2 : Successful
    5) number of customers at a server : Successful
    6) time in system : Successful
    7) standard deviation for time in system : Successful
    8) drop probability : Successful
Output bad format : No
Output wrong precision for statistics (should be 6-8 significant digits) : No
Large service time test : Pass
Large inter-arrival time test : Pass
Tiny inter-arrival time test : Pass
Tiny service time test : Pass
Large total number of customers test : Pass
Large total number of customers with high arrival rate test : Pass
Dropped tokens test : Pass
Cannot handle <Cntrl+C> at all (ignored or no statistics) : Handling <Cntrl+C> successfully
Can handle <Cntrl+C> but statistics way off : No
Not using condition variables and do some kind of busy-wait : No, using condition variables
Synchronization check : Pass
Deadlocks : No
  


+------+
| BUGS |
+------+

Comments: None found

+------------------+
| OTHER (Optional) |
+------------------+

Comments on design decisions: 1) Used 6 threads, 1 packet creation, 1 token generation, 2 server, 1 main and 1 signal handler thread.
                              2) Each thread is having its implementation in seperate file. 
                              3) Globally shared data are accessed by header file.

Comments on deviation from spec: No deviations from the spec.

