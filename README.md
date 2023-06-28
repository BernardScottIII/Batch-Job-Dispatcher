# Batch-Job-Dispatcher

# Description
A program which consists of a job queue and three threads, a `scheduler`, a `dispatcher`, and an `executer`. These threads work together during program execution. 

## Scheduler Thread
The `scheduler` thread listens for input to accept new jobs during execution. It accepts input in the form of direct user input, or a prewritten `input.txt` file created for automated testing. The `scheduler` thread is responsible for adding new jobs, deleting the first job, and printing the job list for debugging.

## Dispatcher Thread
The `dispatcher` thread watches the job queue to determine if the first job in the queue should execute. If there is no job ready to execute, the thread will use the `sleep()` call for one second, then check the queue again. If the check does not result in a job ready for execution, the thread sleeps again and continuously repeates this behavior.

## Executer Thread
When appropriate, the `executer` thread executes jobs at their specified time \(seconds after the program starts\). The executer thread uses the `fork` system call to execute the job, and waits until the job has completed execution to terminate.