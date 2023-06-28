/*program name: asgn5-scottb4.c
 *location:     /scottb4/cs352
 *compile:      gcc asgn5-scottb4.c
 *run:          ./a.out
 *description:  Final implementation of Batch Job Dispatcher which contains
 *              implementations of insertJob and deleteFirstJob.
 *collaborated
 *with:         enabledornot
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <pthread.h>

struct JOB {
   char *command[5]; 
   long submitTime;  // domain = { positive long integer, seconds }
   long startTime;   // domain = { positive long integer, seconds}
   struct JOB *next;
};
typedef struct JOB Job;

struct LIST {
   int numOfJobs;        // number of jobs in the list
   Job *firstJob;        // points to first job. NULL if empty
   Job *lastJob;         // points to last job. NULL if empty
};
typedef struct LIST List;

void appendJob(List *list, Job *jobPtr) {
    if(list->firstJob == NULL) {
        list->firstJob = jobPtr;
        list->lastJob = jobPtr;
    }
    else {
        list->lastJob->next = jobPtr;
        list->lastJob = jobPtr;
    }
    return;
}

void printCommands(Job *jobPtr) {
    if(jobPtr == NULL) {
        printf("can't print NULL job\n");
        return;
    }
    int i = 0;
    while(jobPtr->command[i] != NULL) {
        printf("%s ", jobPtr->command[i]);
        i++;
    }
    printf("\n");
    return;
}

void printJob(Job *jobPtr) {
    printf("program name: ");
    printCommands(jobPtr);
    printf("submission time: %ld\n", jobPtr->submitTime);
    printf("start time: %ld\n", jobPtr->startTime);
}

void printList(List *list) {
    if(list->firstJob == NULL) {
        printf("List Empty\n");
        return;
    }
    Job *jobPtr = list->firstJob;
    printf("program started at: %ld", time(NULL));
    printf("\n# of jobs: %d\n", list->numOfJobs);
    for(int i = 0; jobPtr != NULL; i++) {
        printf("Job %d:\n", i+1);
        printJob(jobPtr);
        jobPtr = jobPtr->next;
    }
}

void getParameters(Job *jobPtr) {
    int numArgs;
    jobPtr->submitTime = time(NULL);
    jobPtr->next = NULL;
    scanf("%d ", &numArgs);
    int i;
    for(i = 0; i < numArgs; i++) {
        jobPtr->command[i] = malloc(sizeof(char)*25);  //J.P. Fox
        scanf("%s ", jobPtr->command[i]);
    }
    jobPtr->command[i] = NULL;
    scanf("%ld",&jobPtr->startTime);
}

int sumOfTimes(Job *jobPtr) {
    if(jobPtr == NULL) {
        return 0;
    }
    return jobPtr->startTime + jobPtr->submitTime;
}

/* If the newJobPtr has a greater startTime + submitTime, return 1
 * If the jobInListPtr has a greater startTime + submitTime, return 0
*/
int ifNewJobIsYounger(Job *newJobPtr, Job *jobInListPtr) {
    if(jobInListPtr == NULL) {
        return 1;
    }
    if(sumOfTimes(newJobPtr) > sumOfTimes(jobInListPtr)) {
        return 1;
    }
    else if(sumOfTimes(newJobPtr) < sumOfTimes(jobInListPtr)) {
        return 0;
    }
    else {
        if(newJobPtr->submitTime < jobInListPtr->submitTime) {
            return 1;
        }
        else {
            return 0;
        }
    }
}

void findInsertionLocation(List *list, Job *jobPtr) {
    Job *nextJob = list->firstJob->next;
    Job *currentJob = list->firstJob;
    while(ifNewJobIsYounger(jobPtr, nextJob) == 1 && nextJob != NULL) {
        currentJob = nextJob;
        nextJob = nextJob->next;
    }
    if(nextJob == NULL) {
        appendJob(list, jobPtr);
    }
    else {
        jobPtr->next = nextJob;
        currentJob->next = jobPtr;
    }
}

void insertJob(List *list, Job *jobPtr) {
    if(list->firstJob == NULL) {//Insert Job as first
        appendJob(list, jobPtr);
    }
    else if(ifNewJobIsYounger(jobPtr, list->firstJob) == 0) {//Insert new 1st
        jobPtr->next = list->firstJob;
        list->firstJob = jobPtr;
    }
    else if(ifNewJobIsYounger(jobPtr, list->lastJob) == 1) {
        appendJob(list, jobPtr);
    }
    else {
        findInsertionLocation(list, jobPtr);
    }
    list->numOfJobs = list->numOfJobs + 1;
}

Job *deleteFirstJob(List *list) {
    if(list->firstJob == NULL) {
        return NULL;
    }
    Job *tempJob = list->firstJob;
    if(list->firstJob == list->lastJob) {
        list->firstJob = NULL;
        list->lastJob = NULL;
    }
    else {
        list->firstJob = list->firstJob->next;
        printf("Job Deleted:\n");
        printf("program name:");
        printCommands(tempJob);
        printf("submit time: %ld\n", tempJob->submitTime);
        printf("start time: %ld\n", tempJob->startTime);
    }
    list->numOfJobs = list->numOfJobs - 1;
    return tempJob;
}

void freeJob(Job *jobPtr) {
    for(int i = 0; i < 5 && jobPtr->command[i] != NULL; i++) {
        free(jobPtr->command[i]);
    }
    free(jobPtr);
}

void *executerThread(void *args) {
    Job *jobPtr = (Job*)args;
    int child_pid = fork();
    if(child_pid != 0) {//You're the parent
        //Waits for child process to complete
        waitpid(child_pid, NULL, 0);
        printf("WEXITSTATUS: %d\n", WEXITSTATUS(0));
    }
    else {//You're the child which will be replaced with the command
        execvp(jobPtr->command[0], jobPtr->command);
        printf("WEXITSTATUS: %d\n", WEXITSTATUS(1));
    }
    return NULL;
}

void *dispatcherThread(void *args) {
    List *jobQueue = (List*)args;
    long currentSysTime;
    while(1) {
        currentSysTime = time(NULL);
        if(jobQueue->numOfJobs <= 0) {
            sleep(1);
        }
        else if(currentSysTime >= sumOfTimes(jobQueue->firstJob)) {
            Job *jobPtr = deleteFirstJob(jobQueue);
            pthread_t executer;
            pthread_create(&executer, NULL, executerThread, (void*)jobPtr);
            pthread_join(executer, NULL);
            freeJob(jobPtr);
        }
        else {
            sleep(1);
        }
    }
}

void *schedulerThread(void *args) {
    List *jobQueue = (List*)args;
    char command = ' ';
    while(1) {
        scanf(" %c", &command);
        if(command == '+') {
            Job *newJob = (Job*)malloc(sizeof(Job));
            getParameters(newJob);
            insertJob(jobQueue, newJob);
        }
        else if(command == '-') {
            Job *oldJob = deleteFirstJob(jobQueue);
            freeJob(oldJob);
        }
        else if(command == 'p') {
            printList(jobQueue);
        }
        command = ' ';
        sleep(1);
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    List *jobQueue = (List*)malloc(sizeof(List));
    jobQueue->firstJob = NULL;
    jobQueue->lastJob = NULL;
    jobQueue->numOfJobs = 0;
    pthread_t scheduler;
    pthread_t dispatcher;
    pthread_create(&scheduler, NULL, schedulerThread, (void*)jobQueue);
    pthread_create(&dispatcher, NULL, dispatcherThread, (void*)jobQueue);
    pthread_join(scheduler, NULL);
    pthread_join(dispatcher, NULL);
}