#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<time.h>
#include<pthread.h>
#define BUSY 0
#define FREE 1
#define UNFORMED 0
#define FORMED 1
#define TRUE 1
#define FALSE 0

//critical section used in this
int org_status; //initialise BUSY
pthread_mutex_t orgstatus;

int playermet;
int refereemet;
int group[30000]; //allotment //initialise -1
pthread_mutex_t meeting;

int group_form[10000];//groupformation initialise UNFORMED
pthread_mutex_t groupform;

int game_ready[30000]; //initialise FALSE
pthread_mutex_t gameready;
pthread_mutex_t enter;

//otherglobal variables
int n;
int noplayers;
int noreferees;

void* player_thread(void* arg);
void* referee_thread(void* arg);
void* org_thread(void* arg);
int main()
{
	printf("Enter no of groups[1-1000]:");
	scanf("%d",&n);
	
	for(int i=0;i<3*n;i++){
		game_ready[i]=FALSE;
		group[i]=-1;
	}
	for(int i=0;i<n;i++)
		group_form[i]=UNFORMED;
	org_status=BUSY;
	playermet=0;
	refereemet=0;
	noplayers=0;
	noreferees=0;
	pthread_mutex_init(&orgstatus,NULL);
	pthread_mutex_init(&gameready,NULL);
	pthread_mutex_init(&groupform,NULL);
	pthread_mutex_init(&meeting,NULL);
	pthread_mutex_init(&enter,NULL);


	int arr[3*n];
	int players=0;
	int referees=0;
	for(int i=0;i<3*n;i++)
	{
		int leftplayers=2*n-players;
		float pforplayers=((float) leftplayers)/((float)(3*n+1-i));
		float r=rand()%1000000;
		float p=r/1000000;
		if(referees>=n)
		{
			players++;
			arr[i]=1;
		}
		else if(players>=2*n)
		{
			referees++;
			arr[i]=2;
		}
		else
		{
			if(p<pforplayers)
			{
			players++;
			arr[i]=1;
			}
			else
			{
			referees++;
			arr[i]=2;
			}
		}
		printf("%d ",arr[i]);
	}
	printf("\n");
	
	pthread_t thread[3*n+1];
	int number[3*n];
	
	pthread_create(&thread[3*n],NULL,org_thread,NULL);
	for(int i=0;i<3*n;i++)
	{
		if(arr[i]==1)
		{
			number[i]=noplayers;
			noplayers++;
			pthread_create(&thread[i],NULL,player_thread,&number[i]);
		}
		else
		{
			number[i]=noreferees;
			noreferees++;
			pthread_create(&thread[i],NULL,referee_thread,&number[i]);

		}
	}
	for(int i=0;i<=3*n;i++)
		pthread_join(thread[i],NULL);
	printf("DONE\n");
	return 0;
}

void* player_thread(void* arg)
{
	int* number=(int*) arg;
	int i;
	//can create mutex
	pthread_mutex_lock(&enter);
	printf("Player %d entered the academy\n",*number+1);
	pthread_mutex_unlock(&enter);
	printf("Player %d waiting for organiser\n",*number+1);
	while(1)
	{
		pthread_mutex_lock(&orgstatus);
		if(org_status==FREE)
		{
			//printf("1org_status=%d",org_status);
			pthread_mutex_lock(&meeting);
			//printf("org_status=%d",org_status);
			
			//meetOrganiser
			printf("Player %d met organiser\n",*number+1);
			i=3*(playermet/2)+playermet%2+1;
			group[i]=*number;
			printf("Player %d assigned group%d",*number+1,playermet/2+1);
			playermet++;

			pthread_mutex_unlock(&meeting);
			pthread_mutex_unlock(&orgstatus);
			
			break;
		}
		pthread_mutex_unlock(&orgstatus);
	}

	printf("Player %d waiting for groupformation\n",*number+1);
	//after meeting organiser busy wait for groupformation
	while(1)
	{
		pthread_mutex_lock(&groupform);
		if(group_form[i/3]==FORMED) //initialise with notformed
		{
			//enter court
			printf("Player %d entered the court\n",*number+1);
			pthread_mutex_unlock(&groupform);
			break;
		}
		pthread_mutex_unlock(&groupform);
	}
	printf("Player %d started warming up\n",*number+1);
	sleep(1);
	pthread_mutex_lock(&gameready);
	//printf("i=%d\n",i);
	game_ready[i]=TRUE; //initialise with false
	pthread_mutex_unlock(&gameready);
	printf("Player %d finished warming up\n",*number+1);
	return 0;
}
void* referee_thread(void* arg)
{
	int* number=(int*) arg;
	int i;

	//enterAcademy
	pthread_mutex_lock(&enter);
	printf("Referee %d entered the academy\n",*number+1);
	pthread_mutex_unlock(&enter);

	printf("Referee %d waiting for organiser\n",*number+1);
	//waitfororg
	while(1)
	{
		pthread_mutex_lock(&orgstatus);
		if(org_status==FREE) //initialise free only, other one is busy
		{
			//meetOrganiser
		//printf("1org_status=%d",org_status);
			pthread_mutex_lock(&meeting);
		//printf("org_status=%d",org_status);
			printf("Referee %d met organiser\n",*number+1);
			i=3*refereemet;
			group[i]=*number;
			printf("Referee %d assigned group%d",*number+1,refereemet+1);

			refereemet++;
			pthread_mutex_unlock(&meeting);
			pthread_mutex_unlock(&orgstatus);


			break;
		}
		pthread_mutex_unlock(&orgstatus);
	}
	printf("Player %d waiting for groupformation\n",*number+1);
	//after meeting organiser busy wait for groupformation
	while(1)
	{
		pthread_mutex_lock(&groupform);
		if(group_form[i/3]==FORMED) //initialise with notformed
		{
			printf("Referee %d entered court\n",*number+1);
			pthread_mutex_unlock(&groupform);
			break;
		}
		pthread_mutex_unlock(&groupform);
	}

	//equipment adjusting
	printf("Referee %d adjusting equipment\n",*number+1);
	sleep(0.5);

	//gameready
	pthread_mutex_lock(&gameready);
	//printf("i=%d\n",i);
	game_ready[i]=TRUE; //initialise with false
	pthread_mutex_unlock(&gameready);
	printf("Referee %d adjusted equipment\n",*number+1);
	return 0;
}
void* org_thread(void* arg)
{
	int group_no=0;
	//mutex
	//initialise with busy
	pthread_mutex_lock(&orgstatus);
	printf("started\n");
	org_status=FREE;
	pthread_mutex_unlock(&orgstatus);

	while(group_no<n)
	{
		//wait for that shit
		//wait until group is formed
		int ref_no=-1;
		while(1){

			pthread_mutex_lock(&meeting);
			if(group[3*group_no]!=-1&&group[3*group_no+1]!=-1&&group[3*group_no+2]!=-1) //initialise -1
			{
				ref_no=group[3*group_no];
				pthread_mutex_unlock(&meeting);
				//printf("meeting unlocked\n");	
				
				pthread_mutex_lock(&groupform);
				group_form[group_no]=FORMED; //initialise unformed
				pthread_mutex_unlock(&groupform);
				printf("Group %d formed with referee=%d\n",group_no+1,ref_no+1);	
				
				pthread_mutex_lock(&orgstatus);
				org_status=BUSY;
				pthread_mutex_unlock(&orgstatus);
				//printf("2orgstatus unlocked\n");
				
				break;
			}
			pthread_mutex_unlock(&meeting);
		//	sleep(0.5);

		}
		printf(" after group formationgroupno=%d\n",group_no);
		//after group formation the pplayers will change game_ready
		while(1){
		//	printf("groupno=%d\n",group_no);
			pthread_mutex_lock(&gameready);
			if(game_ready[3*group_no]==TRUE&&game_ready[3*group_no+1]==TRUE&&game_ready[3*group_no+2]==TRUE) //initialise -1
			{
				pthread_mutex_unlock(&gameready);
				
				
				printf("Referee %d started the game\n",ref_no+1);	
				pthread_mutex_lock(&orgstatus);
				org_status=FREE;
				pthread_mutex_unlock(&orgstatus);
				
				
				break;
			}
			pthread_mutex_unlock(&gameready);
		}

		group_no++;
	}
	printf("ORG DONE\n");

	return NULL;
}

