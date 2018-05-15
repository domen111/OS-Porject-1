#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <algorithm>
#include <vector>
#include <set>
#include <queue>
#define MAX_NAME_LENGTH (int)32
#define MAX_PROCESS_NUM (int)1E1
#define TIME_QUANTUM (int)5E2
#define N (long long)1E9
#define PRIORITYH (int)80
#define PRIORITYL (int)10
#define PRIORITYINIT (int)50;

using namespace std;

typedef struct process{
	char name[MAX_NAME_LENGTH];
	int ready_time;
	int exec_time;
	int ID;
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

int main()
{
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

	if (strcmp(policy_name, "FIFO") == 0) {
		qsort(P,num_process,sizeof(PROCESS),Compare_Process);
		int last_finish_time = 0;
		for (int i = 0; i < num_process; ++i) {
			int start_time = max(P[i].ready_time, last_finish_time);
			int finish_time = last_finish_time = start_time + P[i].exec_time;
			printf("%d %d\n", start_time, finish_time);
		}
	} else if(strcmp(policy_name, "RR") == 0) {
		set<int> waiting;
		vector<bool> added(num_process);
		vector<int> left_time(num_process);
		vector<pair<int,int>> result(num_process);
		int now = 0;
		int finish_count = 0;
		for (int i = 0; finish_count < num_process; i += 500) {
			for (int j = 0; j < num_process; ++j)
				if(!added[j] && P[j].ready_time <= i)
				{
					result[j].first = i;
					waiting.insert(j);
					added[j] = true;
					left_time[j] = P[j].exec_time;
					break;
				}
			if (waiting.empty()) continue;
			do {
				now = (now + 1) % num_process;
			} while(waiting.count(now) == 0);
			left_time[now] -= 500;
			if(left_time[now] <= 0)
			{
				waiting.erase(now);
				result[now].second = i + 500 - left_time[now];
				finish_count++;
			}
		}
		for (int i = 0; i < num_process; ++i)
			printf("%d %d\n", result[i].first, result[i].second);
	} else if(strcmp(policy_name, "PSJF") == 0) {
		priority_queue<pair<int,int>, vector<pair<int,int>>, greater<pair<int,int>>> waiting;
		vector<bool> added(num_process);
		vector<int> left_time(num_process);
		vector<pair<int,int>> result(num_process);
		int now = 0;
		int finish_count = 0;
		for (int i = 0; finish_count < num_process; i += 500) {
			for (int j = 0; j < num_process; ++j)
				if(!added[j] && P[j].ready_time <= i)
				{
					waiting.push({P[j].exec_time,j});
					added[j] = true;
					left_time[j] = P[j].exec_time;
				}
			if (waiting.empty()) continue;
			now = waiting.top().second;
			if (waiting.top().first == left_time[now])
				result[now].first = i;
			left_time[now] -= 500;
			if(left_time[now] <= 0)
			{
				waiting.pop();
				result[now].second = i + 500 - left_time[now];
				finish_count++;
			}
		}
		for (int i = 0; i < num_process; ++i)
			printf("%d %d\n", result[i].first, result[i].second);
	} else if(strcmp(policy_name, "SJF") == 0) {
		priority_queue<pair<int,int>, vector<pair<int,int>>, greater<pair<int,int>>> waiting;
		vector<bool> added(num_process);
		vector<int> left_time(num_process);
		vector<pair<int,int>> result(num_process);
		int now = -1;
		int finish_count = 0;
		for (int i = 0; finish_count < num_process; i += 100) {
			if (now == -1) {
				for (int j = 0; j < num_process; ++j)
					if(!added[j] && P[j].ready_time <= i)
					{
						printf(" %d \n", j);
						waiting.push({P[j].exec_time,j});
						added[j] = true;
						left_time[j] = P[j].exec_time;
					}
				if (waiting.empty()) continue;
				now = waiting.top().second;
				result[now].first = i;
				printf(" now = %d \n", now);
			}
			left_time[now] -= 100;
			if(left_time[now] <= 0)
			{
				waiting.pop();
				result[now].second = i + 100 - left_time[now];
				finish_count++;
				now = -1;
			}
		}
		for (int i = 0; i < num_process; ++i)
			printf("%d %d\n", result[i].first, result[i].second);
	}
}
