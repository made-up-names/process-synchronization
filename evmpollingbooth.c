#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<pthread.h>
#include<sys/wait.h>
//max slots for each evm=10 ???  did not understand the slots part
#define BUSY 0
#define FREE 1
#define UNALLOCATED 2
#define TO_VOTE 3
#define VOTED 4
#define FINISHED 0
typedef struct evm{
	pthread_t evm_id;

	int evmindex;
	struct booth* boothindex;
	int status; //busy //free

	int slots;
}Evm;

typedef struct voter{
	pthread_t voter_id;

	int voterindex; // for printing purposes and for evm to know which voter it is
	struct booth* boothindex; // accessed by booth[boothindex]
	struct evm* evmindex; // this would be accessed by booth[boothindex].evmarray[evmindex] ;
	int status; // unalloted [waiting for evm]; waiting to cast vote [voter_in_slot] ; 
}Voter;

typedef struct booth{
	pthread_t booth_id;
	
	int boothindex;
	int noofvoters;
	int noofevms;
	Evm* evmarray;
	Voter* voterarray;

	pthread_mutex_t mutex;
	pthread_cond_t voters;
	pthread_cond_t evms;

	int status;
}Booth;

void booth_thread(void* arg);
void voter_thread(void* arg);
void voter_wait_for_evm(Booth* booth_i,int voterindex);
void voter_in_slot(Booth* booth_i,int voterindex);
void evm_thread(void* arg);
int polling_ready_evm(Booth* booth_i,int count,int evmindex);

Booth booth[10000];	
int main()
{
	//first scan inputs
	int noofbooths;
	printf("Enter the no of booths :");
	scanf("%d",&noofbooths);

	
	for(int i=0;i<noofbooths;i++)
	{
		printf("Booth %d -\n",i+1);
		booth[i].boothindex=i;
		printf("Enter the no of voters - ");
		scanf("%d",&(booth[i].noofvoters));
		printf("Enter the no of evms - ");
		scanf("%d",&(booth[i].noofevms));
		if(booth[i].noofevms<=0|| booth[i].noofvoters<0)
		{
			printf("make sure evms>0 and noofvoters>=0\n");
			exit(1);
		}
		//taking pointers since no other option
		booth[i].evmarray=(Evm*)malloc(sizeof(Evm)*booth[i].noofevms);
		booth[i].voterarray=(Voter*)malloc(sizeof(Voter)*booth[i].noofvoters);

		booth[i].status=booth[i].noofvoters;
		for(int j=0;j<booth[i].noofevms;j++)
		{
			booth[i].evmarray[j].slots=0;
			booth[i].evmarray[j].evmindex=j;
			booth[i].evmarray[j].boothindex=&booth[i];
			booth[i].evmarray[j].status=FREE;
		}
		for(int j=0;j<booth[i].noofvoters;j++)
		{
			booth[i].voterarray[j].voterindex=j;
			booth[i].voterarray[j].boothindex=&booth[i];
			booth[i].voterarray[j].evmindex=NULL;
			booth[i].voterarray[j].status=UNALLOCATED;
		}
		pthread_cond_init(&(booth[i].voters),NULL);
		pthread_cond_init(&(booth[i].evms),NULL);
		pthread_mutex_init(&(booth[i].mutex),NULL);
	}
		
	for(int i=0;i<noofbooths;i++)
		pthread_create(&booth[i].booth_id,NULL,&booth_thread,&booth[i]);
	for(int i=0;i<noofbooths;i++)
		pthread_join(booth[i].booth_id,0);

	printf("Elections finished successfully\n");
	return 0;
}
void booth_thread(void* arg)
{
	Booth* booth_i=(Booth*)arg; //this is basically pointer of booth[i]
	//booth[i] is same as the original one now :) 
	for(int j=0;j<booth_i->noofevms;j++)
		pthread_create(&(booth_i->evmarray[j].evm_id),NULL,&evm_thread,&(booth_i->evmarray[j]));

	for(int j=0;j<booth_i->noofvoters;j++)
		pthread_create(&(booth_i->voterarray[j].voter_id),NULL,&voter_thread,&(booth_i->voterarray[j]));

	for(int j=0;j<booth_i->noofevms;j++)
		pthread_join(booth_i->evmarray[j].evm_id,0);
	for(int j=0;j<booth_i->noofvoters;j++)
		pthread_join(booth_i->voterarray[j].voter_id,0);

	// booth is finished
	printf("Voting is finished at Booth %d\n",booth_i->boothindex+1);
	free(booth_i->evmarray);
	free(booth_i->voterarray);

	return ;
}
void voter_thread(void* arg)
{
	Voter* voter_j=(Voter*)arg;
	voter_wait_for_evm(voter_j->boothindex,voter_j->voterindex);
	voter_in_slot(voter_j->boothindex,voter_j->voterindex);
	
	return ;
}

void voter_wait_for_evm(Booth* booth_i,int voterindex)
{
	pthread_mutex_lock(&(booth_i->mutex));
	int Vstatus=booth_i->voterarray[voterindex].status;
	while(Vstatus==UNALLOCATED) //unallocated waiitng for evm 
	{
		pthread_cond_wait(&(booth_i->evms),&(booth_i->mutex));
		Vstatus=booth_i->voterarray[voterindex].status;
	}
	//printf("Voter %d from Booth %d allocated EVM %d\n",voterindex+1,booth_i->boothindex+1,(booth_i->voterarray[voterindex].evmindex)->evmindex+1);
	pthread_mutex_unlock(&(booth_i->mutex));
	return ;
}
void voter_in_slot(Booth* booth_i,int voterindex)
{
	//HERE voter->status --> TO_VOTE 
	pthread_mutex_lock(&(booth_i->mutex));
	Evm* EVM=booth_i->voterarray[voterindex].evmindex;
	int Estatus=EVM->status;
	while(Estatus==BUSY)
	{
		pthread_cond_wait(&(booth_i->evms),&(booth_i->mutex));
		Estatus=EVM->status;
	}
	EVM->slots--;
	booth_i->voterarray[voterindex].status=VOTED;
	printf("Voter %d from Booth %d has casted vote\n",voterindex+1,booth_i->boothindex+1);
	pthread_cond_broadcast(&(booth_i->voters));
	pthread_mutex_unlock(&(booth_i->mutex));

}
void evm_thread(void* arg)
{	
	Evm* evm_j=(Evm*)arg;

	// mutex lock for reading 
	pthread_mutex_lock(&(evm_j->boothindex->mutex));
	while(evm_j->boothindex->status!=FINISHED)
	{
		pthread_mutex_unlock(&(evm_j->boothindex->mutex));
		int count=rand()%10+1;
		int noofslotsallocated=polling_ready_evm(evm_j->boothindex,count,evm_j->evmindex);
		if(noofslotsallocated==0)
		{
			printf("EVM %d of Booth %d TOTALLY DONE since all voters have been allocated\n",evm_j->evmindex+1,evm_j->boothindex->boothindex+1);
			return ;
		}
		//while no of slots!=0
		//wait for all voters; 
		printf("EVM %d of Booth %d started voting \n",evm_j->evmindex+1,evm_j->boothindex->boothindex+1);
		pthread_mutex_lock(&(evm_j->boothindex->mutex));
		evm_j->slots=noofslotsallocated;
		evm_j->status=FREE;
		pthread_cond_broadcast(&(evm_j->boothindex->evms));
		while(evm_j->slots!=0)
		{
			pthread_cond_wait(&(evm_j->boothindex->voters),&(evm_j->boothindex->mutex));
		}

		//pthread_mutex_unlock(evm_j->boothindex->mutex);

		printf("EVM %d of Booth %d finished voting \n",evm_j->evmindex+1,evm_j->boothindex->boothindex+1);

	}
	printf("EVM %d of Booth %d TOTALLY DONE since all voters have been allocated \n",evm_j->evmindex+1,evm_j->boothindex->boothindex+1);
	pthread_mutex_unlock(&(evm_j->boothindex->mutex));
	return ;
}
int polling_ready_evm(Booth* booth_i,int count,int evmindex)
{
	
	pthread_mutex_lock(&(booth_i->mutex));
	if(booth_i->status==FINISHED)
	{	
		pthread_mutex_unlock(&(booth_i->mutex));
		return 0;
	}
	booth_i->evmarray[evmindex].status=BUSY;
	booth_i->evmarray[evmindex].slots=count;
	printf("EVM %d of Booth %d has %d slots free\n",evmindex+1,booth_i->boothindex+1,count);
	pthread_mutex_unlock(&(booth_i->mutex));
	
	//allocate to all people
	int noofslotsallocated=0;
	while(noofslotsallocated<count)
	{
		pthread_mutex_lock(&(booth_i->mutex));
		if(booth_i->status==FINISHED)
		{
			pthread_mutex_unlock(&(booth_i->mutex));
			return noofslotsallocated;
		}
	//Assumption random function does proper distribution
	// another way is doing all from 0--> noofvoters all the time
		int j=rand()%booth_i->noofvoters;
		if(booth_i->voterarray[j].status==UNALLOCATED)
		{
			booth_i->voterarray[j].evmindex=&(booth_i->evmarray[evmindex]);
			booth_i->voterarray[j].status=TO_VOTE;
			booth_i->status--;
			noofslotsallocated++;
	printf(" Voter %d from Booth %d allocated EVM %d\n",j+1,booth_i->boothindex+1,(booth_i->voterarray[j].evmindex)->evmindex+1);
		}
		pthread_mutex_unlock(&(booth_i->mutex));
		
	}

	return noofslotsallocated;
}

