#define _GNU_SOURCE
# define _USE_GNU
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sched.h>
#include<sys/wait.h>
#include<sys/types.h>

#define FIFO 0
#define RR 1
#define SJF 2
#define PSJF 3


#define SYSCALLNUM 334
#define MAX_NAME_LENGTH (int)32
#define MAX_PROCESS_NUM (int)1E1
#define TIME_QUANTUM (int)5E2
#define N (long long)1E9
#define PRIORITYH (int)80
#define PRIORITYL (int)10
#define PRIORITYINIT (int)50;

int cur_process_num = 0;
int finished_process_num = 0;

typedef struct process{
  char name[MAX_NAME_LENGTH];
  int ready_time;
  int exec_time;
  int ID;
  //long long start_s;
  //long long start_ns;
  //long long end_s;
  //long long end_ns;
}PROCESS;

int Compare_Process(const void *p1,const void *p2){
  PROCESS *P1 = (PROCESS *)p1;
  PROCESS *P2 = (PROCESS *)p2;
  if(P1->ready_time < P2->ready_time)
    return -1;
  if(P1->ready_time > P2->ready_time)
    return 1;
  if(P1->ID < P2->ID)
    return -1;
  if(P1->ID > P2->ID)
    return 1;
  return 0;
}

void Insert_Process(PROCESS **waiting_list, int policy, PROCESS *P){
  if(policy==FIFO || policy==RR)
    waiting_list[cur_process_num] = P;
  if(policy==PSJF || policy==SJF){
    waiting_list[cur_process_num] = P;
    for(int i = cur_process_num; i > 0; i--){
        if(waiting_list[i]->exec_time < waiting_list[i-1]->exec_time){
          PROCESS *temp = waiting_list[i];
          waiting_list[i] = waiting_list[i-1];
          waiting_list[i-1] = temp;
        }
        else if(waiting_list[i]->exec_time >= waiting_list[i-1]->exec_time)
          break;
    }
  }
  cur_process_num++;
}

int Exec_Process(PROCESS **waiting_list, int policy){
  if(policy==FIFO || policy==SJF){
    int exec_length = waiting_list[0]->exec_time;
    waiting_list[0]->exec_time = 0;
    waiting_list[0] = NULL;
    cur_process_num--;
    finished_process_num++;
    for(int i = 1;i<=cur_process_num;i++){
      PROCESS *temp = waiting_list[i];
      waiting_list[i] = waiting_list[i-1];
      waiting_list[i-1] = temp;
    }
    return exec_length;
  }
  if(policy==RR){
    int exec_length;
    if(waiting_list[0]->exec_time > TIME_QUANTUM){
      exec_length = TIME_QUANTUM;
      waiting_list[0]->exec_time -= TIME_QUANTUM;
    }
    else{
      exec_length = waiting_list[0]->exec_time;
      waiting_list[0]->exec_time = 0;
      waiting_list[0] = NULL;
    }
    for(int i = 1;i<cur_process_num;i++){
      PROCESS *temp = waiting_list[i];
      waiting_list[i] = waiting_list[i-1];
      waiting_list[i-1] = temp;
    }
    if(waiting_list[cur_process_num-1]==NULL){
      cur_process_num--;
      finished_process_num++;
    }
    return exec_length;
  }
  if(policy==PSJF){//one unit per time cause can't be done a single time like the upper ones
    int exec_length;
    waiting_list[0]->exec_time--;
    if(waiting_list[0]->exec_time == 0){
      waiting_list[0] = NULL;
      cur_process_num--;
      finished_process_num++;
      for(int i = 1;i<=cur_process_num;i++){
        PROCESS *temp = waiting_list[i];
        waiting_list[i] = waiting_list[i-1];
        waiting_list[i-1] = temp;
      }
    }
    if(waiting_list[cur_process_num-1]==NULL){
      cur_process_num--;
      finished_process_num++;
    }
    return 1;
  }
}

int main(){
  PROCESS P[MAX_PROCESS_NUM];
  char policy_name[5];
  int num_process;
  char process_name[MAX_NAME_LENGTH];
  scanf("%s",policy_name);
  scanf("%d",&num_process);

////init process////
  for(int i = 0;i<num_process;i++){
    int ready_time;
    int exec_time;
    scanf("%s%d%d",process_name,&ready_time,&exec_time);
    strcpy(P[i].name,process_name);
    P[i].ready_time = ready_time;
    P[i].exec_time = exec_time;
    P[i].ID = i;
  }
  qsort(P,num_process,sizeof(PROCESS),Compare_Process);

////decide policy////
  int policy = 0;
  char policy_list[4][5] = {"FIFO","RR","SJF","PSJF"};
  for(int i = 0;i<4;i++)
    if(strcmp(policy_name,policy_list[i])==0)
      policy = i;

////process queue initializing////
  PROCESS *waiting_list[MAX_PROCESS_NUM];
  for(int i = 0;i<MAX_PROCESS_NUM;i++)
    waiting_list[i] = NULL;
  cpu_set_t cpu_mask;
  CPU_ZERO(&cpu_mask);
  CPU_SET(0,&cpu_mask);
  if(sched_setaffinity(0,sizeof(cpu_set_t),&cpu_mask)!=0){
    perror("sched_setaffinity error");
    exit(EXIT_FAILURE);
  }

////schedule init////
  struct sched_param param;
  param.sched_priority = PRIORITYINIT;
  pid_t pidP = getpid();
  if(sched_setscheduler(pidP,SCHED_FIFO,&param)!=0){
    //printf("fuck\n");
    perror("sched_setscheduler error");
    exit(EXIT_FAILURE);
  }

////real deal////
  int time_count = 0;
  int fork_count = 0;
  PROCESS *exec_process = NULL;
  PROCESS *exec_process_last = NULL;
  int num_process_finish_last = 0;
  int exec_length = 0;
  pid_t pids[MAX_PROCESS_NUM];
  while(!(finished_process_num==num_process && exec_length==0)){
    //long long start_time = syscall(SYSCALLNUM);
    //long long start_time_s = start_time/N;
    //long long start_time_ns = start_time%N;
    while(P[fork_count].ready_time<=time_count && fork_count < num_process){
      pid_t pid = fork();
      if(pid<0){
        perror("fork error");
        exit(EXIT_FAILURE);
      }
      else if(pid==0){
        char exec_time[10];
        sprintf(exec_time,"%d",P[fork_count].exec_time);
        //char start_time_s_string[20];
        //char start_time_ns_string[20];
        //sprintf(start_time_s_string,"%lld",start_time_s);
        //sprintf(start_time_ns_string,"%lld",start_time_ns);
        if(execlp("./process","process",P[fork_count].name,exec_time/*,start_time_s_string,start_time_ns_string*/,(char*)NULL)<0){
          perror("execlp error");
          exit(EXIT_FAILURE);
        }
      }
      cpu_set_t cpu_mask;
      CPU_ZERO(&cpu_mask);
      CPU_SET(1,&cpu_mask);
      if(sched_setaffinity(pid,sizeof(cpu_set_t),&cpu_mask)!=0){
        perror("sched_setaffinity error");
        exit(EXIT_FAILURE);
      }
      pids[P[fork_count].ID] = pid;
      Insert_Process(waiting_list,policy,(P+fork_count));
      fork_count++;
    }
    if(waiting_list[0]==NULL && exec_length==0){
      time_count++;
      volatile unsigned long i;
      for(i=0;i<1000000UL;i++);
      exec_process_last = NULL;
    }
    else{
      if(exec_length==0){
        exec_process = waiting_list[0];
        exec_length = Exec_Process(waiting_list,policy);
        if(exec_process_last==NULL || exec_process_last->exec_time==0){
          //printf("Set %s to high at time %d\n",exec_process->name,time_count);
          pid_t pid = pids[exec_process->ID];
          //printf("PID: %d\n",pid);
          param.sched_priority = PRIORITYH;
          if(sched_setscheduler(pid,SCHED_RR,&param)!=0){
            //printf("haha\n");
	        perror("sched_setscheduler error");
            exit(EXIT_FAILURE);
          }
          //if(sched_getscheduler(pid)==SCHED_RR)
            //puts("RR");
        }
        else{
          //printf("Set %s to high, recover %s to low at time %d\n",exec_process->name,exec_process_last->name,time_count);
          pid_t pid_last = pids[exec_process_last->ID];
          pid_t pid = pids[exec_process->ID];
          //printf("PID: %d\n",pid);
          param.sched_priority = PRIORITYL;
          if(sched_setscheduler(pid_last,SCHED_RR,&param)!=0){
            //printf("hehe\n");
            perror("sched_setscheduler error");
            exit(EXIT_FAILURE);
          }
          param.sched_priority = PRIORITYH;
          if(sched_setscheduler(pid,SCHED_FIFO,&param)!=0){
            //printf("hoho\n");
            perror("sched_setscheduler error");
            exit(EXIT_FAILURE);
          }
        }
      }
      exec_length--;
      time_count++;
      volatile unsigned long i;
      for(i=0;i<1000000UL;i++);
      if(exec_length==0){
        if(finished_process_num==num_process_finish_last+1){
          //printf("%s : end at time %d\n",exec_process->name,time_count);
          int status;
          if(waitpid(pids[exec_process->ID],&status,0)==-1){
            perror("waitpid error");
            exit(EXIT_FAILURE);
          }
        }
        exec_process_last = exec_process;
        num_process_finish_last = finished_process_num;
      }
    }
  }
  for(int i = 0;i<num_process;i++){
    printf("%s ",P[i].name);
    printf("%d\n",pids[P[i].ID]);
  }
  return 0;
}
