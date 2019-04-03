#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/shm.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<sys/ipc.h>
#include<pthread.h>
// First do with processes
// Next do with threads
// Write REPORT ? performance comparision with normal merge sort, proc1merge sort, thread1 merge sort ? how
// read shared memory ipc pthread fork semaphores mutexlocks
key_t key=IPC_PRIVATE;
typedef struct arg{
	int st1;
	int en2;
	int* arr;
}Arg;
void mergesort(void* arg);
void merge(int* arr,int st1,int en2);
int main()
{
	int n;
	scanf("%d",&n);

	//create
	int arr[n];
	for(int i=0;i<n;i++)
		scanf("%d",&arr[i]);

	Arg arg;
	arg.st1=0;
	arg.en2=n-1;
	arg.arr=arr;

	pthread_t tid;
	
	//create thread //maybe this thread is unnecessary check this
	//one once;
	mergesort(&arg);
	//can create a thread but was taking more time due to creation and joining
	/*int cerror=pthread_create(&tid,NULL,mergesort,&arg);
	if(cerror)
	{
		perror("pthread_create");
		exit(1);
	}

	//join thread
	int jerror=pthread_join(tid,NULL);
	if(jerror)
	{
		perror("pthread_join");
		exit(1);
	}*/

	for(int i=0;i<n;i++)
		printf("%d ",arr[i]);
	printf("\n");
	
	return 0;
}
void mergesort(void* arg)
{
	Arg* arg1=(Arg*)arg;
	int st1=arg1->st1;
	int en2=arg1->en2;
	int en1=(st1+en2)/2;
	int st2=en1+1;
	int* arr=arg1->arr;
	
	//if size < 5 do selection sort
	if(en2-st1<4)
	{
		//selection sort;
	
		for(int i=st1;i<=en2;i++)
		{
			for(int j=i+1;j<=en2;j++)
			{
				if(arr[j]<arr[i])
				{
					int temp=arr[i];
					arr[i]=arr[j];
					arr[j]=temp;
				}
			}

		}
		return ;

	}

	// type 1 // create 2 processes for left and right doing

	//Thread creation for left half
	Arg argl;
	argl.st1=st1;
	argl.en2=en1;
	argl.arr=arr;

	pthread_t tid1;
	int cerror1=pthread_create(&tid1,NULL,&mergesort,&argl);
	if(cerror1)
	{
		perror("pthread_create 1:");
		exit(1);
	}
	
	//Thread creation for right half
	Arg argr;
	argr.st1=st2;
	argr.en2=en2;
	argr.arr=arr;

	pthread_t tid2;
	int cerror2=pthread_create(&tid2,NULL,&mergesort,&argr);
	if(cerror2)
	{
		perror("pthread_create 2:");
		exit(1);
	}

	//Joining left thread
	int jerror1=pthread_join(tid1,NULL);
	if(jerror1)
	{
		perror("pthread_join 1");
		exit(1);
	}

	//Joining right thread
	int jerror2=pthread_join(tid2,NULL);
	if(jerror2)
	{
		perror("pthread_join 2");
		exit(1);
	}

	//Merge the 2 arrays
	merge(arr,st1,en2);
	return ;

}
void merge(int* arr,int st1,int en2)
{
	int en1=(st1+en2)/2;
	int st2=en1+1;
	int i=st1;
	int j=st2;
	int arr2[en2-st1+1];
	for(int k=0;k<=en2-st1;)
	{
		if(i>en1)
		{
			arr2[k++]=arr[j++];
		}
		else if(j>en2)
		{
			arr2[k++]=arr[i++];
		}
		else if(arr[j]<=arr[i])
			arr2[k++]=arr[j++];
		else
			arr2[k++]=arr[i++];
		
	}
	for(int k=st1;k<=en2;k++)
	{
		arr[k]=arr2[k-st1];
	}
	
	return ;
}
