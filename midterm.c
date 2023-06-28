#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <pthread.h>

int main() {
    int x = 200;
    printf("A: %d--%d\n", getpid(), x);
    int cpuid = fork();
    if(cpuid != 0) {
        x = x + 300;
        printf("B: %d--%d--%d\n", getpid(),cpuid, x);
        char* sys_argv[] = {"ps", NULL};
        execvp("ls", sys_argv);
        printf("C: Process done sucessfully!\n");
    }
    else {
        sleep(3);
        x = x + 400;
        printf("D: %d--%d--%d\n", getpid(), cpuid, x);
    }

}