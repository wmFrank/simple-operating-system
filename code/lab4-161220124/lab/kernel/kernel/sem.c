#include "x86.h"
#include "common.h"
#include "device.h"

struct Semaphore sem_queue[MAX_SEM_NUM];
extern void update_s();
int P(struct Semaphore *sem);
int V(struct Semaphore *sem);
void initSemQueue();
int init_sem(int *sem, int value);
int post_sem(int *sem);
int wait_sem(int *sem);
int destroy_sem(int *sem);

int P(struct Semaphore *sem)
{
	sem->value--;
	if(sem->value < 0)
	{
		now->state = STATE_BLOCK_SEM;
		now->next = sem->list;
		sem->list = now;
		now = NULL;
	}
	return 0;
}

int V(struct Semaphore *sem)
{
	sem->value++;
	if(sem -> value <= 0)
	{
		if(sem->list == NULL)
			return 0;
		struct ProcessTable *temp = sem->list;
		while(temp->next != NULL)
			temp = temp->next;
		temp->state = STATE_RUNNABLE;
		if(temp == sem->list)
			sem->list = NULL;
		else
		{
			struct ProcessTable *tmp = sem->list;
			while(tmp->next != temp)
				tmp = tmp->next;
			tmp->next = NULL;
		}
		update_s();
	}
	return 0;
}

void initSemQueue()
{
	int i;
	for(i = 0; i < MAX_SEM_NUM; i++)
	{
		sem_queue[i].value = -1;
		sem_queue[i].list = NULL;
		sem_queue[i].used = 0;
	}
}

int init_sem(int *sem, int value)
{
	int flag = 0;
	int i = 0;
	for(int i = 0; i < MAX_SEM_NUM; i++)
	{
		if(sem_queue[i].used == 0)
		{
			flag = 1;
			break;
		}
	}
	if(flag == 0)
	{
		return -1;
	}
	else
	{
		*sem = i;
		sem_queue[i].used = 1;
		sem_queue[i].value = value;
		return i;
	}
}

int post_sem(int *sem)
{
	return V(&sem_queue[*sem]);
}

int wait_sem(int *sem)
{
	return P(&sem_queue[*sem]);
}

int destroy_sem(int *sem)
{
	if(sem_queue[*sem].list != NULL)
		assert(0);
	sem_queue[*sem].value = -1;
	sem_queue[*sem].used = 0;
	return 0;
}






