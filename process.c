#define _GNU_SOURCE
#define _USE_GNU
#include <stdio.h>
#include <stdlib.h> 
#include <sys/types.h>
#include <unistd.h>
#include <sched.h>
const long long n = 1E9;
const int buffer_size = 1E2;

int main(int argc, char* argv[]){
    
    
    
    char* name = argv[1];
    pid_t pid = getpid();
    int exec_time;
    exec_time = strtol(argv[2], NULL, 10);
    //char* start_time_s = argv[3];
    //char* start_time_ns = argv[4];
    long start_time = syscall(334);


    int i;
    for(i = 0; i < exec_time; i++){
        volatile unsigned long j;
        for(j = 0; j < 1000000UL; j++); 
    }
    
    long end_time = syscall(334);
    long long end_time_s = end_time / n;
    long long end_time_ns = end_time % n;

    //printf("[project1] %d %s.%s %lld.%lld\n", pid, start_time_s, start_time_ns, end_time_s, end_time_ns);
    printf("[Project1] %d %ld.%ld %ld.%ld\n", pid, start_time/1000000000l%1000, start_time%1000000000l, end_time/1000000000l%1000, end_time%1000000000l);
    syscall(337, pid, start_time, end_time);
    return 0;
}
