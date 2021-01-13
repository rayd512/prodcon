# Producer Consumer

A simple introduction into multithreaded programming. 

# Description
The program will create the desired number of threads and will take input from the command line to give work to the threads.

# How to use

First compile and then run the program with the following syntax.  
```./prodcon <nthreads> <log_id>```
Where nthreads is the number of threads to spawn and log_id is the number associated with the log file created by the program.  

Once running, the program can now accept one of two inputs:  
`T<n>`, where n is the "amount" of work and,
`S<n>`, where s is the amount of time the program will sleep for, to simulate wait time between transactions  

# Sample Input
T8  
S8  
T100  
T40  
S9

